#!/usr/bin/env python3
"""Step 14 (Q65): two objects that become (K2) statics in step 10 are declared two ways inside their own
file; a static has ONE declaration, so each gets its one truthful type first (own byte-identical commit).

  text1b_tu1c, D_800A3468: the file declares it `s32` (file scope and eight functions: an integer holding the
    object's address, `*(s32 *)(D_800A3468 + 0xC)`) and, in func_80060A68 alone, as a block-scope
    `extern struct Ob *` (member access). It keeps the file's `s32`; func_80060A68 reaches the object through
    a cast view macro, the spelling text1b.c already uses for its s32 work pointer
    (`#define W ((Work_80053E9C *)D_800A33F4)`). Body change: func_80060A68 needs its own layer-2.
  text1a_post, g_anim_hit_flags: an array everywhere (func_800420E8 indexes it with a0 < 2, func_8004211C
    reads [0]) except func_800420D0, which declared a block-scope scalar to clear element 0. One declaration:
    the array; func_800420D0 writes element 0. Body change: func_800420D0 needs its own layer-2.
usage: s14_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
p = "src/text1b_tu1c.c"
t = rd(p)
a = "    extern struct Ob *D_800A3468;\n"
assert t.count(a) == 1
i = t.index(a)
fstart = t.rindex("\nvoid func_80060A68(void) {", 0, i)
fend = t.index("\n}\n", i) + 3
body = t[fstart:fend].replace(a, "")
n = body.count("D_800A3468->")
body = body.replace("D_800A3468->", "OB->")
body = body.replace("\nvoid func_80060A68(void) {",
                    "\nextern s32 D_800A3468;\n#define OB ((struct Ob *)D_800A3468)\nvoid func_80060A68(void) {", 1)
body = body.rstrip(NL) + "\n#undef OB\n"
wr(p, t[:fstart] + body + t[fend:])
print(f"text1b_tu1c func_80060A68: {n} member accesses through OB")
p = "src/text1a_post.c"
sub1(p, "void func_800420D0(void) {\n    extern s16 g_anim_hit_flags;\n    g_anim_counter = 0;\n    g_anim_hit_flags = 0;\n",
     "extern s16 g_anim_hit_flags[];\nvoid func_800420D0(void) {\n    g_anim_counter = 0;\n    g_anim_hit_flags[0] = 0;\n")
# the later file-scope array declaration (before func_800420E8) now repeats the one above: drop it
t = rd(p)
k = t.index("void func_800420D0(void) {")
rest = t[k:]
assert rest.count("extern s16 g_anim_hit_flags[];\n") == 1
wr(p, t[:k] + rest.replace("extern s16 g_anim_hit_flags[];\n", "", 1))
print("step 14 applied")
