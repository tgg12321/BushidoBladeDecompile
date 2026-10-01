#!/usr/bin/env python3
"""aspsx_negctl.py — negative controls, decided by Sony's ASPSX 2.34:
 (1) the two contradiction TUs UNSPLIT (one file defining everything any of its functions reaches gp):
     ASPSX gives gp to the accesses the shipped bytes have non-gp (func_80044800 / D_800A3820,
     func_800343F0 / D_800A3140) -> those files cannot have been one original file;
 (2) every POC file with its definitions removed (all extern, as today's C declares): ASPSX gp count.
usage (WSL, repo root): python3 tmp/q56/aspsx_negctl.py -> tmp/q56/aspsx_negctl.txt"""
import re, sys
sys.path.insert(0, "tmp/q56")
import aspsx_wholeprog as W
from pathlib import Path
rep = []
for tu, sym in (("text1a_c", "D_800A3820"), ("code6cac_b_tu2", "D_800A3140")):
    p, k, ins = W.probe(f"negctl_unsplit_{tu}", W.stream(tu))
    a = W.aspsx_decisions(p, k)
    hits = [(ins[i].strip(), a[i]) for i in range(k) if re.search(r",%s\b" % sym, ins[i])]
    rep.append(f"(1) {tu} unsplit: accesses to {sym} and ASPSX's gp decision: {hits}")
tot = gp = 0
for c in sorted(Path(W.M + "/src").glob("*.c")):
    text = "\n".join(l for l in W.stream(c.stem).split("\n") if not l.startswith(("\t.comm", "\t.lcomm")))
    p, k, ins = W.probe(f"negctl_extern_{c.stem}", text)
    if k:
        a = W.aspsx_decisions(p, k); tot += k; gp += sum(a)
rep.append(f"(2) all files, definitions removed (extern only): {tot} accesses, ASPSX gp = {gp}")
open("tmp/q56/aspsx_negctl.txt", "w", newline="\n").write("\n".join(rep) + "\n")
print("\n".join(rep))
