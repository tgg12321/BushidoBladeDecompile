#!/usr/bin/env python3
"""Step 9: M3 merge (Q65/Q67, per-file-gp-model.md "Merge"): text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b
are one file, text1b.c. Verbatim merge in link order (mergec.py; verbatim-identical re-declarations dropped);
the declarations were reconciled in the previous step.

Evidence (per-file-gp-model.md, recorded as rodata-object-alignment section 7; tmp/q56/adopt/m34_evidence.md):
the PSYLINK probe (a file's statics are one contiguous .lcomm block in link order), the static-region gp
reach per group, mergecheck2 (no contradiction inside the merged file), and the jump-table phase check (all
members phase 4: no rodata-align boundary is removed).
usage: s09_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
parts = ["text1a_c2", "text1a_b", "text1a_b_pre_rodata", "sound", "text1b"]
run(f"{H}/mergec.py", "src/text1b.c", *[f"src/{p}.c" for p in parts])
for p in parts[:-1]:
    os.remove(f"src/{p}.c")
ld_merge("text1b", parts)
t = rd("src/text1b.c")
assert "(owner ruling Q65: one original file)" in t
wr("src/text1b.c", t.replace("(owner ruling Q65: one original file)", "(owner ruling Q67: one original file)"))
# rodata-align record, section 9 table: text1b.c is now the merged M3 file; its rodata starts at text1a_c2's
D = "docs/grind/rodata-align-2026-09-30.md"
L = rd(D).split(NL)
k = [i for i, l in enumerate(L) if l.startswith("| text1b.c | unchanged head, then snd_Init .. ")]
assert len(k) == 1, k
cells = L[k[0]].split(" | ")
assert cells[-1] == "0x8001585C..0x800158B4 (func_80058580's tables) |", cells[-1]
cells[1] = "text1a_c2 + text1a_b + text1a_b_pre_rodata + sound merged in ahead of it (owner ruling Q67, Q65 step 09), then " + cells[1]
cells[-1] = ("0x800152B4..0x800158B4 (step 09: text1a_c2 0x800152B4, text1a_b 0x800153B4, text1a_b_pre_rodata "
             "0x800153F0, sound none, text1b 0x8001585C = func_80058580's tables) |")
L[k[0]] = " | ".join(cells)
wr(D, NL.join(L))
print("M3 merge applied")
