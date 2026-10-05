#!/usr/bin/env python3
# (F) FAKE ablation over F02 batch 6 (v/17AFC.b6.c), each removed alone:
#   c22c: func_8002C22C's rec1 alias (decl + comment removed; rec1-> -> D_80101EC8[1].)
#   c61c: func_8002C61C's s1 / s0 aliases (decls + comment removed; s1 / s0 -> D_80101EC8[0] / [1])
# usage: abl_b6.py c22c|c61c
import re, sys
sys.path.insert(0, "tmp/p2/lt/f02")
which = sys.argv[1]; sys.argv = sys.argv[:1]
import f02b6 as T
import f02b2 as B
s = open("tmp/p2/lt/f02/v/17AFC.b6.c", encoding="utf-8").read()
g = open("include/game.h", encoding="utf-8").read()
def cut(b, start, decl_end):
    a = b.index(start); e = b.index(decl_end) + len(decl_end)
    return b[:a] + b[e:]
if which == "c22c":
    fn = "func_8002C22C"; i, j = B.span(s, fn); b = s[i:j]
    b = cut(b, "    /* FAKE: pointer alias to D_80101EC8[1]", "    Unk80101EC8Record *rec1 = &D_80101EC8[1];\n")
    b = re.sub(r"\brec1->", "D_80101EC8[1].", b)
    assert "rec1" not in b
else:
    fn = "func_8002C61C"; i, j = B.span(s, fn); b = s[i:j]
    b = cut(b, "    /* FAKE: pointer aliases to D_80101EC8[0] / [1]", "    Unk80101EC8Record *s0 = &D_80101EC8[1];\n")
    b = re.sub(r"\bs1->", "D_80101EC8[0].", b); b = re.sub(r"\bs0->", "D_80101EC8[1].", b)
    b = re.sub(r"\bs1\b", "&D_80101EC8[0]", b); b = re.sub(r"\bs0\b", "&D_80101EC8[1]", b)
T.measure(s[:i] + b + s[j:], g, [fn])
