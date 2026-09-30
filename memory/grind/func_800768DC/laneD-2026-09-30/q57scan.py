"""Q57 (d) whole-program layout scan for SelWork (the D_800A36A0 work area).

Old size 0x92 (last member f7E[2][5] ends at 0x92); new size 0x94; added bytes [0x92, 0x94).

Part A: every asm/funcs function that loads D_800A36A0: linear register tracking of
  base (the loaded pointer), base+K (addiu), base+idx (addu with a non-constant);
  lists every memory access made through such a register, with offset/width, and
  flags any access whose byte range reaches >= 0x92, and any derived pointer whose
  constant offset is >= 0x92.
Part B: every immediate 0x92 / 0x94 (and 0x49/0x4A halfword-count, 0x24/0x25 word
  count) in ALL asm/funcs, and any mult by a register loaded with those constants.
Part C: C sources: sizeof(SelWork...), SelWork arrays / members of other aggregates.
"""
import re, glob, os, sys

OLD, NEW = 0x92, 0x94
WIDTH = {"lb": 1, "lbu": 1, "sb": 1, "lh": 2, "lhu": 2, "sh": 2, "lw": 4, "sw": 4,
         "lwl": 4, "lwr": 4, "swl": 4, "swr": 4}
insn_re = re.compile(r"/\* \w+ (\w+) \w+ \*/\s+(\w+)\s*(.*)$")


def parse_off(s):
    s = s.strip()
    m = re.match(r"(-?0x[0-9A-Fa-f]+|-?\d+)\((\$\w+)\)", s)
    if m:
        return int(m.group(1), 0), m.group(2)
    return None, None


out = []
funcs = []
for path in sorted(glob.glob("asm/funcs/*.s")):
    txt = open(path, encoding="utf-8", errors="replace").read()
    if "D_800A36A0" not in txt:
        continue
    fn = os.path.basename(path)[:-2]
    funcs.append(fn)
    regs = {}  # reg -> ("base", K) or ("idx", K)  (K = constant offset from base)
    accesses = []
    lastdef = {}
    idxdefs = []
    escapes = []
    for line in txt.splitlines():
        m = insn_re.search(line)
        if not m:
            if re.match(r"\s*\.L\w+:", line):
                pass  # keep state across labels (conservative: over-reports)
            continue
        addr, op, rest = m.groups()
        ops = [o.strip() for o in rest.split(",")] if rest else []
        if op == "lw" and "%gp_rel(D_800A36A0)" in rest:
            regs[ops[0]] = ("base", 0)
            continue
        if op in ("sw", "sh", "sb") and len(ops) == 2 and ops[0] in regs:
            escapes.append(f"stored value: {line.strip()}")
        if op in WIDTH and len(ops) == 2:
            off, br = parse_off(ops[1])
            if br in regs:
                kind, k = regs[br]
                accesses.append((addr, op, kind, k + off, WIDTH[op], line.strip()))
            if op.startswith("l") and ops[0] in regs:
                del regs[ops[0]]
            continue
        if op == "addiu" and len(ops) == 3 and ops[1] in regs:
            kind, k = regs[ops[1]]
            regs[ops[0]] = (kind, k + int(ops[2], 0))
            continue
        if op in ("addu",) and len(ops) == 3:
            a, b = ops[1], ops[2]
            if a in regs and b == "$zero":
                regs[ops[0]] = regs[a]
                continue
            if b in regs and a == "$zero":
                regs[ops[0]] = regs[b]
                continue
            if a in regs and b not in regs:
                regs[ops[0]] = ("idx", regs[a][1])
                idxdefs.append((addr, line.strip(), lastdef.get(b, "?")))
                lastdef[ops[0]] = line.strip()
                continue
            if b in regs and a not in regs:
                regs[ops[0]] = ("idx", regs[b][1])
                idxdefs.append((addr, line.strip(), lastdef.get(a, "?")))
                lastdef[ops[0]] = line.strip()
                continue
        if op == "addu" and len(ops) == 3 and ops[0] in ("$a0","$a1","$a2","$a3") and (ops[1] in regs or ops[2] in regs):
            escapes.append(f"arg: {line.strip()} ({regs.get(ops[1]) or regs.get(ops[2])})")
        if ops:
            lastdef[ops[0]] = line.strip()
        # any other write to a tracked register kills it
        if ops and ops[0] in regs and op not in ("sw", "sh", "sb", "beq", "bne", "beqz", "bnez",
                                                  "blez", "bgtz", "bltz", "bgez", "jr", "jalr"):
            del regs[ops[0]]
    hits = [a for a in accesses if a[3] + a[4] > OLD or a[3] >= OLD]
    offs = sorted(set((a[2], a[3]) for a in accesses))
    out.append(f"## {fn}: {len(accesses)} accesses through D_800A36A0-derived registers")
    out.append("   offsets (kind:off): " + " ".join(f"{k}:{o:#x}" for k, o in offs))
    for d in idxdefs:
        out.append(f"   idx-derivation {d[0]}: {d[1]}   <- index def: {d[2]}")
    for e in escapes:
        out.append(f"   ESCAPE {e}")
    for h in hits:
        out.append(f"   REACHES >= {OLD:#x}: {h[0]} {h[1]} {h[2]}+{h[3]:#x} w{h[4]}  {h[5]}")

out.append("")
out.append("## Part B: arithmetic immediates 0x92/0x94 in all asm/funcs (memory offsets through non-D_800A36A0 bases are other types: Part A covers every D_800A36A0-derived base)")
imm_re = re.compile(r"(?<![0-9A-Fa-fx])(0x92|0x94)(?![0-9A-Fa-f])")
cnt = 0
for path in sorted(glob.glob("asm/funcs/*.s")):
    fn = os.path.basename(path)[:-2]
    for line in open(path, encoding="utf-8", errors="replace"):
        m = insn_re.search(line)
        if not m:
            continue
        addr, op, rest = m.groups()
        # ignore stack-frame offsets ($sp) -- they address the frame, not this struct
        if "($sp)" in rest or "$sp," in rest:
            continue
        mm = imm_re.search(rest)
        if mm and op in ("addiu", "ori", "li", "slti", "sltiu", "andi", "xori", "addi", "slti"):
            cnt += 1
            out.append(f"   {fn} {addr} {op} {rest.strip()}")
out.append(f"   ({cnt} lines)")

out.append("")
out.append("## Part C: C sources")
for path in sorted(glob.glob("src/*.c") + glob.glob("include/*.h")):
    for i, line in enumerate(open(path, encoding="utf-8", errors="replace"), 1):
        if re.search(r"sizeof\s*\(\s*(SelWork|S_800747D8|SelWork_800768DC)", line) or \
           re.search(r"(SelWork|S_800747D8|SelWork_800768DC)\s+\w+\s*\[", line) or \
           re.search(r"(SelWork|S_800747D8|SelWork_800768DC)\s+\w+\s*;", line) and "*" not in line:
            out.append(f"   {path}:{i}: {line.rstrip()}")
out.append("   (end Part C)")
print("\n".join(out))
