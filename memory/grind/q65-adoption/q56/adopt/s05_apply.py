#!/usr/bin/env python3
"""Step 5 (Q65, per-file-gp-model.md "Merge"): code6cac_b2_pre + replay_camera_rob_back_loose2 +
code6cac_b2_post are one file, code6cac_b2_post.c. Verbatim merge in link order (mergec.py: union of the
include blocks, identical re-declarations dropped, nothing else); the parts declare nothing differently.

Evidence: the initialized small-data objects D_800A31D8 (gp in code6cac_b2_pre, code6cac_b2_post) and
D_800A31DA (gp in all three) are (K3) objects: one defining file, and only that file gets gp for them. The
three objects are contiguous in link order (text 0x80035438..0x80035F30). No rodata-align boundary lies between
them (legacy splits). No contradiction inside the merged file (tmp/q56/mergecheck2.py).
usage: s05_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
M = ["code6cac_b2_pre", "replay_camera_rob_back_loose2", "code6cac_b2_post"]
run(f"{H}/mergec.py", "src/code6cac_b2_post.c", *[f"src/{m}.c" for m in M])
for m in M[:-1]:
    os.remove(f"src/{m}.c")
ld_merge("code6cac_b2_post", M)
print("step 5 applied")
