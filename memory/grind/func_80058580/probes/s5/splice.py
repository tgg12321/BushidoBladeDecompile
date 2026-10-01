#!/usr/bin/env python3
"""splice.py <candidate.c> <out.c> [--no-838] [--no-8c8]
Writes text1b.c with: the jtbl transcriptions removed, the candidate in place of the INCLUDE_ASM,
the D_8009A838 array retype (declaration + func_80056FE8's read), and the D_8009A8C8 entry-table
model (func_80055138's read). The matching include/code6cac.h edit is written to
tmp/func_80058580/inc/code6cac.h (put -Itmp/func_80058580/inc before -Iinclude)."""
import sys, os
src = open("src/text1b.c", encoding="utf-8").read()
cand = open(sys.argv[1], encoding="utf-8").read()
a = src.index("/* func_80058580's three switch tables")
b = src.index('INCLUDE_ASM("asm/funcs", func_80058580);')
e = b + len('INCLUDE_ASM("asm/funcs", func_80058580);')
out = src[:a] + cand + src[e:]

EDITS_838 = [
    ("extern s8 D_8009A838;\n", "extern s8 D_8009A838[];\n"),
    ("base += (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;",
     "base += D_8009A838[*((s16 *) (a2 + 0xE))] * 8;"),
]
EDITS_8C8 = [
    ("    u8 *src;\n    u8 *pair;\n    u8 (*row)[4];\n", "    CpuLevelEntry *src;\n    u8 *pair;\n"),
    ("            row = D_8009A8C4[*(s16 *)(p + 0x86)];\n            src = row[D_800A37A0];\n            p[0x424] = src[0];\n            p[0x3F6] = src[1];\n",
     "            src = &D_8009A8C8[*(s16 *)(p + 0x86)][D_800A37A0 - 1];\n            p[0x424] = src->unk0;\n            p[0x3F6] = src->unk1;\n"),
]
HDR_OLD = """/* Rows of eight 4-byte entries: func_80055138 reads [row][col][0..1] (row*0x20 + col*4);
 * func_80058580 reads the halfword at byte 2 of [row][col] (D_8009A8CA + row<<5 +
 * (col-1)*4 = D_8009A8C4 + row*0x20 + col*4 + 2, 0x8005A854-78; D_8009A8CA is the alias
 * row D_8009A8C4+6). */
extern u8 D_8009A8C4[][8][4];
"""
HDR_NEW = """/* Rows of eight 4-byte entries starting at 0x8009A8C8 (each row ends with a zero entry; the
 * word at 0x8009A8C4 is the 0x80 terminator of the script D_8009A8C0). Both readers index the
 * column 1-based, [row][D_800A37A0 - 1]: func_80055138 reads unk0/unk1 (GCC folds the -1
 * into the address, %lo(0x8009A8C4)), func_80058580 the mask halfword (%lo(0x8009A8CA)). */
typedef struct CpuLevelEntry {
    u8 unk0;
    u8 unk1;
    u16 mask;
} CpuLevelEntry;
extern CpuLevelEntry D_8009A8C8[][8];
"""
if "--no-838" not in sys.argv:
    for o, n in EDITS_838:
        assert out.count(o) == 1, o
        out = out.replace(o, n)
os.makedirs("tmp/func_80058580/inc", exist_ok=True)
hdr = open("include/code6cac.h", encoding="utf-8").read()
if "--no-8c8" not in sys.argv:
    for o, n in EDITS_8C8:
        assert out.count(o) == 1, o
        out = out.replace(o, n)
    assert hdr.count(HDR_OLD) == 1
    hdr = hdr.replace(HDR_OLD, HDR_NEW)
open("tmp/func_80058580/inc/code6cac.h", "w", encoding="utf-8", newline="\n").write(hdr)
open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(out)
