#!/usr/bin/env python3
"""Decode BIOS A0/B0/C0 vector trampolines from asm/funcs/*.s by raw instruction bytes.

Trampoline shapes:
  (1) j 0x000000A0 ; li $t1, <idx>          (delay slot)
  (2) li $t2, 0xA0 ; jr $t2 ; li $t1, <idx>
  (3) li $t1, <idx> ; j 0xA0
Anything that reaches the vector with $t1 set is a BIOS call.
"""
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from tables import TABLES

REPO = Path(__file__).resolve().parents[2]
ASM = REPO / "asm" / "funcs"

LINE = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/\s*(.*)")


def word_of(hexbytes):
    b = bytes.fromhex(hexbytes)
    return int.from_bytes(b, "little")


def decode(w):
    op = w >> 26
    rs = (w >> 21) & 31
    rt = (w >> 16) & 31
    rd = (w >> 11) & 31
    imm = w & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    tgt = (w & 0x03FFFFFF) << 2
    return op, rs, rt, rd, imm, simm, tgt


def scan(path):
    insns = []  # (addr, word, text)
    name = None
    for line in path.read_text(errors="replace").splitlines():
        s = line.strip()
        if s.startswith("glabel ") and name is None:
            name = s.split()[1]
        m = LINE.match(s)
        if m:
            insns.append((int(m.group(1), 16), word_of(m.group(2)), m.group(3).strip()))
    if name is None:
        name = path.stem
    return name, insns


def analyze(name, insns):
    """Return (table, index, vector_kind) or None."""
    if not insns:
        return None
    # track last immediate loaded into $t1 (reg 9) and $t2 (reg 10)
    hits = []
    t1val = None
    reg_imm = {}
    for i, (addr, w, txt) in enumerate(insns):
        op, rs, rt, rd, imm, simm, tgt = decode(w)
        vec = None
        if op == 0x02:  # j
            if tgt in TABLES:
                vec = tgt
        elif op == 0x00 and (w & 0x3F) == 0x08:  # jr rs
            if reg_imm.get(rs) in TABLES:
                vec = reg_imm[rs]
        if vec is not None:
            # index = $t1 value; delay slot may set it
            idx = reg_imm.get(9)
            if i + 1 < len(insns):
                dop, drs, drt, drd, dimm, dsimm, dtgt = decode(insns[i + 1][1])
                if drt == 9 and dop in (0x09, 0x0D, 0x0F):  # addiu/ori/lui into $t1
                    idx = dimm
            hits.append((vec, idx, addr))
        # update immediate tracking
        if op in (0x09, 0x0D) and rs == 0:  # addiu/ori rt, $zero, imm
            reg_imm[rt] = imm
        elif op == 0x0F:  # lui
            reg_imm[rt] = imm << 16
        elif op == 0x09 and rs in reg_imm:  # addiu rt, rs, imm
            reg_imm[rt] = (reg_imm[rs] + simm) & 0xFFFFFFFF
    return hits


def main():
    rows = []
    for path in sorted(ASM.glob("*.s")):
        name, insns = scan(path)
        hits = analyze(name, insns) or []
        if not hits:
            continue
        for vec, idx, addr in hits:
            spec = TABLES[vec].get(idx) if idx is not None else None
            rows.append(dict(
                func_addr=f"0x{insns[0][0]:08X}",
                file=path.name,
                current_name=name,
                table=f"{vec:02X}",
                index=f"0x{idx:02X}" if idx is not None else "?",
                spec_name=spec or "UNKNOWN",
                call_addr=f"0x{addr:08X}",
                n_insns=len(insns),
            ))
    out = Path(__file__).parent / "raw_hits.csv"
    with out.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    print(f"{len(rows)} BIOS call sites in {len(set(r['file'] for r in rows))} functions -> {out}")
    for r in rows:
        print(f"{r['func_addr']} {r['current_name']:<45} {r['table']}:{r['index']} = {r['spec_name']:<28} insns={r['n_insns']}")


if __name__ == "__main__":
    main()
