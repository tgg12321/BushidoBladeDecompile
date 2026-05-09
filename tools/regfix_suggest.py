#!/usr/bin/env python3
"""Auto-suggest regfix rules to close the build-vs-target diff for a function.

Reads the target asm (asm/funcs/<func>.s) and the current post-regfix
build output (via dump_text_indices --post-regfix), aligns by content,
and emits regfix rules — `delete`, `insert_after`, `subst` — that
would shrink the diff. Call this AFTER stub-replacement and a first
build, BEFORE writing any regfix rules by hand.

Usage:
    python3 tools/regfix_suggest.py <func>
    python3 tools/regfix_suggest.py <func> --apply        # append to regfix.txt
    python3 tools/regfix_suggest.py <func> --comment "..."

Workflow:
    1. dc.sh inline-replace <func> <c>     # integrate C
    2. dc.sh build                         # produce a buildable mismatched binary
    3. dc.sh regfix-suggest <func>         # print suggested rules
    4. Review, then `--apply` (or hand-edit regfix.txt)
    5. dc.sh build && dc.sh verify <func>  # confirm

What it can do well:
  * `delete` rules where mine has extra instructions vs target.
  * `insert_after` rules where mine is missing a contiguous block.
  * `subst` rules for 1:1 operand differences.
  * Detect `lui+lw %hi/%lo` vs gp-rel single-`lw` and emit a
    `sdata_exclude.txt` hint instead of a regfix rule.

What it skips (and why):
  * Reorders. SequenceMatcher will see a reorder as delete+insert,
    which is what we want anyway — `reorder` is dangerous when a
    label is attached to one of the moved instructions (the trap
    that bit func_80069AE4's loop body).
  * Hardcoded `.L<N>` label substitutions. The right `.L<N>` depends
    on GCC's per-build label numbering, so we just print a HINT when
    we see a branch with mismatching label suffix.

Limitations:
  * Best-effort. Always review suggestions before `--apply`.
  * Fewer than a dozen suggestions usually means high confidence;
    more than that may indicate a structural C-level mismatch
    (reach for permuter/smart_match instead of regfix).
  * Operates on POST-regfix build output, so existing rules already
    apply — suggestions add to whatever's there.
"""
from __future__ import annotations

import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path
from typing import List, Tuple

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
from active_func_scope import enforce_scope
ASM_FUNCS = ROOT / "asm" / "funcs"


# Reuse normalization from diff_build.
sys.path.insert(0, str(ROOT / "tools"))
try:
    from diff_build import normalize_insn  # type: ignore
except Exception:
    def normalize_insn(s: str) -> str:
        return re.sub(r"\s+", " ", s.strip())


def parse_target(func: str) -> List[str] | None:
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return None
    out: List[str] = []
    insn_re = re.compile(r"/\*\s*[\dA-Fa-f]+\s+[\dA-Fa-f]+\s+[\dA-Fa-f]+\s*\*/\s+(.+)$")
    for line in p.read_text(encoding="utf-8").splitlines():
        m = insn_re.match(line.strip())
        if m:
            out.append(normalize_insn(m.group(1).strip()))
    return out


def parse_mine(func: str) -> Tuple[List[Tuple[int, str, str]] | None, str | None]:
    """Returns (list of (idx, raw_text, normalized_text)) for the post-regfix
    pipeline output of `func`. Falls through to error str on failure."""
    result = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "dump_text_indices.py"),
         "--post-regfix", func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    if result.returncode != 0:
        return None, result.stderr.strip() or "dump_text_indices failed"
    out: List[Tuple[int, str, str]] = []
    for line in result.stdout.splitlines():
        m = re.match(r"\s*(\d+):\s*(.*)", line)
        if m:
            raw = m.group(2).strip()
            out.append((int(m.group(1)), raw, normalize_insn(raw)))
    if not out:
        return None, "dump_text_indices produced no instructions"
    return out, None


def _escape_for_regex_subst(s: str) -> str:
    """Escape s so it can be used as a regex pattern matching itself.

    Whitespace is collapsed to `\\s+` so target/mine tab-vs-space
    differences don't break the match."""
    # Escape regex metacharacters one-pass
    parts = []
    for ch in s:
        if ch == "\t" or ch == " ":
            parts.append(r"\s+")
        elif ch in r".^$*+?{}[]\|()":
            parts.append("\\" + ch)
        else:
            parts.append(ch)
    # Collapse runs of \s+ to a single \s+
    pat = "".join(parts)
    pat = re.sub(r"(?:\\s\+){2,}", r"\\s+", pat)
    return pat


def _gp_rel_pseudo(s: str) -> str | None:
    """If the line is a maspsx gp-rel pseudo `lw $X, D_<sym>`, return the symbol.

    Used to spot the func-needs-sdata_exclude case."""
    m = re.match(r"^l[wbh]u?\s+\$\w+,\s*([A-Za-z_]\w*)\s*$", s)
    if m:
        return m.group(1)
    return None


def _is_branch_with_label(s: str) -> Tuple[str, str] | None:
    """If the line is a branch/jump to a `.L<x>` label, return (op, label).

    Matches GCC's `.L<digits>` (mine) and disassembler-synthesized
    `.L<hex-addr>` (target, e.g. `.L80069B98`). Used to spot the
    label-mismatch case (different `.L<x>` in target vs mine) that
    can't be auto-rewritten without knowing offsets."""
    m = re.match(r"^(b\w+|j)\b\s+.*?(\.L[0-9A-Fa-f]+|\.L[A-Za-z_]\w*)", s)
    if m:
        return m.group(1), m.group(2)
    return None


def suggest(func: str) -> Tuple[List[str], List[str], dict]:
    """Run alignment, return (rules, hints, stats).

    rules: list of regfix.txt-format lines (already prefixed with `<func>:`).
    hints: list of human-readable hints that need manual action (e.g.
           label drift, sdata_exclude additions).
    stats: dict of counts (n_delete, n_insert, n_subst, n_label_mismatch, ...).
    """
    target = parse_target(func)
    if target is None:
        return [], [f"FAIL: asm/funcs/{func}.s not found"], {}
    mine, err = parse_mine(func)
    if err:
        return [], [f"FAIL: {err}"], {}
    if not mine:
        return [], [f"FAIL: no instructions extracted for {func}"], {}

    mine_norm = [n for (_, _, n) in mine]
    rules: List[str] = []
    hints: List[str] = []
    stats = {
        "delete": 0, "insert": 0, "subst": 0,
        "label_mismatch": 0, "gp_rel_hint": 0,
    }

    # Detect gp-rel pseudo on mine that target has as lui+lw — emit
    # sdata_exclude hint before we fight it with regfix. When the hint
    # fires, set `gp_rel_skip_syms` so we suppress the (otherwise
    # noisy) per-occurrence delete+insert rules for the same symbol.
    gp_rel_skip_syms: set[str] = set()
    seen_syms: set[str] = set()
    for idx, raw, norm in mine:
        sym = _gp_rel_pseudo(norm)
        if not sym or sym in seen_syms:
            continue
        seen_syms.add(sym)
        for tline in target:
            if re.search(rf"%hi\({re.escape(sym)}\)", tline):
                hints.append(
                    f"sdata_exclude hint: target uses lui+lw for {sym} but mine emits "
                    f"a gp-rel `lw`. Add `{func}: {sym}` to sdata_exclude.txt to "
                    f"force lui+lw and re-run regfix-suggest. Until you do, all "
                    f"{sym} sites will look like 1->2 instruction expansions."
                )
                stats["gp_rel_hint"] += 1
                gp_rel_skip_syms.add(sym)
                break

    sm = difflib.SequenceMatcher(None, mine_norm, target, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            continue
        if tag == "delete":
            # mine[i1:i2] is extra; target has nothing here.
            # Emit delete rules in *descending* idx order so multiple deletes
            # in the same hunk don't shift indices on each other (deletes are
            # processed desc by regfix.py anyway, but writing them desc keeps
            # the regfix.txt readable).
            for k in range(i2 - 1, i1 - 1, -1):
                idx = mine[k][0]
                rules.append(f"{func}: delete @ {idx}  # auto: extra `{mine[k][1]}`")
                stats["delete"] += 1

        elif tag == "insert":
            # mine missing target[j1:j2]. Insert after the previous mine line.
            if i1 == 0:
                hints.append(
                    f"INSERT-AT-START: target[{j1}..{j2}] = "
                    f"{[target[j] for j in range(j1, j2)][:3]}... — must insert "
                    f"before mine idx 0; emit `insert` (not `insert_after`) by hand."
                )
                continue
            anchor_idx = mine[i1 - 1][0]
            # Use insert_after at SAME anchor; regfix processes file-order at
            # the same idx as LIFO, so write target's instructions in REVERSE
            # so that the FIRST written rule ends up FARTHEST from the anchor
            # (which matches target order: anchor, target[j1], target[j1+1], ...).
            for j in range(j2 - 1, j1 - 1, -1):
                rules.append(
                    f'{func}: insert_after "{target[j]}" @ {anchor_idx}  '
                    f"# auto: target has this after idx {anchor_idx}"
                )
                stats["insert"] += 1

        elif tag == "replace":
            # If this hunk is a `lw $X, sym` -> `lui+lw` expansion for a
            # symbol we already hinted for sdata_exclude, suppress the
            # per-occurrence rules: the user fixes it once via
            # sdata_exclude.txt and re-runs regfix-suggest.
            if (i2 - i1 == 1) and (j2 - j1 == 2):
                mine_sym = _gp_rel_pseudo(mine_norm[i1])
                if mine_sym and mine_sym in gp_rel_skip_syms:
                    if re.search(rf"%hi\({re.escape(mine_sym)}\)", target[j1]):
                        continue

            # Both sides have content. If lengths match, emit per-line subst.
            # Otherwise emit delete+insert.
            if (i2 - i1) == (j2 - j1):
                for off in range(i2 - i1):
                    idx, raw, mine_n = mine[i1 + off]
                    target_n = target[j1 + off]
                    # Branch label mismatch — flag, don't auto-rewrite.
                    mb = _is_branch_with_label(mine_n)
                    tb = _is_branch_with_label(target_n)
                    if mb and tb and mb[1] != tb[1]:
                        hints.append(
                            f"BRANCH-LABEL idx {idx}: mine `{mine_n}`, target `{target_n}`. "
                            f"Find the `.L<N>` in mine that lives at the same offset as "
                            f"target's `{tb[1]}` and write a `subst` rule pointing there. "
                            f"Hardcoded `.L<N>` rules drift across builds — use a "
                            f"`\\.L\\d+` regex pattern."
                        )
                        stats["label_mismatch"] += 1
                        continue
                    pattern = _escape_for_regex_subst(raw)
                    # Replacement: write target's normalized text. Tabs are
                    # canonicalized to `\t` so the file is readable.
                    repl = target_n.replace(" ", "\t", 1)
                    rules.append(
                        f'{func}: subst "{pattern}" "{repl}" @ {idx}  '
                        f"# auto: was `{raw}` -> `{target_n}`"
                    )
                    stats["subst"] += 1
            else:
                # Length mismatch within a replace — emit delete + insert.
                for k in range(i2 - 1, i1 - 1, -1):
                    idx = mine[k][0]
                    rules.append(
                        f"{func}: delete @ {idx}  "
                        f"# auto (replace): drop `{mine[k][1]}`"
                    )
                    stats["delete"] += 1
                anchor_idx = mine[i1 - 1][0] if i1 > 0 else None
                if anchor_idx is None:
                    hints.append(
                        f"INSERT-AT-START (replace): target[{j1}..{j2}] needs hand-written "
                        f"`insert @ 0`."
                    )
                else:
                    for j in range(j2 - 1, j1 - 1, -1):
                        rules.append(
                            f'{func}: insert_after "{target[j]}" @ {anchor_idx}  '
                            f"# auto (replace): target has after idx {anchor_idx}"
                        )
                        stats["insert"] += 1

    return rules, hints, stats


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--apply", action="store_true",
                    help="Append the suggested rules to regfix.txt")
    ap.add_argument("--comment", default=None,
                    help="Prepend a `# <comment>` line above the rules")
    ap.add_argument("--max-rules", type=int, default=40,
                    help="Cap on emitted rules; >max means likely structural mismatch")
    args = ap.parse_args()

    rules, hints, stats = suggest(args.func)

    if hints:
        print("# regfix-suggest: HINTS (manual action required)")
        for h in hints:
            print(f"#   {h}")
        print()

    if not rules:
        if not hints:
            print(f"# regfix-suggest: no diff for {args.func} (already matching, or no rules suggested)")
        return 0

    if len(rules) > args.max_rules:
        print(f"# regfix-suggest: WARNING — {len(rules)} suggested rules exceeds "
              f"--max-rules={args.max_rules}.")
        print(f"# Likely a structural C-level mismatch; reach for "
              f"`dc.sh permute` / `smart_match` BEFORE adding {len(rules)} regfix lines.")
        print(f"# (Showing first {args.max_rules}; pass --max-rules 0 to disable cap.)")
        if args.max_rules > 0:
            rules = rules[: args.max_rules]
        if args.apply:
            print("# Refusing to --apply with rule count above cap. Lower the cap or hand-edit.",
                  file=sys.stderr)
            args.apply = False

    summary = (
        f"# regfix-suggest stats: "
        f"{stats['delete']} delete, "
        f"{stats['insert']} insert, "
        f"{stats['subst']} subst, "
        f"{stats['label_mismatch']} label-mismatch hint(s), "
        f"{stats['gp_rel_hint']} sdata-exclude hint(s)"
    )
    print(summary)
    print()

    for r in rules:
        print(r)

    if args.apply:
        # Refuse cross-function applies. regfix-suggest --apply is exactly the
        # path subagent #1 used to (auto-)remove rules from func_8005D554 et al.
        enforce_scope(args.func, action="append regfix-suggest rules for")
        target = ROOT / "regfix.txt"
        text = target.read_text(encoding="utf-8") if target.exists() else ""
        if text and not text.endswith("\n"):
            text += "\n"
        if args.comment:
            text += f"\n# {args.comment}\n"
        else:
            text += f"\n# regfix-suggest: auto-generated rules for {args.func}\n"
        text += "\n".join(rules) + "\n"
        target.write_text(text, encoding="utf-8")
        print(f"\n# Appended {len(rules)} rules to {target.name}.")
        print(f"# Next: `bash tools/dc.sh build && bash tools/dc.sh verify {args.func}`")
    else:
        print()
        print(f"# (Preview only. Pass --apply to append to regfix.txt.)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
