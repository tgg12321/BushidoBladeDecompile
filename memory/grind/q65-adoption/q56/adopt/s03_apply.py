#!/usr/bin/env python3
"""Step 3 (Q65, per-file-gp-model.md "Merge" bullet, boundary move in place of a merge): the text1b /
text1b_tu1c boundary moves from func_80061064 to func_80060A68 (func_80060A68, func_80060B70 and
func_80060E38 move from the end of text1b.c to the start of text1b_tu1c.c). Moves only.

Evidence: the initialized small-data object D_800A32BC (K3) and 40 statics of text1b_tu1c's per-file block
(0x800A3444..0x800A34EC, amendment A1) are reached gp-relative from both sides of the current boundary: by
func_80060A68 / func_80060B70 / func_80060E38 (text1b.c since rodata-align section 9) and by functions of
text1b_tu1c.c. The boundary was set under rodata-object-alignment (section 9) with the recorded window "after
func_8005C2A8 up to func_80061064" (the functions in between own no rodata). Inside it, the cut positions that
put every gp user of each per-file object on one side are before func_8005C614 .. func_8005FBC8, before
func_800600C8 .. func_80060758, and before func_80060A68 (tmp/q56/adopt/cutsearch.py); the one closest to the
current position is before func_80060A68, the only one there. text1b_tu1c's rodata start (0x800158E0,
func_80061064's string) is unchanged: the three moved functions own no rodata. The section 9 record is
updated in this commit.
usage: s03_apply.py <tree>"""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
os.makedirs("/tmp/q56/s03", exist_ok=True)
L = rd("src/text1b.c").split(NL)
funcs = [re.match(r"^[\w\s\*]*\b(func_\w+|snd_\w+)\s*\(", l).group(1) for l in L
         if re.match(r"^[A-Za-z_][\w\s\*]*\b(func_\w+|snd_\w+)\s*\([^;]*$", l)]
assert funcs[-1] == "func_80060E38", funcs[-4:]
prev = funcs[funcs.index("func_80060A68") - 1]
run(f"{H}/splitc.py", "src/text1b.c", str(defline("src/text1b.c", "func_80060A68")), "/tmp/q56/s03/tail.c")
import subprocess, io, contextlib
_r = subprocess.run([sys.executable, f"{H}/mergec.py", "src/text1b_tu1c.c", "/tmp/q56/s03/tail.c", "src/text1b_tu1c.c"],
                    capture_output=True, text=True)
print(_r.stdout.strip()); assert _r.returncode == 0, _r.stderr
DUPS = [l.strip() for l in _r.stdout.splitlines() if "dropped verbatim repeat (" in l]
sub1("src/text1b_tu1c.c", "/* ---- merged from tail.c (owner ruling Q65: one original file) ---- */",
     "/* func_80060A68 .. func_80060E38 moved here from text1b.c: the file boundary follows the per-file gp evidence\n"
     " * (owner ruling Q65; docs/grind/rodata-align-2026-09-30.md section 9 record). */")
sub1("src/text1b_tu1c.c", "/* ---- merged from text1b_tu1c.c (owner ruling Q65: one original file) ---- */\n", "")
_buf = io.StringIO()
with contextlib.redirect_stdout(_buf):
    drop_header_duplicates("src/text1b_tu1c.c")
print(_buf.getvalue().strip())
HDR = [l.split(": dropped verbatim repeat of a header definition: ")[1] for l in _buf.getvalue().splitlines()
       if ": dropped verbatim repeat of a header definition: " in l]
INC_ADDED = [l for l in rd("src/text1b_tu1c.c").split(NL)[:12] if l.startswith("#include")]
D = "docs/grind/rodata-align-2026-09-30.md"
sub1(D, "| text1b.c | unchanged head, then snd_Init .. func_80060E38 |",
     f"| text1b.c | unchanged head, then snd_Init .. {prev} |")
sub1(D, "| text1b_tu1c.c | func_80061064 .. the function before func_8006E534 |",
     "| text1b_tu1c.c | func_80060A68 .. the function before func_8006E534 (func_80060A68 .. func_80060E38 moved here by owner ruling Q65: per-file gp model, boundary move in the recorded window) |")
msg = [
    "Rule: per-file-gp-model.md, Merge bullet \"Boundary move instead of a merge\" (owner ruling Q65; original text",
    "64c69153a, dropped by the slim commit ffec95a08, restored in rules: commit e5317cbf9). The text1b / text1b_tu1c",
    f"boundary moves from func_80061064 to func_80060A68: func_80060A68 .. func_80060E38 move verbatim from the end of",
    f"text1b.c (which now ends with {prev}) to the start of text1b_tu1c.c; the section 9 record of",
    "docs/grind/rodata-align-2026-09-30.md is updated. Evidence: s03_apply.py docstring (cutsearch.py).",
    "",
    "Besides the move, text1b_tu1c.c changes only in its declarations (mergec.py / drop_header_duplicates):",
    "- the include block is the union of both parts' includes: " + ", ".join(INC_ADDED) + ";",
    "- typedef -> gte.h: text1b_tu1c.c's own one-line typedefs, verbatim repeats of include/gte.h's definitions now",
    "  that the moved part brings `#include \"gte.h\"`, are dropped (a repeat is a redefinition error):",
] + ["    " + h for h in HDR] + [
    f"- duplicate-declaration dedup: {len(DUPS)} file-scope declarations repeated verbatim (normalized text identical to",
    "  one already emitted earlier in the merged file) are dropped; each named object keeps its first declaration:",
] + ["    " + d.split("): ", 1)[1] + "  (" + d.split("(", 1)[1].split(")")[0] + ")" for d in DUPS]
wr(f"{H}/s03_msg.txt", NL.join(msg) + NL)
print(f"step 3 applied (text1b.c now ends with {prev})")
