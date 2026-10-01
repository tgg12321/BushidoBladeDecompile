#!/usr/bin/env python3
"""Step 7 (Q65, per-file-gp-model.md "Merge"): code6cac_c2 + config are one file, code6cac_c2.c. Verbatim
merge in link order (mergec.py); the declarations were reconciled in step 5.

Evidence: the initialized small-data object D_800A322C is a (K3) object reached gp-relative from both parts
(and the bss object D_800A336C too); the two are contiguous in link order (text 0x8003B9D0..0x800401CC); no
rodata-align boundary lies between them; no contradiction inside the merged file (tmp/q56/mergecheck2.py).
usage: s07_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
run(f"{H}/mergec.py", "src/code6cac_c2.c", "src/code6cac_c2.c", "src/config.c")
os.remove("src/config.c")
ld_merge("code6cac_c2", ["code6cac_c2", "config"])
print("step 7 applied")
