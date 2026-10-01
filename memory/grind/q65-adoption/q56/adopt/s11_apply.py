#!/usr/bin/env python3
"""Step 11: M4 merge (Q65/Q67, per-file-gp-model.md "Merge"): text1b_tu2 + text1b_b are one file, text1b_b.c
(Q67's group exactly). text1a_b_mid_rodata.c, between them in .rodata only, is empty (no bytes in any
section, no .bss line) and stays as it is. Verbatim merge in link order (mergec.py; verbatim-identical re-declarations dropped); the
declarations were reconciled in the previous step.

Evidence (recorded as rodata-object-alignment section 7; tmp/q56/adopt/m34_evidence.md): the PSYLINK probe,
the static-region gp reach per group, mergecheck2, and the jump-table phase check (text1b_tu2 and text1b_b
both phase 4: no rodata-align boundary is removed).
usage: s11_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
parts = ["text1b_tu2", "text1b_b"]
merge_msg("11", "Rule: per-file-gp-model.md Merge (owner ruling Q67 group): text1b_tu2 + text1b_b are one original file; declarations reconciled in step 10 (evidence: s11_apply.py docstring, m34_evidence.md).", "src/text1b_b.c", *[f"src/{p}.c" for p in parts])
for p in parts[:-1]:
    os.remove(f"src/{p}.c")
ld_merge("text1b_b", parts)
t = rd("src/text1b_b.c")
assert "(owner ruling Q65: one original file)" in t
wr("src/text1b_b.c", t.replace("(owner ruling Q65: one original file)", "(owner ruling Q67: one original file)"))
# rodata-align record, section 7 table: text1b_tu2.c is merged into text1b_b.c
sub1("docs/grind/rodata-align-2026-09-30.md",
     "  | text1b_tu2.c | func_800747D8 | 0x80015A0C | after func_8006ECF4 .. up to func_800747D8 |",
     "  | text1b_tu2.c (merged into text1b_b.c, owner ruling Q67, Q65 step 11; text1b_b.c now starts here) | func_800747D8 | 0x80015A0C | after func_8006ECF4 .. up to func_800747D8 |")
print("M4 merge applied")
