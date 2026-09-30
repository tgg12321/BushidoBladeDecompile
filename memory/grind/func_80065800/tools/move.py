"""Move the text1b | text1b_tu1c cut from snd_Init to func_80061064 (run from the tree root,
AFTER land.py).  Moves only:
- snd_Init's extern block .. func_80060E38 (text1b_tu1c.c) are appended to text1b.c, the file
  they came from (306ae8548); text1b_tu1c.c's header is rebuilt by the rodata-align split tool
  (splitc.py: its include block, the earlier declarations the rest uses, verbatim and in order,
  then externs derived from definitions);
- D_800158E0 (func_80061064's string, 0x800158E0) moves from text1a_b_pre_rodata_b.c to
  text1b_tu1c.c, replacing func_80061064's `extern s32 D_800158E0;` (they would conflict).
text1b_tu1c.o's .rodata then starts at 0x800158E0 (phase 0, like its first table)."""
import subprocess
import sys
from pathlib import Path

NL = chr(10)
SPLIT = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/memory/grind/func_80058580/aspsx-align-check/poc/splitc.py'
# Declarations of the moved block that text1b.c already carries (include/gte.h:27-28, which
# text1b.c includes and text1b_tu1c.c does not): a second definition would not compile.
DROP = ['typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;',
        'typedef struct DVECTOR { s16 vx, vy; } DVECTOR;']


def rd(p):
    t = Path(p).read_bytes().decode('utf-8')
    assert '\r' not in t, p
    return t


def wr(p, t):
    Path(p).write_bytes(t.encode('utf-8'))


def one(t, s):
    assert t.count(s) == 1, s[:80]
    return t.index(s)


C = 'src/text1b_tu1c.c'
t = rd(C)
hdr_end = one(t, '\nextern s16 D_800A3400;\n') + 1
cut_at = one(t, 'extern s32 func_80041E10();\nextern s32 func_800421A4();\n')
cut_line = t[:cut_at].count(NL) + 1
subprocess.run([sys.executable, SPLIT, C, str(cut_line), 'src/_tu1c_tail.c'], check=True)
head = rd(C)
block = head[hdr_end:]
assert block.startswith('extern s16 D_800A3400;') and 'void func_80060E38(s32 arg0, s32 arg1) {' in block
for d in DROP:
    block = block.replace(d + NL, '', 1)
tail = rd('src/_tu1c_tail.c').replace(
    '/* Declarations from the file this TU was split from (text1b_tu1c.c). */',
    '/* Declarations from the file this TU was split from (text1b.c). */')
Path('src/_tu1c_tail.c').unlink()

R = 'src/text1a_b_pre_rodata_b.c'
r = rd(R)
defn = ('/* D_800158E0: 24B @ 0x800158E0 — "eff prim over :%d \\n" + alignment + empty trailing string */\n'
        'const char D_800158E0[24] = "eff prim over :%d \\n";\n')
i = one(r, defn)
r = r[:i] + r[i + len(defn):]
tail = tail.replace('\nextern s32 D_800158E0;\n', '\n' + defn, 1)
assert tail.count(defn) == 1
# func_80065800's two transcribed switch tables follow D_800158E0 in the same object, so they move
# with it, verbatim, to just before their owner's INCLUDE_ASM line (as section 8 did for
# func_80058580's). Skipped when land.py already replaced them with compiled tables.
JT = '/* jtbl_800158F8: 18 words (72B) @ 0x800158F8 */'
if JT in r:
    ja = one(r, JT)
    jb = one(r, '/* NOTE: the cluster continues in src/text1a_b_mid_rodata.c.')
    tables = r[ja:jb]
    r = r[:ja] + r[jb:]
    inc = 'INCLUDE_ASM("asm/funcs", func_80065800);\n'
    k = one(tail, inc)
    tail = tail[:k] + tables + tail[k:]
# Comments only: what this file still holds, and who supplies the bytes after it.
r = r.replace(""" * 0x800158B4..0x80015987: the sound-bank loader's strings (snd_LoadCommonVab,
 * func_8005C2A8), func_80061064's string and func_80065800's two tables. */""",
              """ * 0x800158B4..0x800158DF: the sound-bank loader's strings (snd_LoadCommonVab,
 * func_8005C2A8), rodata of text1b.c's TU (docs/grind/rodata-align-2026-09-30.md
 * section 9). */""", 1)
r = r.replace("""/* NOTE: the cluster continues in src/text1a_b_mid_rodata.c. The bytes from
 * 0x80015988 through 0x80015A0B are supplied by build/src/text1b_tu1c.o
 * (func_8006B578's compiler-generated switch table and the warning string,
 * up to 0x800159AF)""", """/* NOTE: the cluster continues in src/text1a_b_mid_rodata.c. The bytes from
 * 0x800158E0 through 0x80015A0B are supplied by build/src/text1b_tu1c.o
 * (func_80061064's string, func_80065800's two tables, func_8006B578's
 * compiler-generated switch table and the warning string, up to 0x800159AF)""", 1)
assert '0x800158B4..0x800158DF' in r and '0x800158E0 through 0x80015A0B' in r
wr(C, tail)
wr(R, r)

T = 'src/text1b.c'
x = rd(T)
assert x.endswith('INCLUDE_ASM("asm/funcs", func_80058580);\n'), repr(x[-80:])
wr(T, x + block)
print('moved %d lines to text1b.c' % block.count(NL))
