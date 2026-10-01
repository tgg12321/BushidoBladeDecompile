#!/usr/bin/env python3
"""conflict_info.py <tree> sym...: for each symbol, every declaration / definition line in src/*.c and
include/*.h (file:line), and in the Q67 member files every use line. Evidence gathering for the M3/M4
declaration reconciliation."""
import os, re, sys
T = sys.argv[1]
MEMBERS = {"text1a_c2", "text1a_b", "text1a_b_pre_rodata", "sound", "text1b", "text1b_tu2", "text1a_b_mid_rodata", "text1b_b"}
files = [f"src/{f}" for f in sorted(os.listdir(f"{T}/src")) if f.endswith(".c")] + \
        [f"include/{f}" for f in sorted(os.listdir(f"{T}/include")) if f.endswith(".h")]
for sym in sys.argv[2:]:
    print(f"==== {sym}")
    pat = re.compile(r"\b%s\b" % re.escape(sym))
    for f in files:
        L = open(f"{T}/{f}", errors="replace").read().split("\n")
        stem = os.path.basename(f)[:-2]
        for i, l in enumerate(L, 1):
            if not pat.search(l):
                continue
            s = l.strip()
            decl = s.startswith("extern") or re.match(r"^[A-Za-z_][\w\s\*]*\b%s\s*\(" % re.escape(sym), s) and s.endswith((";", "{")) \
                or re.match(r"^(const\s+)?[A-Za-z_][\w\s\*]*\b%s\b(\s*\[[^\]]*\])*\s*(=|;)" % re.escape(sym), s)
            if decl or stem in MEMBERS:
                tag = "DECL" if decl else "use "
                print(f"  {tag} {f}:{i}: {s[:150]}")
