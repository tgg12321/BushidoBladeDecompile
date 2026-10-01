#!/usr/bin/env python3
"""Step 2 (Q65, per-file-gp-model.md "Cut position", outcome (i)): the code6cac_b_tu2 / code6cac_b_tu3
boundary moves from func_800344B4 to func_800343F0. Moves only.

Evidence: D_800A3140 is reached gp-relative by func_8002AB08 and by a direct lui/%lo store in func_800343F0,
both in code6cac_b_tu2 today; no single definition in one file produces both (the split test). The window
(tmp/q56/adopt/window.py record, PLAN.md) runs from after func_8002AB08 up to func_800343F0; every position
shares no gp symbol across the cut; the conventional position is immediately before func_800343F0. That
position lies inside the recorded window of the rodata-align boundary site 3 (after func_80033498 .. up to
func_800344B4), so that boundary MOVES there (outcome (i)); no new file. func_800343F0 owns no compiled rodata:
code6cac_b_tu3's rodata start (INCLUDE_RODATA jtbl_8001084C, 0x8001081C) is unchanged.
usage: s02_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
os.makedirs("/tmp/q56/s02", exist_ok=True)
BEFORE_TU3 = rd("src/code6cac_b_tu3.c")
run(f"{H}/splitc.py", "src/code6cac_b_tu2.c", str(defline("src/code6cac_b_tu2.c", "func_800343F0")), "/tmp/q56/s02/f800343F0.c")
run(f"{H}/mergec.py", "src/code6cac_b_tu3.c", "/tmp/q56/s02/f800343F0.c", "src/code6cac_b_tu3.c")
# mergec's part banner names the scratch file; name the source instead
sub1("src/code6cac_b_tu3.c", "/* ---- merged from f800343F0.c (owner ruling Q65: one original file) ---- */",
     "/* func_800343F0 moved here from code6cac_b_tu2.c: the file boundary follows the per-file gp evidence\n"
     " * (owner ruling Q65; docs/grind/rodata-align-2026-09-30.md section 7, site 3 record). */")
sub1("src/code6cac_b_tu3.c", "/* ---- merged from code6cac_b_tu3.c (owner ruling Q65: one original file) ---- */\n", "")
# the rodata-align record of the boundary moves with it
sub1("docs/grind/rodata-align-2026-09-30.md",
     "  | code6cac_b_tu3.c | `INCLUDE_RODATA jtbl_8001084C`, func_800344B4 | 0x8001081C | after func_80033498 .. up to func_800344B4 |",
     "  | code6cac_b_tu3.c | func_800343F0, then `INCLUDE_RODATA jtbl_8001084C`, func_800344B4 (moved from func_800344B4 by owner ruling Q65, per-file gp model, cut outcome (i): D_800A3140 is reached gp by func_8002AB08 and by lui/%lo in func_800343F0) | 0x8001081C | after func_80033498 .. up to func_800344B4 |")
new = added_decls(BEFORE_TU3, rd("src/code6cac_b_tu3.c"))
msg = ["Rule: per-file-gp-model.md \"Cut position\", outcome (i) (owner ruling Q65): the code6cac_b_tu2 / code6cac_b_tu3",
       "boundary moves to func_800343F0, inside the recorded window of rodata-align site 3; no new file. Evidence:",
       "s02_apply.py docstring (memory/grind/q65-adoption/q56/adopt/). func_800343F0 moves verbatim; the split carries",
       "the declarations it needs from code6cac_b_tu2.c's file-scope block (splitc.py), and code6cac_b_tu3.c gains,",
       "besides the function, these declaration lines it did not have:"] + ["- " + l for l in new] + [
       "The rodata-align section 7 site-3 row records the move. Byte-identical: full build SHA1 == oracle, every object compared."]
wr(f"{H}/s02_msg.txt", NL.join(msg) + NL)
print("step 2 applied")
