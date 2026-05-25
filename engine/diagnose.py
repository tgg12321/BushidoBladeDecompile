"""Diff diagnosis — classify a function's cheat-disabled gap against the known
immutable-plateau patterns (docs/IMMUTABLE_PLATEAUS.md), so the controller
permutes only the genuinely-matchable subset instead of burning cycles on gaps
the toolchain provably can't close. No model, no build (reuses the scan's
all-rules-stripped .o vs the canonical .o).

Verdicts:
  MATCHABLE      gap has no immutable signature — worth the permuter rung.
  SINGLE-REG     1-insn single-source-register diff (Pattern 4): matchable in
                 principle via pure-C restructure; hard for random search — a
                 prime targeted candidate.
  PLATEAU        accepted immutable pattern (addressing-mode / reorder /
                 epilogue); regfix is the SOTN-accepted compromise.
  CANONICAL-ASM  mfhi/mflo synthetic intermediate — no C form; the legit path is
                 inline_asm_canonical, not pure C.
"""
from __future__ import annotations

import difflib
import re

from . import sandbox
from . import score

_MFHI = re.compile(r"^(mfhi|mflo)\b")
_LOADSTORE = re.compile(r"^(lw|lh|lhu|lb|lbu|sw|sh|sb|lwl|lwr|swl|swr)\b")
_JR = re.compile(r"^(jr|j)\b")


def _opcode(insn: str) -> str:
    return insn.split()[0] if insn else ""


def diff_pairs(stripped_o: str, ref_o: str, func: str):
    target = score.normalized_insns(ref_o, func, mask=False)
    built = score.normalized_insns(stripped_o, func, mask=False)
    sm = difflib.SequenceMatcher(a=target, b=built, autojunk=False)
    pairs = [(tag, target[i1:i2], built[j1:j2])
             for tag, i1, i2, j1, j2 in sm.get_opcodes() if tag != "equal"]
    return pairs, len(target), len(built)


def _is_branch(insn: str) -> bool:
    return bool(score._BRANCH.match(_opcode(insn)))


def classify(pairs) -> tuple[str, str]:
    flat_t = [x for _, t, _ in pairs for x in t]
    flat_b = [x for _, _, b in pairs for x in b]
    allins = flat_t + flat_b
    n = max(len(flat_t), len(flat_b))
    if any(_MFHI.match(x) for x in allins):
        return "CANONICAL-ASM", "mfhi/mflo synthetic intermediate (no C variable holds it)"
    # single 1-instruction replacement
    if len(pairs) == 1 and pairs[0][0] == "replace" and len(flat_t) == 1 and len(flat_b) == 1:
        t, b = flat_t[0], flat_b[0]
        if _opcode(t) == _opcode(b):
            if _LOADSTORE.match(t):
                return "PLATEAU", f"single load/store addressing/reg diff (Pattern 2): {t} | {b}"
            if not _is_branch(t):
                return "SINGLE-REG", f"1-insn single-reg diff: target[{t}] built[{b}]"
    # control-flow-dominated: mostly branch/jump differences = label rename /
    # branch retarget / reorder — usually link-critical or SOTN-accepted regfix
    nbranch = sum(1 for x in allins if _is_branch(x))
    if allins and nbranch >= 0.6 * len(allins):
        return "CONTROL-FLOW", f"{nbranch}/{len(allins)} diffs are branch/jump (rename/retarget/reorder — usually link-critical or accepted)"
    if n <= 4:
        return "MATCHABLE", f"{n} non-control-flow differing insn(s) — worth permuting"
    return "LARGE", f"{n} differing insn(s) — deep restructure (likely accepted regfix or PARK)"


def diagnose(func: str, workdir: str = "tmp/scan") -> dict:
    stem = sandbox.func_file(func)
    stripped_o = f"{workdir}/{stem}/{stem}.o"
    ref_o = f"build/src/{stem}.o"
    pairs, nt, nb = diff_pairs(stripped_o, ref_o, func)
    verdict, reason = classify(pairs)
    return {"func": func, "file": stem, "verdict": verdict, "reason": reason,
            "ndiff": sum(max(len(t), len(b)) for _, t, b in pairs),
            "target_insns": nt, "built_insns": nb,
            "pairs": [(tag, t, b) for tag, t, b in pairs]}
