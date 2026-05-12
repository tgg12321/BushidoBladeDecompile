#!/usr/bin/env python3
"""Focused pre-work brief for a function: one tool, one screen, all
the context needed to decide how to attack it.

Replaces what was 3-4 separate tool calls (classify, agent-brief,
kengo-ref, near-miss, asmfix grep). Designed to be the FIRST thing an
agent runs after pulling a function from the queue.

What it shows (in order, no surprises):

  1. Bridge state — is this function bridged in asmfix.txt right now?
  2. Source location — which .c file, line range, stub or real body?
  3. Size + classification — instruction count, branch/jal/load count,
     classifier recommendation (standard / gte_function / needs_xxx_fix /
     permanently_blocked / bios_or_syscall), blocker tags.
  4. Kengo PS2-sequel reference (if any) — strong / weak / ambiguous
  5. Existing regfix / asmfix rules for this function (counts only;
     pointer to where to read them)
  6. Pre-applied: recent commits that retired similar-shape functions
     (heuristic: same .c file, similar size, recent in git log) —
     candidate templates to clone.
  7. One-line recommendation: easy_attempt / standard / needs_<X> /
     consult_user / etc.

Usage:
    python3 tools/preflight.py <func>
    python3 tools/preflight.py <func> --json
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
ASM_FUNCS = ROOT / "asm" / "funcs"


def _wsl_run(cmd_str: str, timeout: int = 30) -> str:
    """Run a shell command inside the project root, return stdout."""
    res = subprocess.run(["bash", "-c", cmd_str], capture_output=True,
                         text=True, cwd=str(ROOT), timeout=timeout)
    return res.stdout + res.stderr


def get_bridge_state(func: str) -> str:
    """Return one of 'active', 'retired-comment', 'none'."""
    asmfix = ROOT / "asmfix.txt"
    if not asmfix.exists():
        return "none"
    text = asmfix.read_text(encoding="utf-8", errors="ignore")
    if re.search(rf"^{re.escape(func)}:\s+replace_with_asmfile",
                 text, re.MULTILINE):
        return "active"
    if re.search(rf"^# RETIRE:\s+{re.escape(func)}:\s+replace_with_asmfile",
                 text, re.MULTILINE):
        return "retired-comment"
    return "none"


def find_src_for_func(func: str) -> tuple[Path | None, int | None, str]:
    """Locate the function in src/. Return (path, lineno, kind).

    kind is one of:
        "c_function"  - real-or-stub C function definition
        "inline_asm"  - glabel inside an inline __asm__() block
        "include_asm" - INCLUDE_ASM("asm/funcs", <func>) stub
        ""            - not found
    """
    func_def_re = re.compile(
        rf"^[A-Za-z_][\w *]*?\s\**{re.escape(func)}\s*\([^)]*\)\s*\{{",
        re.MULTILINE,
    )
    glabel_re = re.compile(rf"glabel\s+{re.escape(func)}\b")
    include_re = re.compile(rf'INCLUDE_ASM\s*\(\s*"asm/funcs"\s*,\s*{re.escape(func)}\s*\)')
    for p in sorted(SRC.glob("*.c")):
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        m = func_def_re.search(text)
        if m:
            line = text[:m.start()].count("\n") + 1
            return p, line, "c_function"
        m = glabel_re.search(text)
        if m:
            line = text[:m.start()].count("\n") + 1
            return p, line, "inline_asm"
        m = include_re.search(text)
        if m:
            line = text[:m.start()].count("\n") + 1
            return p, line, "include_asm"
    return None, None, ""


def get_classification(func: str) -> dict:
    """Run classify_func.py and parse its output (best-effort).

    classify_func emits text like:
      === func_X -- recommendation ===
        size       : N insns, M branches, K jal, L loads, P stores
        ...
    """
    out = _wsl_run(f'python3 tools/classify_func.py {func} 2>&1')
    rec_m = re.match(r"^=== (\S+) -- (\S+) ===", out.splitlines()[0]) if out else None
    rec = rec_m.group(2) if rec_m else "?"
    size_m = re.search(r"size\s+:\s+(\d+)\s+insns,\s+(\d+)\s+branches,\s+(\d+)\s+jal,"
                       r"\s+(\d+)\s+loads,\s+(\d+)\s+stores", out)
    blockers_m = re.search(r"blockers\s+:\s+(.+)", out)
    kengo_m = re.search(r"kengo\s+:\s+(.+)", out)
    return {
        "recommendation": rec,
        "size_insns": int(size_m.group(1)) if size_m else None,
        "branches": int(size_m.group(2)) if size_m else None,
        "jal": int(size_m.group(3)) if size_m else None,
        "loads": int(size_m.group(4)) if size_m else None,
        "stores": int(size_m.group(5)) if size_m else None,
        "blockers": blockers_m.group(1).strip() if blockers_m else "",
        "kengo": kengo_m.group(1).strip() if kengo_m else "",
    }


def count_existing_rules(func: str) -> dict:
    """Count regfix/asmfix rules referencing this function."""
    regfix = ROOT / "regfix.txt"
    asmfix = ROOT / "asmfix.txt"
    sdata_exclude = ROOT / "sdata_exclude.txt"
    counts = {"regfix": 0, "asmfix": 0, "sdata_exclude": 0}
    for p, key in [(regfix, "regfix"), (asmfix, "asmfix"), (sdata_exclude, "sdata_exclude")]:
        if not p.exists():
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        # Count lines starting with `<func>:` (not `# RETIRE:` etc.)
        counts[key] = sum(
            1 for line in text.splitlines()
            if re.match(rf"^{re.escape(func)}:\s", line)
        )
    return counts


def find_sibling_clones(func: str, classification: dict) -> list[str]:
    """Heuristic: recent commits whose messages mention 'retire' or
    'decomp' AND share a similar-class shape (same .c file, similar
    instruction count). Returns up to 5 commit SHAs as candidates to
    read for the cloning template.
    """
    src_path, _, _ = find_src_for_func(func)
    if not src_path:
        return []
    src_rel = src_path.relative_to(ROOT).as_posix()
    target_insns = classification.get("size_insns") or 0
    # Get last 60 commits touching src/<file>
    out = _wsl_run(f"git log --oneline -60 -- {src_rel} 2>&1")
    candidates: list[tuple[int, str, str]] = []
    for line in out.splitlines():
        m = re.match(r"^([0-9a-f]{7,40})\s+(.+)", line)
        if not m:
            continue
        sha, subj = m.group(1), m.group(2)
        if not re.search(r"retire|decomp|match", subj, re.IGNORECASE):
            continue
        # Pull the instruction count from the commit if it mentions one
        # ("47-insn", "60 insns", etc.). Score by closeness to target.
        insn_m = re.search(r"(\d+)[-\s]?(?:insn|instruction)", subj)
        insns = int(insn_m.group(1)) if insn_m else target_insns
        # Skip the function itself
        if func in subj:
            continue
        score = abs(insns - target_insns) if target_insns else 0
        candidates.append((score, sha, subj))
    candidates.sort()
    return [f"{sha} {subj}" for _, sha, subj in candidates[:5]]


def make_recommendation(func: str, bridge: str, cls: dict, counts: dict) -> str:
    rec = cls.get("recommendation", "?")
    if rec.startswith("permanently_blocked"):
        return f"out-of-scope: {rec} — do not work; classifier blocks this function"
    if rec.startswith("bios_or_syscall"):
        return f"out-of-scope: {rec} — canonical BIOS/syscall, leave as-is"
    if rec == "not_code_symbol" or rec == "not_found":
        return f"out-of-scope: {rec} — not a code symbol"
    if bridge == "none" and cls.get("size_insns", 0) and cls["size_insns"] > 0:
        return f"already decomped (no bridge in asmfix.txt) — verify-c to confirm"
    if bridge == "active":
        size = cls.get("size_insns", 0)
        if rec == "gte_function":
            return f"GTE wrapper, {size}-insn — clone the most-recent gte recipe from `git log`; use `.word` opcodes + register-asm pins; AVOID inline `addu` (use `move %0,%1`)"
        if rec == "needs_delay_slot_ra":
            return f"{size}-insn standard with aspsx_swra_delay — write C with explicit-goto dispatch so `sw $ra` falls into a branch delay slot naturally"
        if rec == "needs_rodata_split":
            return f"{size}-insn needs_rodata_split — jump-table or rodata fragment that splat hasn't split off; consult feedback_workflow_rules"
        if rec == "needs_lwl_fix":
            return f"{size}-insn needs lwl/lwr fix — use --expand-lh or check fix_lwl flag for this .c file"
        return f"{size}-insn {rec} — bridge is active; `dc.sh retire {func}` to start, write C, `dc.sh verify-c {func}`"
    if bridge == "retired-comment":
        return f"`# RETIRE: ` comment present — retirement already started; check src/ for C body or finish writing it"
    return f"recommendation: {rec}"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--json", action="store_true",
                    help="Emit machine-readable JSON")
    args = ap.parse_args()
    func = args.func

    bridge = get_bridge_state(func)
    src_path, lineno, kind = find_src_for_func(func)
    cls = get_classification(func)
    counts = count_existing_rules(func)
    siblings = find_sibling_clones(func, cls)
    recommendation = make_recommendation(func, bridge, cls, counts)

    asm_path = ASM_FUNCS / f"{func}.s"
    asm_exists = asm_path.exists()

    out = {
        "func": func,
        "bridge": bridge,
        "src": {
            "path": str(src_path.relative_to(ROOT).as_posix()) if src_path else None,
            "line": lineno,
            "kind": kind,
        },
        "asm_funcs_exists": asm_exists,
        "classification": cls,
        "rule_counts": counts,
        "sibling_recent_commits": siblings,
        "recommendation": recommendation,
    }

    if args.json:
        print(json.dumps(out, indent=2))
        return 0

    print(f"=== preflight: {func} ===")
    print()
    print(f"  bridge:    {bridge}")
    print(f"  src:       {out['src']['path']}:{lineno} ({kind})" if src_path else "  src:       (not found)")
    print(f"  asm/funcs: {asm_path.name}{' ✓' if asm_exists else ' ✗ (not found)'}")
    print()
    print(f"  classification: {cls.get('recommendation', '?')}")
    if cls.get("size_insns") is not None:
        print(f"    size:     {cls['size_insns']} insns "
              f"({cls.get('branches', 0)} br, {cls.get('jal', 0)} jal, "
              f"{cls.get('loads', 0)} ld, {cls.get('stores', 0)} st)")
    if cls.get("blockers"):
        print(f"    blockers: {cls['blockers']}")
    if cls.get("kengo"):
        print(f"    kengo:    {cls['kengo']}")
    print()
    print(f"  existing rules: regfix={counts['regfix']}  "
          f"asmfix={counts['asmfix']}  sdata_exclude={counts['sdata_exclude']}")
    print()
    if siblings:
        print(f"  recent sibling commits (candidates to clone):")
        for s in siblings:
            print(f"    {s}")
        print()
    print(f"  → {recommendation}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
