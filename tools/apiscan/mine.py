#!/usr/bin/env python3
"""apiscan/mine.py — read-only evidence miner for the API-restatement naming class.

For every function in asm/funcs/ it records
  * the VERIFIED (libscan-verbatim) library entry points the body calls, and
  * every C string the body references (resolved through %hi/%lo pairs and
    lui/addiu immediates, read straight out of disc/SLUS_006.63),
and writes docs/naming/apiscan/evidence.json. The rename manifest next to it
(rename_manifest.csv) is hand-curated FROM this evidence and verified per row by
a fresh adversarial agent; this tool never proposes names.

Usage:  python3 tools/apiscan/mine.py            (from the repo root or anywhere)
"""
from __future__ import annotations

import csv
import glob
import json
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EXE = os.path.join(ROOT, "disc", "SLUS_006.63")
OUT = os.path.join(ROOT, "docs", "naming", "apiscan", "evidence.json")
CENSUS = os.path.join(ROOT, "docs", "naming", "function-names.csv")
LOAD, HDR = 0x80010000, 0x800

INSN = re.compile(r"^\s*/\* [0-9A-F]+ (8[0-9A-F]{7}) ([0-9A-F]{8}) \*/\s+(\S+)\s*(.*)$", re.M)
SYMDEF = re.compile(r"\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)")


def main() -> int:
    if not os.path.exists(EXE):
        print("missing disc/SLUS_006.63 (gitignored disc extraction)", file=sys.stderr)
        return 2
    exe = open(EXE, "rb").read()

    def cstr(addr: int):
        o = addr - LOAD + HDR
        if not (0 <= o < len(exe)):
            return None
        b = exe[o:o + 200]
        i = b.find(b"\0")
        if i < 3:
            return None
        s = b[:i]
        return s.decode() if all(32 <= c < 127 or c in (9, 10, 13) for c in s) else None

    sym2addr: dict[str, int] = {}
    for f in ("symbol_addrs.txt", "named_syms.txt", "undefined_syms_auto.txt", "undefined_funcs_auto.txt"):
        p = os.path.join(ROOT, f)
        if not os.path.exists(p):
            continue
        for line in open(p, encoding="utf-8", errors="replace"):
            m = SYMDEF.match(line)
            if m:
                sym2addr[m.group(1)] = int(m.group(2), 16)

    verified = set()
    if os.path.exists(CENSUS):
        for r in csv.DictReader(open(CENSUS, encoding="utf-8", errors="replace")):
            if r.get("tier") == "VERIFIED":
                verified.add(r["current_name"])
                if r.get("glabel"):
                    verified.add(r["glabel"])

    out = {}
    for p in sorted(glob.glob(os.path.join(ROOT, "asm", "funcs", "*.s"))):
        txt = open(p, encoding="utf-8", errors="replace").read()
        g = re.search(r"^glabel\s+(\w+)", txt, re.M)
        if not g:
            continue
        name = g.group(1)
        ins = [(int(m.group(1), 16), m.group(3), m.group(4)) for m in INSN.finditer(txt)]
        lui: dict[str, int] = {}
        refs: set[int] = set()
        calls: list[str] = []
        for _pc, op, args in ins:
            if op == "jal":
                calls.append(args.strip())
            elif op == "lui":
                m = re.match(r"(\$\w+),\s*(0x[0-9A-Fa-f]+)", args)
                if m:
                    lui[m.group(1)] = int(m.group(2), 16) << 16
                else:
                    lui.pop(args.split(",")[0].strip(), None)
            else:
                m = re.search(r"%lo\((\w+)((?:\+0x[0-9A-Fa-f]+)?)\)", args)
                if m and m.group(1) in sym2addr:
                    refs.add(sym2addr[m.group(1)] + (int(m.group(2)[1:], 16) if m.group(2) else 0))
                m3 = re.match(r"(\$\w+),\s*(\$\w+),\s*(-?0x[0-9A-Fa-f]+|-?\d+)$", args)
                if op == "addiu" and m3 and m3.group(2) in lui:
                    v = m3.group(3)
                    refs.add(lui[m3.group(2)] + (int(v, 16) if "x" in v else int(v)))
        strings = sorted({(hex(a), s) for a in refs if (s := cstr(a))})
        addr = ins[0][0] if ins else sym2addr.get(name)
        out[name] = dict(
            addr=hex(addr) if addr else None,
            insns=len(ins),
            verified_callees=sorted({c for c in calls if c in verified}),
            other_callees=sorted({c for c in calls if c not in verified}),
            strings=[dict(addr=a, text=s) for a, s in strings],
        )

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(out, fh, indent=1, sort_keys=True)
        fh.write("\n")
    n_auto = sum(1 for k in out if re.match(r"^func_8", k))
    print(f"{len(out)} functions ({n_auto} auto-named) -> {os.path.relpath(OUT, ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
