"""Function-level asm scorer for the cheat-invisible sandbox.

Compares a cheat-disabled build of a function against the canonical (cheat-on,
byte-correct) build AT THE OBJECT LEVEL — both are .o files, so relocations
render identically and there are no false diffs from unresolved symbols. Control-
flow targets (branch/jump destinations) are masked so the score is
cascade-immune: removing an instruction shifts later addresses but not the
normalized instruction form.

score = instruction edit-distance (SequenceMatcher); 0 == instruction-identical.
This GUIDES the loop. verify-integrated (full-build SHA1) is the real match gate
— a masked-target false-zero is impossible to commit because integration checks
actual bytes.
"""
from __future__ import annotations

import re

from . import buildconfig as cfg
from . import pipeline as P

# objdump -t function line:  OFFSET <flags> F <section> SIZE NAME
_SYMOFF_RE = re.compile(r"^([0-9a-fA-F]+)\s+\S.*\sF\s+\S+\s+([0-9a-fA-F]+)\s+(\S+)\s*$")
# objdump -d instruction line:  "   <addr>:\t<bytes>\t<mnemonic>\t<operands>"
_INSN_RE = re.compile(r"^\s*[0-9a-f]+:\t[0-9a-f ]+\t(\S+)\s*(.*)$")
_BRANCH = re.compile(r"^(b|bal|beq|bne|blez|bgtz|bltz|bgez|beqz|bnez|j|jal|jr|jalr)\b")


def _o_func_table(o_path: str) -> dict[str, tuple[int, int]]:
    """name -> (offset, size) for functions in a .o; size recomputed from the
    next function's offset when the symbol size is 0 (maspsx omits .size)."""
    out = P.sh(f"{cfg.OBJDUMP} -t {o_path}", capture_output=True, text=True).stdout
    funcs = {}
    for line in out.splitlines():
        m = _SYMOFF_RE.match(line)
        if m:
            funcs[m.group(3)] = (int(m.group(1), 16), int(m.group(2), 16))
    offs = sorted({o for o, _ in funcs.values()})
    nxt = {o: offs[i + 1] for i, o in enumerate(offs[:-1])}
    # section end as a backstop for the last function
    end = _section_size(o_path, ".text")
    return {
        n: (o, (s if s else (nxt.get(o, end) - o if (nxt.get(o, end) or 0) > o else 0)))
        for n, (o, s) in funcs.items()
    }


def _section_size(o_path: str, section: str) -> int:
    out = P.sh(f"{cfg.OBJDUMP} -h {o_path}", capture_output=True, text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] == section:
            return int(parts[2], 16)
    return 0


def normalized_insns(o_path: str, func: str) -> list[str]:
    tbl = _o_func_table(o_path)
    if func not in tbl:
        raise KeyError(f"{func} not found in {o_path}")
    off, size = tbl[func]
    cmd = (f"{cfg.OBJDUMP} -dr --start-address={off} "
           f"--stop-address={off + size} {o_path}")
    out = P.sh(cmd, capture_output=True, text=True).stdout
    insns = []
    for line in out.splitlines():
        m = _INSN_RE.match(line)
        if not m:
            continue  # labels, reloc lines, section headers
        mn, ops = m.group(1), m.group(2).strip()
        ops = re.split(r"\s+<", ops)[0]      # drop "<sym+0x..>" annotation
        ops = ops.split("#")[0].strip()       # drop trailing comment
        if _BRANCH.match(mn):
            parts = ops.split(",")
            if parts:
                parts[-1] = "@"               # mask control-flow target
            ops = ",".join(parts)
        insns.append(f"{mn} {ops}".strip())
    return insns


def _levenshtein(a: list[str], b: list[str]) -> int:
    """Exact minimal edit distance (insert/delete/substitute) between two
    instruction lists. SequenceMatcher's greedy block-matching over-counts when
    deletions shift positions (an identical line gets scored as delete+insert);
    the DP gives the true minimum. Lists are short (<~few hundred insns)."""
    m, n = len(a), len(b)
    prev = list(range(n + 1))
    for i in range(1, m + 1):
        cur = [i] + [0] * n
        ai = a[i - 1]
        for j in range(1, n + 1):
            cost = 0 if ai == b[j - 1] else 1
            cur[j] = min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost)
        prev = cur
    return prev[n]


def score_func(cheat_disabled_o: str, reference_o: str, func: str) -> dict:
    """Edit-distance between the cheat-disabled function and the canonical
    (cheat-on) function. reference_o is build/src/<file>.o (byte-correct).
    0 == instruction-identical (modulo masked control-flow targets)."""
    target = normalized_insns(reference_o, func)
    built = normalized_insns(cheat_disabled_o, func)
    return {"score": _levenshtein(target, built),
            "target_insns": len(target), "build_insns": len(built)}
