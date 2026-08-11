#!/usr/bin/env python3
"""Decode every raw `.word 0x...` inside __asm__ blocks of the given src files
and classify per the 2026-08-11 owner ruling:
  - cop2-internal (opcode 0x12=COP2 with co-bit set, no GPR field in play,
    e.g. mvmva/sqr/avsz/nclip/rtpt): CANONICAL in kind.
  - cop2 move/control (mfc2/mtc2/cfc2/ctc2 encoded as .word): RED FLAG —
    these have mnemonics; a .word spelling hardcodes the GPR.
  - anything else (non-cop2 opcode): INJECTION.
Also decodes .word lines in the named asm targets for cross-reference.
"""
import re, sys

GTE_FUNCS = {  # cop2 function-field names for co=1 (GTE ops)
    0x01: "rtps", 0x06: "nclip", 0x0C: "op", 0x10: "dpcs", 0x11: "intpl",
    0x12: "mvmva", 0x13: "ncds", 0x14: "cdp", 0x16: "ncdt", 0x1B: "nccs",
    0x1C: "cc", 0x1E: "ncs", 0x20: "nct", 0x28: "sqr", 0x29: "dcpl",
    0x2A: "dpct", 0x2D: "avsz3", 0x2E: "avsz4", 0x30: "rtpt",
    0x3D: "gpf", 0x3E: "gpl", 0x3F: "ncct",
}

def classify(w):
    op = (w >> 26) & 0x3F
    if op == 0x12:  # COP2
        if w & (1 << 25):  # co bit -> GTE operation, no GPR fields
            fn = w & 0x3F
            return ("COP2-INTERNAL", GTE_FUNCS.get(fn, f"gte_fn_0x{fn:02X}"))
        rs = (w >> 21) & 0x1F
        sub = {0x00: "mfc2", 0x02: "cfc2", 0x04: "mtc2", 0x06: "ctc2"}.get(rs)
        if sub:
            rt = (w >> 16) & 0x1F
            return ("COP2-MOVE-GPR", f"{sub} $r{rt} (GPR hardcoded in .word)")
        return ("COP2-OTHER", f"rs=0x{rs:02X}")
    if op == 0x32:
        return ("LWC2", f"GPR base ${(w>>21)&0x1F} hardcoded")
    if op == 0x3A:
        return ("SWC2", f"GPR base ${(w>>21)&0x1F} hardcoded")
    return ("NON-COP2", f"opcode 0x{op:02X}")

def audit_file(path):
    txt = open(path, encoding="utf-8", errors="replace").read()
    out = []
    for m in re.finditer(r'\.word\s+(0x[0-9A-Fa-f]+)', txt):
        w = int(m.group(1), 16)
        line = txt.count("\n", 0, m.start()) + 1
        cls, detail = classify(w)
        out.append((line, m.group(1), cls, detail))
    return out

for path in sys.argv[1:]:
    rows = audit_file(path)
    print(f"=== {path}: {len(rows)} .word(s)")
    from collections import Counter
    c = Counter(r[2] for r in rows)
    print("   summary:", dict(c))
    for line, wrd, cls, detail in rows:
        print(f"   L{line:5d} {wrd:>12} {cls:14} {detail}")
