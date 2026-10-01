#!/usr/bin/env python3
"""Step 4 (Q65, per-file-gp-model.md "Split", cut outcome (ii)): text1a_c.c is split before func_80044800
into the new file text1a_c_tu2.c. Moves only (splitc.py: verbatim tail + the declarations it needs).

Evidence: D_800A3820 is reached gp-relative by func_80044504 and by direct lui/%lo accesses in func_80044800,
both in text1a_c today; no single definition in one file produces both. Window: cut before func_80044650,
func_80044670, func_8004473C or func_80044800; none shares a gp symbol across the cut and the later part owns
no compiled rodata at any of them (rodata condition 2 met trivially; text1a_c.o's 0x18 rodata bytes stay with
the head). No rodata-align boundary's window contains the position (text1a_c / text1a_c2 is a legacy split), so
a new part begins at the conventional position, immediately before func_80044800. The new part inherits
text1a_c's per-file memberships (none). Record: PLAN.md.
usage: s04_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
run(f"{H}/splitc.py", "src/text1a_c.c", str(defline("src/text1a_c.c", "func_80044800")), "src/text1a_c_tu2.c")
ld_follow("text1a_c_tu2", "text1a_c")
new = added_decls("", rd("src/text1a_c_tu2.c"))
msg = ["Rule: per-file-gp-model.md \"Split\", cut outcome (ii) (owner ruling Q65): text1a_c.c splits before func_80044800",
       "into the new file text1a_c_tu2.c (D_800A3820 is reached gp by func_80044504 and by direct lui/%lo in",
       "func_80044800: no single definition in one file produces both). Evidence and the window: s04_apply.py",
       "docstring (memory/grind/q65-adoption/q56/adopt/). The functions from func_80044800 on move verbatim; the new",
       f"file opens with text1a_c.c's include block and carries the {len(new)} declaration lines its functions use",
       "(splitc.py: the parent's earlier declarations, verbatim and in order, plus externs for the parent's",
       "definitions it calls):"] + ["- " + l for l in new] + [
       "bb2.ld places text1a_c_tu2.o after text1a_c.o in every section list. Byte-identical: full build SHA1 ==",
       "oracle, every object compared."]
wr(f"{H}/s04_msg.txt", NL.join(msg) + NL)
print("step 4 applied")
