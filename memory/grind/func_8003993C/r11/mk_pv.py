"""Derive the one-variable-per-value twin(s) of the Ruling 11 reuse body.

usage: python3 mk_pv.py <reuse.c> <out.c> [temp|entry|both]
  temp  : split only `temp`  (selector -> `sel` in the else arm, window -> `window` at function scope)
  entry : split only `entry` (`entry_a` in the if arm, `entry_b` in the else arm)
  both  : split both (the full one-variable-per-value spelling)
Only declarations and identifiers change; the statement list is untouched.
"""
import re
import sys

src_path, out_path = sys.argv[1], sys.argv[2]
mode = sys.argv[3] if len(sys.argv) > 3 else "both"
src = open(src_path).read()


def sub1(s, old, new):
    assert s.count(old) == 1, (old, s.count(old))
    return s.replace(old, new)


def drop_decl(s, name):
    # remove the Ruling 11 comment + declaration of `name`
    pat = re.compile(r"\n[ \t]*/\* Ruling 11 [^*]*(?:\*(?!/)[^*]*)*\*/\n[ \t]*s32 " + name + r";\n")
    assert len(pat.findall(s)) == 1, name
    return pat.sub("\n", s)


if mode in ("temp", "both"):
    s = drop_decl(src, "temp")
    s = sub1(s, "    u8 save40;\n", "    s32 window;\n    u8 save40;\n")
    s = sub1(s, "            temp = (*(u8 *)(p + 0x17) >> 1) & 1;\n",
             "            s32 sel;\n\n            sel = (*(u8 *)(p + 0x17) >> 1) & 1;\n")
    s = s.replace("D_801027B0[temp]", "D_801027B0[sel]")
    s = sub1(s, "temp = 0x77", "window = 0x77")
    s = sub1(s, "temp = D_800A36F8", "window = D_800A36F8")
    s = sub1(s, ">= temp", ">= window")
    s = sub1(s, "<= temp", "<= window")
    assert not re.search(r"\btemp\b", s)
    src = s

if mode in ("entry", "both"):
    s = drop_decl(src, "entry")
    # the loop body keeps its opening line; drop the blank line left after the removed declaration
    s = sub1(s, "    for (i = 0; i < 2; i++) {\n\n", "    for (i = 0; i < 2; i++) {\n")
    s = sub1(s, "        if (*(u8 *)(p + 0x17) & 1) {\n            entry = ",
             "        if (*(u8 *)(p + 0x17) & 1) {\n            s32 entry_a;\n\n            entry_a = ")
    s = sub1(s, "D_80102768 + *(u16 *)(entry + 2)", "D_80102768 + *(u16 *)(entry_a + 2)")
    # else arm: declaration goes first in the block (after `sel` if split)
    if mode == "both":
        s = sub1(s, "            s32 sel;\n\n", "            s32 sel;\n            s32 entry_b;\n\n")
    else:
        s = sub1(s, "        } else {\n            temp = (*(u8",
                 "        } else {\n            s32 entry_b;\n\n            temp = (*(u8")
    s = sub1(s, "            entry = D_801027B0", "            entry_b = D_801027B0")
    s = sub1(s, "*(u16 *)(entry + 2);", "*(u16 *)(entry_b + 2);")
    assert not re.search(r"\bentry\b", s)
    src = s

open(out_path, "w", newline="\n").write(src)
