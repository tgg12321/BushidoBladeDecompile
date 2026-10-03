#!/usr/bin/env python3
"""rrscan: register+register addu/or sites in the ORIGINAL binary whose two
operands are provably bit-disjoint by producer tracing.

For each `addu rd, rs, rt` / `or rd, rs, rt` (rs, rt real registers, not $zero)
trace each operand's producer backward (in-block first; if it crosses a label
or a branch, keep going linearly but flag the site XBLOCK). Masks are derived
recursively from producers (andi, lbu, lhu, sll, srl, slt*, lui, li, and/or/xor,
moves). Calls kill $v0/$v1/$a*/$t* (caller-saved) -> producer unknown.
Also classify the CONSUMER of rd (first reader in the forward direction).
Output: argv[1] (default tmp/rrscan/sites.tsv). Columns: state (C/ASM at scan
time), function, address, op, operands, maskA, producerA, maskB, producerB,
BLOCK/XBLOCK, consumer of rd, Z (an operand provably zero).

Committed 2026-09-25 as the register-plus-register scanner required by owner
ruling bcdc1648e item (A) (docs/ORACLE-COMPILER.md). Run from the repo root:
    python3 tools/rrscan_plus_ior.py [out.tsv]
"""
import sys
import glob, os, re, collections

FULL = 0xFFFFFFFF
INSN = re.compile(r"/\*\s+([0-9A-F]+)\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s+\*/\s+(\S+)\s*(.*)$")
CALLER_SAVED = {"$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3",
                "$t4", "$t5", "$t6", "$t7", "$t8", "$t9", "$at", "$ra"}
STORES = {"sb", "sh", "sw", "swl", "swr", "swc2"}
LOADS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "lwc2"}
BR = re.compile(r"^(b[a-z]*|j|jr|jal|jalr)$")


def imm(s):
    s = s.strip()
    if s.startswith("%"):
        return None
    try:
        return int(s, 0)
    except ValueError:
        return None


def parse(path):
    items = []  # ("label", name) | ("insn", addr, op, ops)
    for ln in open(path, encoding="utf-8", errors="replace").read().splitlines():
        s = ln.strip()
        if s.endswith(":") and not s.startswith("/*"):
            items.append(("label", s[:-1]))
            continue
        if s.startswith("glabel") or s.startswith("jlabel"):
            items.append(("label", s.split()[1]))
            continue
        mo = INSN.search(ln)
        if not mo:
            continue
        rest = mo.group(5).split("#")[0].strip()
        ops = [o.strip() for o in rest.split(",")] if rest else []
        items.append(("insn", mo.group(2), mo.group(4), ops))
    return items


def dest_of(op, ops):
    if not ops or op in STORES or BR.match(op) or op in ("mult", "multu", "div", "divu",
                                                        "mtc2", "ctc2", "nop", "break",
                                                        "syscall", "mthi", "mtlo", "cop2"):
        if op in ("jal", "jalr"):
            return "CALL"
        return None
    if op in ("mfc2", "cfc2"):
        return ops[0]
    d = ops[0]
    return d if d.startswith("$") else None


def srcs_of(op, ops):
    if op in STORES:
        r = [ops[0]]
        m = re.search(r"\((\$\w+)\)", ops[1]) if len(ops) > 1 else None
        if m:
            r.append(m.group(1))
        return r
    if op in LOADS:
        m = re.search(r"\((\$\w+)\)", ops[1]) if len(ops) > 1 else None
        return [m.group(1)] if m else []
    if op in ("jr", "jalr"):
        return [ops[-1]]
    if op in ("mult", "multu", "div", "divu"):
        return [o for o in ops if o.startswith("$")]
    if op.startswith("b"):
        return [o for o in ops if o.startswith("$")]
    if op in ("mtc2", "ctc2", "mthi", "mtlo"):
        return [ops[0]]
    return [o for o in ops[1:] if o.startswith("$")]


class Func:
    def __init__(self, path):
        self.name = os.path.basename(path)[:-2]
        self.items = parse(path)
        self.insns = [i for i, it in enumerate(self.items) if it[0] == "insn"]

    def producer(self, idx, reg, depth=0):
        """Walk back from item idx for the writer of reg.
        Returns (item_index or None, crossed_block:bool, note)."""
        crossed = False
        i = idx - 1
        # delay-slot handling: an insn right after a branch belongs to the branch's block
        while i >= 0:
            it = self.items[i]
            if it[0] == "label":
                if it[1] == self.name:
                    return None, crossed, "entry"
                crossed = True
                i -= 1
                continue
            _, addr, op, ops = it
            d = dest_of(op, ops)
            if d == "CALL":
                if reg in CALLER_SAVED:
                    return None, crossed, "call"
                # the insn after jal (delay slot) executes before the call; it is
                # later in the list so it was already scanned.
            elif d == reg:
                return i, crossed, ""
            if BR.match(op) and op not in ("jal", "jalr") and i != idx - 1:
                # a branch above us (not our own delay slot parent) = block boundary
                crossed = True
            i -= 1
        return None, crossed, "none"

    def mask(self, idx, reg, depth=0, trail=None):
        """(mask, provenance-string, crossed) for reg as read at item idx."""
        if reg == "$zero":
            return 0, "zero", False
        if depth > 6:
            return FULL, "?", False
        p, crossed, note = self.producer(idx, reg)
        if p is None:
            return FULL, f"?({note})", crossed
        _, addr, op, ops = self.items[p]
        def sub(r):
            m, pr, c = self.mask(p, r, depth + 1)
            return m, pr, c
        prov = f"{op}@{addr}"
        m = FULL
        c2 = False
        try:
            if op == "andi":
                sm, _, c2 = sub(ops[1])
                m = imm(ops[2]) & sm
                prov = f"andi {ops[2]}@{addr}"
            elif op == "ori" or op == "xori":
                sm, sp, c2 = sub(ops[1])
                v = imm(ops[2])
                m = sm | v if v is not None else FULL
                prov = f"{op}({sp})@{addr}"
            elif op == "addiu" and ops[1] == "$zero":
                v = imm(ops[2])
                m = (v & FULL) if v is not None else FULL
                prov = f"li {ops[2]}@{addr}"
            elif op == "lui":
                v = imm(ops[1])
                m = ((v << 16) & FULL) if v is not None else FULL
                prov = f"lui@{addr}"
            elif op == "lbu":
                m, prov = 0xFF, f"lbu@{addr}"
            elif op == "lhu":
                m, prov = 0xFFFF, f"lhu@{addr}"
            elif op == "sll":
                sm, sp, c2 = sub(ops[1])
                m = (sm << imm(ops[2])) & FULL
                prov = f"sll {ops[2]}({sp})@{addr}"
            elif op == "srl":
                sm, sp, c2 = sub(ops[1])
                m = sm >> imm(ops[2])
                prov = f"srl {ops[2]}({sp})@{addr}"
            elif op == "sra":
                sm, sp, c2 = sub(ops[1])
                if not (sm & 0x80000000):
                    m = sm >> imm(ops[2])
                prov = f"sra {ops[2]}({sp})@{addr}"
            elif op in ("slt", "sltu", "slti", "sltiu"):
                m, prov = 1, f"{op}@{addr}"
            elif op == "and":
                a, pa, ca = sub(ops[1]); b, pb, cb = sub(ops[2]); c2 = ca or cb
                m = a & b
                prov = f"and({pa},{pb})@{addr}"
            elif op in ("or", "xor"):
                a, pa, ca = sub(ops[1]); b, pb, cb = sub(ops[2]); c2 = ca or cb
                m = a | b
                prov = f"{op}({pa},{pb})@{addr}"
            elif op == "addu" and ops[2] == "$zero":
                m, sp, c2 = sub(ops[1])
                prov = f"move({sp})@{addr}"
            elif op == "addu" and ops[1] == "$zero":
                m, sp, c2 = sub(ops[2])
                prov = f"move({sp})@{addr}"
            elif op == "mflo" or op == "mfhi" or op in LOADS:
                prov = f"{op}@{addr}"
            else:
                prov = f"{op}@{addr}"
        except (TypeError, IndexError, ValueError):
            m = FULL
        return m, prov, crossed or c2

    def consumer(self, idx, rd):
        """Classify how rd (written at item idx) is consumed."""
        # delay slot of a call?
        prev = idx - 1
        while prev >= 0 and self.items[prev][0] == "label":
            prev -= 1
        if prev >= 0 and self.items[prev][0] == "insn":
            pop = self.items[prev][2]
            if pop in ("jal", "jalr") and rd in ("$a0", "$a1", "$a2", "$a3"):
                return f"CALL-ARG({self.items[prev][3][-1]})"
            if pop == "jr" and self.items[prev][3] == ["$ra"] and rd in ("$v0", "$v1"):
                return "RETURN(delay)"
        i = idx + 1
        crossed = False
        while i < len(self.items):
            it = self.items[i]
            if it[0] == "label":
                crossed = True
                i += 1
                continue
            _, addr, op, ops = it
            tag = "x" if crossed else ""
            if rd in srcs_of(op, ops):
                if op == "jr":
                    return "RETURN-ish(jr)" + tag
                if op in STORES and ops[0] == rd:
                    return f"STORE-{op}{tag}"
                if op == "addu" and "$zero" in ops[1:]:
                    return f"COPY->{ops[0]}{tag}"
                if op.startswith("b"):
                    return f"BRANCH-{op}{tag}"
                return f"ARITH-{op}{tag}"
            if op in ("jal", "jalr"):
                # the delay slot insn runs first; check it
                nxt = self.items[i + 1] if i + 1 < len(self.items) else None
                if nxt and nxt[0] == "insn" and rd in srcs_of(nxt[2], nxt[3]):
                    return f"(dslot)ARITH-{nxt[2]}"
                if rd in ("$a0", "$a1", "$a2", "$a3"):
                    return f"CALL-ARG({ops[-1]}){tag}"
                if rd in CALLER_SAVED:
                    return f"DEAD-at-call{tag}"
            if op == "jr" and ops == ["$ra"]:
                nxt = self.items[i + 1] if i + 1 < len(self.items) else None
                if nxt and nxt[0] == "insn" and rd in srcs_of(nxt[2], nxt[3]):
                    return f"(dslot)ARITH-{nxt[2]}"
                if rd == "$v0":
                    return "RETURN" + tag
                return "DEAD-at-return"
            d = dest_of(op, ops)
            if d == rd:
                return "DEAD(overwritten)" + tag
            i += 1
        return "END"


def main():
    asm = set()
    for c in glob.glob("src/**/*.c", recursive=True):
        asm |= set(re.findall(r'INCLUDE_ASM\("asm/funcs", (\w+)\);', open(c, encoding="utf-8", errors="replace").read()))
    rows = []
    for path in sorted(glob.glob("asm/funcs/*.s")):
        f = Func(path)
        for i, it in enumerate(f.items):
            if it[0] != "insn":
                continue
            _, addr, op, ops = it
            if op not in ("addu", "or") or len(ops) != 3:
                continue
            rd, rs, rt = ops
            if "$zero" in (rs, rt):
                continue
            if rs == rt:
                continue
            a, pa, ca = f.mask(i, rs)
            b, pb, cb = f.mask(i, rt)
            if a == FULL or b == FULL or (a & b):
                continue
            if a == 0 or b == 0:
                zero = "Z"
            else:
                zero = ""
            st = "ASM" if f.name in asm else "C"
            rows.append((st, f.name, addr, op, ", ".join(ops), hex(a), pa, hex(b), pb,
                         "XBLOCK" if (ca or cb) else "BLOCK", f.consumer(i, rd), zero))
    out = sys.argv[1] if len(sys.argv) > 1 else "tmp/rrscan/sites.tsv"
    with open(out, "w", newline="\n") as fo:
        for r in rows:
            fo.write("\t".join(r) + "\n")
    cnt = collections.Counter((r[0], r[3], r[9]) for r in rows)
    for k, v in sorted(cnt.items()):
        print(k, v)


main()
