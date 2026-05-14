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


def _clean_ws(s: str) -> str:
    """Collapse whitespace to a single `mnemonic\\toperands` form, commas tight.

    Keeps the REAL opcode and operands (unlike normalize_insn, which aliases
    opcodes and rewrites hex for byte-comparison) — safe to use as a regfix
    replacement that `as` will assemble."""
    s = re.sub(r"#.*$", "", s).strip()
    s = re.sub(r"\s*,\s*", ",", s)
    s = re.sub(r"\s+", " ", s)
    return s.replace(" ", "\t", 1)


def parse_target(func: str) -> List[Tuple[str, str]] | None:
    """Return [(raw, norm)] per target instruction.

    raw  = whitespace-cleaned actual instruction text (safe as a regfix
           replacement — real opcode, real operands).
    norm = normalize_insn() output (lossy: opcode aliases, hex->dec) — used
           ONLY for difflib alignment / byte-equivalence comparison.

    Using `norm` as a replacement is the bug this split fixes: normalize_insn
    rewrites `subu $X,$Y,N` -> `addiu $X,$Y,-N` and aliases `b`/`j`, so a
    suggested rule would emit different bytes than target actually has."""
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return None
    out: List[Tuple[str, str]] = []
    insn_re = re.compile(r"/\*\s*[\dA-Fa-f]+\s+[\dA-Fa-f]+\s+[\dA-Fa-f]+\s*\*/\s+(.+)$")
    for line in p.read_text(encoding="utf-8").splitlines():
        m = insn_re.match(line.strip())
        if m:
            raw = m.group(1).strip()
            out.append((_clean_ws(raw), normalize_insn(raw)))
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


_J_LABEL_RE = re.compile(r"^j\s+(\.L\w+)\s*$")
_B_LABEL_RE = re.compile(r"^b\s+(\.L\w+)\s*$")
_NOP_RE = re.compile(r"^nop\b")


def _detect_j_to_b_with_delay_fill(
    mine: List[Tuple[int, str, str]], target: List[str], func: str
) -> List[str]:
    """Detect the pattern where mine has `[X, j .L<N>, nop]` and target has
    `[b .L<N>, X-equivalent-in-delay-slot]` at the same logical position.

    When found, emit a paired `fill_delay` + `subst j->b` rule (hint-style)
    so the agent doesn't have to discover the recipe manually.

    Detection heuristic (per maspsx position k in mine):
      - mine[k]   text matches `^j\\s+(\\.L\\w+)$`        (J-format jump)
      - mine[k+1] text matches `^nop\\b`                  (delay slot is nop)
      - mine[k-1] text exists and is a regular insn       (the candidate)
      - target has a `b .L<N>` near the same logical position
      - target's instruction in the b's delay slot text-matches mine[k-1]

    Emits both `fill_delay` and `subst` as a hint string (not a rule)
    so the agent can audit before applying. Recurring pattern from
    func_8007EA0C (3 occurrences) and similar GTE+sign-preserving multiplies.
    """
    hints: List[str] = []
    target_norm = [t.strip() for t in target]

    # Use RAW text for the j/b detection (not normalized): diff_build's
    # normalize_insn aliases `b` -> `j` for byte-comparison purposes, but
    # they have different MIPS encodings (BEQ vs J format). We want to
    # detect mine's actual `j` and target's actual `b`.
    target_raw = [t.strip() for t in target]

    for k in range(1, len(mine) - 1):
        idx_mine, raw_mine, _norm_mine = mine[k]
        m_j = _J_LABEL_RE.match(raw_mine.strip())
        if not m_j:
            continue
        if not _NOP_RE.match(mine[k + 1][1].strip()):
            continue
        candidate_raw = mine[k - 1][1].strip()
        candidate_idx = mine[k - 1][0]
        # Look in target for `b .L<N>` near k (within +/-3)
        for tj in range(max(0, k - 3), min(len(target_raw), k + 3)):
            m_b = _B_LABEL_RE.match(target_raw[tj])
            if not m_b:
                continue
            # Verify target's delay slot matches mine's candidate (raw text).
            # Use a loose match: same opcode + at least the destination reg.
            if tj + 1 >= len(target_raw):
                continue
            if target_raw[tj + 1].split()[:1] != candidate_raw.split()[:1]:
                continue
            hints.append(
                f"J->B+FILL-DELAY at mine idx {idx_mine}: mine has "
                f"`{candidate_raw}; j {m_j.group(1)}; nop` but target has "
                f"`b {m_b.group(1)}; {target_raw[tj+1]} (in delay)`. Paired recipe "
                f"(the subst is the drift-immune mnemonic-only form — touches only "
                f"the `j`, no label reference, so it survives .L<N> renumbering):\n"
                f"    {func}: fill_delay @ {idx_mine} <- {candidate_idx}\n"
                f"    {func}: subst \"\\bj\\b\" \"b\" @ {idx_mine}\n"
                f"  (Don't apply via regfix-suggest --apply — these don't compose with "
                f"the difflib-emitted delete+insert rules. Add by hand via add-regfix or tmp/inject_*.py.)"
            )
            break  # one match per j

    return hints


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

    # raw  = byte-correct instruction text (use for rule REPLACEMENTS)
    # norm = lossy normalized text (use ONLY for difflib alignment)
    target_raw = [r for (r, _) in target]
    target_norm = [n for (_, n) in target]
    mine_norm = [n for (_, _, n) in mine]
    rules: List[str] = []
    hints: List[str] = []
    stats = {
        "delete": 0, "insert": 0, "subst": 0,
        "label_mismatch": 0, "gp_rel_hint": 0, "j_to_b_paired": 0,
    }

    # Pre-pass: detect the j+nop ↔ b+useful-insn pattern (recurring in
    # GTE wrappers and sign-preserving multiplies, e.g. func_8007EA0C).
    # Emits hint-style paired recipes that don't compose well with the
    # difflib-emitted delete/insert rules.
    j_to_b_hints = _detect_j_to_b_with_delay_fill(mine, target_raw, func)
    hints.extend(j_to_b_hints)
    stats["j_to_b_paired"] = len(j_to_b_hints)

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
        for tline in target_raw:
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

    sm = difflib.SequenceMatcher(None, mine_norm, target_norm, autojunk=False)
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
                    f"{target_raw[j1:j2][:3]}... — must insert "
                    f"before mine idx 0; emit `insert` (not `insert_after`) by hand."
                )
                continue
            anchor_idx = mine[i1 - 1][0]
            # Use insert_after at SAME anchor; regfix processes file-order at
            # the same idx as LIFO, so write target's instructions in REVERSE
            # so that the FIRST written rule ends up FARTHEST from the anchor
            # (which matches target order: anchor, target[j1], target[j1+1], ...).
            # RAW target text — normalize_insn aliases opcodes (subu->addiu,
            # b<->j), which would assemble to different bytes.
            for j in range(j2 - 1, j1 - 1, -1):
                rules.append(
                    f'{func}: insert_after "{target_raw[j]}" @ {anchor_idx}  '
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
                    if re.search(rf"%hi\({re.escape(mine_sym)}\)", target_raw[j1]):
                        continue

            # Both sides have content. If lengths match, emit per-line subst.
            # Otherwise emit delete+insert.
            if (i2 - i1) == (j2 - j1):
                for off in range(i2 - i1):
                    idx, raw, mine_n = mine[i1 + off]
                    target_n = target_norm[j1 + off]   # alignment / detection only
                    target_r = target_raw[j1 + off]    # byte-correct replacement
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
                    # Replacement: target's RAW text (already `mnemonic\toperands`
                    # from _clean_ws). NOT target_n — normalize_insn aliases
                    # opcodes (subu->addiu, b<->j) and rewrites hex, which would
                    # make the rule assemble to different bytes than target.
                    repl = target_r
                    rules.append(
                        f'{func}: subst "{pattern}" "{repl}" @ {idx}  '
                        f"# auto: was `{raw}` -> `{target_r}`"
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
                            f'{func}: insert_after "{target_raw[j]}" @ {anchor_idx}  '
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
    ap.add_argument("--max-rules", type=int, default=15,
                    help="Cap on emitted rules. >15 usually means the structural "
                         "shape is wrong (auto-suggest doesn't track index-shift "
                         "cascades from interleaved insert/delete, so big rule "
                         "sets often regress). Lowered from 40 after the "
                         "cpu_check_tubazeri retrospective: a 43-rule auto-apply "
                         "regressed mine from 32→44 diffs because the rules "
                         "operated on each other's intermediate indices.")
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
        print(f"# Likely a structural C-level mismatch; auto-apply will probably")
        print(f"# REGRESS the build because rules operate on each other's intermediate")
        print(f"# indices and don't account for as-pass delay-slot fills.")
        print(f"# Switch tactic INSTEAD of stretching the cap:")
        print(f"#   - re-read m2c base.c for shape mismatch (top of `dc.sh agent-brief`)")
        print(f"#   - try `dc.sh permute-adaptive {args.func}` for C-structural search")
        print(f"#   - pick 3-5 KEY rules by hand from this output, not all of them")
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
        f"{stats['gp_rel_hint']} sdata-exclude hint(s), "
        f"{stats.get('j_to_b_paired', 0)} j->b+fill-delay paired hint(s)"
    )
    print(summary)
    print()

    for r in rules:
        print(r)

    if args.apply:
        # Refuse cross-function applies. regfix-suggest --apply is exactly the
        # path an earlier agent used to (auto-)remove rules from func_8005D554 et al.
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
