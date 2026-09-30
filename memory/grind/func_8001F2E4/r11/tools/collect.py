"""collect.py <dumps-dir> <out dumps.txt> : Ruling 11 (D)(1) excerpts for func_8001F2E4.
Reads the per-variant dump dirs written by dump.sh (+ conf.py tables) and writes the excerpts:
commands, instcheck, allocation tables, FINDREG blocks, .lreg lines, and the cse excerpts of the
three `/ 8` expansions and of the func_8002F770 argument moves."""
import os
import re
import subprocess
import sys

NL = chr(10)
D = sys.argv[1]
out = []
P = out.append


def rd(p):
    return open(p, encoding="utf-8", errors="replace").read()


def sect(t):
    P("")
    P("=" * 100)
    P(t)
    P("=" * 100)


sect("COMMANDS (tools/dump.sh; every variant): cmd.txt of R_base")
P(rd(os.path.join(D, "R_base", "cmd.txt")).rstrip())
P("instcheck (instrumented cc1 .s == build cc1 .s, per variant):")
for v in sorted(os.listdir(D)):
    p = os.path.join(D, v, "instcheck.txt")
    if os.path.exists(p):
        P("  %-22s %s" % (v, rd(p).strip()))

sect("ALLOCATION TABLES (tools/conf.py: pseudo, hard reg, global.c ALLOCDBG order/priority or local-alloc, "
     ".greg hard-register conflicts). Register numbers: 2 v0, 3 v1, 4 a0, 5 a1, 16 s0 ... 18 s2")
for v in sorted(os.listdir(D)):
    t = os.path.join(D, v, "table.txt")
    if os.path.exists(t):
        P("--- " + v)
        P(rd(t).rstrip())

sect("FINDREG (global.c find_reg, BB2_FINDREG_DEBUG=<pseudo>; func_8001F2E4 block only)")
for v in sorted(os.listdir(D)):
    for f in sorted(os.listdir(os.path.join(D, v))):
        m = re.match(r"findreg_(\d+)\.txt$", f)
        if not m:
            continue
        s = rd(os.path.join(D, v, f)).split(NL)
        blk = []
        for ln in s:
            if ln.startswith("FINDREGDBG func=") and blk:
                break
            blk.append(ln)
        P("--- %s pseudo %s" % (v, m.group(1)))
        P(NL.join(x for x in blk if x.strip()))

sect(".lreg per-register lines (flow.c: 'in block N' = referenced in one basic block, local-alloc "
     "candidate, local-alloc.c:472; no block = REG_BLOCK_GLOBAL, flow.c:2072-2075/2508-2511) and "
     "local-alloc dispositions (';; Register N in R.')")
WANT = {"R_base": [78, 79, 80, 81, 82], "split_all": [78, 79, 80, 81, 98, 99, 188, 214, 215, 216, 217, 281, 282],
        "only_V5": [82, 275], "only_V6": [82, 275], "only_XZ2": [80, 81, 212, 213]}
for v, regs in WANT.items():
    p = os.path.join(D, v, "tu.i.lreg.fn")
    if not os.path.exists(p):
        continue
    s = rd(p)
    P("--- " + v)
    for r in regs:
        for m in re.finditer(r"^(?:;; )?Register %d (?:used|in).*$" % r, s, re.M):
            P("  " + m.group(0))


def div_blocks(s):
    """each signed `/ 8` expansion: from the quotient copy insn through the ashiftrt insn."""
    res = []
    for m in re.finditer(r"\(const_int 7\)\)\)", s):
        a = s.rfind("\n(insn ", 0, s.rfind("\n(jump_insn ", 0, m.start()))
        e = s.find("(const_int 3))", m.end())
        e = s.find("\n\n", e)
        res.append(s[a + 1:e])
    return res


sect("cse.c make_regs_eqv (cse.c:841-855) at the three `x / 8` expansions (expmed.c: quotient = copy of x; "
     "if (quotient >= 0) skip; quotient += 7; result = quotient >> 3). .rtl = before cse, .cse = after cse1. "
     "In R_base cse rewrites the branch test and the +7 to read the variable (reg/v); where the variable's "
     "last reference is the copy itself (per-value spellings), the quotient pseudo stays canonical.")
for v in ["R_base", "split_all", "only_V1", "only_V2", "only_V4", "only_V56"]:
    for kind in ["rtl", "cse"]:
        p = os.path.join(D, v, "tu.i.%s.fn" % kind)
        if not os.path.exists(p):
            continue
        for k, b in enumerate(div_blocks(rd(p))):
            P("--- %s .%s division %d (%s)" % (v, kind, k + 1, ["obj+0x1E6", "obj+0x1E8", "obj+0x1EA"][k] if k < 3 else "?"))
            P(b)

sect("tgt_x: the func_8002F770 4th-argument moves after cse1 (R_base: from the pseudo; T3_after_join: "
     "folded to the constant)")
for v in ["R_base", "T2_init_once", "T3_after_join"]:
    p = os.path.join(D, v, "tu.i.cse.fn")
    if not os.path.exists(p):
        continue
    s = rd(p)
    P("--- " + v)
    for m in re.finditer(r"\(insn \d+ \d+ \d+ \(set \(reg:SI 7 a3\)\n.*\n.*\n", s):
        P(m.group(0).rstrip())
open(sys.argv[2], "w", newline=NL).write(NL.join(out) + NL)
print("wrote", sys.argv[2], len(out), "blocks")
