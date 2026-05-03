#!/usr/bin/env python3
"""Auto-fix `.L<N>` label drift in regfix.txt rules.

Background: GCC numbers `.L<N>` labels file-wide. When you add/remove a
function in a .c file, the label numbers shift in every other function
in that file. regfix subst rules with literal `.L347` in the replacement
become broken — the linker errors out with `undefined reference`.

This tool DRIVES OFF THE LINKER ERROR. By default it runs `make`, parses
linker error messages of the form `undefined reference to '.L<N>'`,
finds the matching regfix rule, and proposes a fix. The fix heuristic:
the label currently in the instruction at the rule's @ idx is what the
rule's pattern is matching; substitute that label number into the
replacement, preserving any offset suffix (e.g., `.L347-4`).

Usage:
    python3 tools/fix_label_drift.py            # dry-run from current build state
    python3 tools/fix_label_drift.py --apply    # actually edit regfix.txt + rebuild

Limitations:
- Only triggered by linker errors. If the build passes, no fixes are
  proposed even if a rule looks suspicious (we trust the build).
- Only fixes the "match label at idx, replace with offset version" case.
  If the rule branches to a different label than what's at the matched
  idx, the heuristic won't apply (we warn).
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REGFIX = ROOT / "regfix.txt"


def find_source_file(func_name: str) -> Path | None:
    src_dir = ROOT / "src"
    decl_re = re.compile(rf'\b{re.escape(func_name)}\s*\(', re.MULTILINE)
    for c in sorted(src_dir.glob("*.c")):
        try:
            text = c.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if decl_re.search(text):
            return c
    return None


def get_function_asm(func: str) -> tuple[list[str], str | None]:
    """Run pipeline, return (asm_lines, error_message)."""
    src = find_source_file(func)
    if src is None:
        return [], f"{func} not found in src/"
    sys.path.insert(0, str(ROOT / "tools"))
    try:
        from dump_text_indices import build_pipeline_cmd  # type: ignore
    except ImportError:
        return [], "dump_text_indices not importable"
    cmd = build_pipeline_cmd(ROOT, src)
    result = subprocess.run(
        ["bash", "-c", cmd], capture_output=True, text=True, cwd=str(ROOT)
    )
    if not result.stdout.strip():
        return [], f"pipeline failed: {result.stderr[:300]}"
    text = result.stdout
    in_func = False
    out: list[str] = []
    func_label = re.compile(rf'^{re.escape(func)}:$')
    for line in text.splitlines():
        s = line.strip()
        if func_label.match(s):
            in_func = True
            continue
        if in_func:
            if re.match(rf'^\s*\.end\s+{re.escape(func)}', s):
                break
            out.append(s)
    return out, None


def is_instruction(line: str) -> bool:
    s = line.strip()
    if not s:
        return False
    if s.startswith(".") or s.startswith("#") or s.endswith(":"):
        return False
    if s.startswith("gcc2_compiled"):
        return False
    return True


def build_label_map(asm_lines: list[str]) -> tuple[dict[str, int], dict[int, str]]:
    label_to_idx: dict[str, int] = {}
    idx_to_insn: dict[int, str] = {}
    pending: list[str] = []
    idx = 0
    for line in asm_lines:
        s = line.strip()
        if s.endswith(":") and not s.startswith(".") and not is_instruction(s):
            continue  # function symbol or similar
        elif s.endswith(":"):
            pending.append(s[:-1])
        elif is_instruction(s):
            for lbl in pending:
                label_to_idx[lbl] = idx
            pending = []
            idx_to_insn[idx] = s
            idx += 1
    return label_to_idx, idx_to_insn


# Match: func: subst "PATTERN" "REPLACEMENT" @ IDX
RULE_RE = re.compile(
    r'^(?P<func>\w+)\s*:\s*subst\s+'
    r'"(?P<pattern>[^"]+)"\s+'
    r'"(?P<replacement>[^"]*)"\s*'
    r'@\s*(?P<idx>\d+)\s*$'
)
LABEL_RE = re.compile(r'\.L(\d+)')


def fix_rule(rule_text: str, asm_data: dict[str, tuple],
             synthesized_labels: dict[str, set[str]],
             broken_labels_by_func: dict[str, set[str]] | None) -> tuple[str, str | None]:
    """Return (fixed_rule, status_message). status is None if no change needed,
    else a one-line description.

    If broken_labels_by_func is provided, only fix rules whose
    replacement contains a label the linker reported as undefined.
    """
    m = RULE_RE.match(rule_text.strip())
    if not m:
        return rule_text, None
    func = m["func"]
    pattern = m["pattern"]
    replacement = m["replacement"]
    idx = int(m["idx"])

    # Only care about rules with .L<N> in replacement (where drift hurts)
    rep_labels = LABEL_RE.findall(replacement)
    if not rep_labels:
        return rule_text, None

    # In linker-driven mode: skip rules that don't reference a linker-
    # broken label.
    if broken_labels_by_func is not None:
        broken = broken_labels_by_func.get(func, set())
        if not any(f".L{n}" in broken for n in rep_labels):
            return rule_text, None

    if func not in asm_data:
        return rule_text, f"  SKIP: {func} pipeline not available"
    label_map, idx_map = asm_data[func]

    insn_at_idx = idx_map.get(idx)
    if insn_at_idx is None:
        return rule_text, f"  WARN: {func} idx {idx} not found"

    # Check if the replacement labels still exist in the asm OR are
    # synthesized by other regfix rules (insert/subst introducing
    # `.LXXX:` literally).
    func_synth = synthesized_labels.get(func, set())
    missing = [
        n for n in rep_labels
        if f".L{n}" not in label_map and f".L{n}" not in func_synth
    ]
    if not missing:
        return rule_text, None  # rule is fine

    # We have drifted labels. Heuristic: find the label currently in the
    # instruction at idx. If pattern uses \.L\d+, it matches whatever's
    # there. The replacement should reference that same number (with the
    # offset preserved).
    cur_labels_in_insn = LABEL_RE.findall(insn_at_idx)
    if not cur_labels_in_insn:
        return rule_text, (
            f"  WARN: {func} idx {idx} has no .L<N> label in current asm "
            f"(`{insn_at_idx}`); manual fix needed"
        )
    if len(set(cur_labels_in_insn)) > 1:
        return rule_text, (
            f"  WARN: {func} idx {idx} has multiple labels {cur_labels_in_insn}, "
            f"ambiguous"
        )

    new_label_num = cur_labels_in_insn[0]
    # Replace each missing .L<N> in replacement with .L<new>
    new_replacement = replacement
    for old_n in missing:
        new_replacement = new_replacement.replace(f".L{old_n}", f".L{new_label_num}")

    if new_replacement == replacement:
        return rule_text, None

    new_rule = (
        f'{func}: subst "{pattern}" "{new_replacement}" @ {idx}'
    )
    msg = (
        f"  FIX: {func} (idx {idx}): "
        f".L{','.join(missing)} -> .L{new_label_num}"
    )
    return new_rule, msg


def find_broken_labels_from_build() -> dict[str, set[str]]:
    """Run `make` and parse linker errors. Returns {func: {label, ...}}.
    Returns empty dict if the build succeeds.
    """
    result = subprocess.run(
        ["make", "-s"], capture_output=True, text=True, cwd=str(ROOT)
    )
    if result.returncode == 0:
        return {}

    # Linker error format:
    #   build/src/text1b.o: in function `func_800644FC':
    #   src/text1b.c:(.text+0x1c684): undefined reference to `.L352'
    output = (result.stdout or "") + (result.stderr or "")
    cur_func: str | None = None
    broken: dict[str, set[str]] = {}
    func_re = re.compile(r"in function `([^']+)'")
    label_re = re.compile(r"undefined reference to `(\.L\d+)'")
    for line in output.splitlines():
        m = func_re.search(line)
        if m:
            cur_func = m.group(1)
            continue
        m = label_re.search(line)
        if m and cur_func:
            broken.setdefault(cur_func, set()).add(m.group(1))
    return broken


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true",
                    help="Write changes to regfix.txt + rebuild (default: dry-run)")
    ap.add_argument("--func", help="Only check rules for this function")
    ap.add_argument("--all-rules", action="store_true",
                    help="Check ALL .L<N> rules (legacy mode, may produce false "
                         "positives for synthesized labels). Default: only fix "
                         "rules implicated by linker errors from a failing build.")
    args = ap.parse_args()

    if not REGFIX.exists():
        print(f"ERROR: {REGFIX} missing", file=sys.stderr)
        return 1

    text = REGFIX.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=True)

    # Default mode: drive off linker errors. Only fix rules whose
    # replacement labels are reported as undefined by the linker.
    broken_from_build: dict[str, set[str]] | None = None
    if not args.all_rules:
        print("Running build to detect actual label drift...")
        broken_from_build = find_broken_labels_from_build()
        if not broken_from_build:
            print("Build passed — no label drift detected.")
            return 0
        print(f"Linker reports undefined labels in {len(broken_from_build)} function(s):")
        for func, labels in sorted(broken_from_build.items()):
            print(f"  {func}: {', '.join(sorted(labels))}")
        print()

    # First pass: collect functions that have rules with .L<N> in replacement,
    # AND collect synthesized labels (introduced by other regfix `insert`/
    # `subst` rules — these ARE valid even if not in pipeline output).
    funcs_to_check: set[str] = set()
    synthesized: dict[str, set[str]] = {}
    label_def_re = re.compile(r'\.L\d+(?=:)')

    func_line_re = re.compile(r'^(\w+)\s*:\s*(\w+)\s+(.*)$')
    for line in lines:
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        m = func_line_re.match(s)
        if not m:
            continue
        func = m.group(1)
        op = m.group(2)
        rest = m.group(3)
        # Find any `.LXXX:` label definitions inside the rule body
        for lbl_def in label_def_re.findall(rest):
            synthesized.setdefault(func, set()).add(lbl_def)

        m2 = RULE_RE.match(s)
        if m2:
            if args.func and m2["func"] != args.func:
                continue
            if LABEL_RE.search(m2["replacement"]):
                funcs_to_check.add(m2["func"])

    # Restrict funcs_to_check to those reported by linker, if available.
    if broken_from_build is not None:
        funcs_to_check &= set(broken_from_build.keys())

    if not funcs_to_check:
        print("No rules with .L<N> labels found.")
        return 0

    print(f"Checking {len(funcs_to_check)} function(s) for label drift...")

    # Run the pipeline for each function (cached for the second pass).
    asm_data: dict[str, tuple] = {}
    for func in sorted(funcs_to_check):
        asm_lines, err = get_function_asm(func)
        if err:
            print(f"  SKIP {func}: {err}")
            continue
        asm_data[func] = build_label_map(asm_lines)

    # Second pass: fix each rule.
    new_lines: list[str] = []
    fixes: list[str] = []
    warnings: list[str] = []
    changed = 0
    for line in lines:
        m = RULE_RE.match(line.strip())
        if not m or (args.func and m["func"] != args.func):
            new_lines.append(line)
            continue
        new_rule, msg = fix_rule(line.rstrip("\n"), asm_data, synthesized, broken_from_build)
        if msg:
            (warnings if msg.lstrip().startswith("WARN") or msg.lstrip().startswith("SKIP") else fixes).append(msg)
        if new_rule + "\n" != line:
            new_lines.append(new_rule + "\n")
            changed += 1
        else:
            new_lines.append(line)

    for w in warnings:
        print(w)
    for f in fixes:
        print(f)

    if changed == 0:
        print("\nNo drift detected — all label rules resolve correctly.")
        return 0

    print(f"\n{changed} rule(s) need updating.")
    if args.apply:
        REGFIX.write_text("".join(new_lines), encoding="utf-8", newline="\n")
        print(f"Wrote {REGFIX.relative_to(ROOT)}.")
    else:
        print("Run with --apply to actually write the changes.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
