#!/usr/bin/env python3
"""M3 merge (Q65/Q67, per-file-gp-model.md "Merge"): text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b
are one file, text1b.c. Verbatim merge in link order (mergec.py; verbatim-identical re-declarations dropped);
the declarations were reconciled in the previous step.

Evidence (per-file-gp-model.md, recorded as rodata-object-alignment section 7; tmp/q56/adopt/m34_evidence.md):
the PSYLINK probe (a file's statics are one contiguous .lcomm block in link order), the static-region gp
reach per group, mergecheck2 (no contradiction inside the merged file), and the jump-table phase check (all
members phase 4: no rodata-align boundary is removed).
usage: m3_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
parts = ["text1a_c2", "text1a_b", "text1a_b_pre_rodata", "sound", "text1b"]
run(f"{H}/mergec.py", "src/text1b.c", *[f"src/{p}.c" for p in parts])
for p in parts[:-1]:
    os.remove(f"src/{p}.c")
ld_merge("text1b", parts)
print("M3 merge applied")
