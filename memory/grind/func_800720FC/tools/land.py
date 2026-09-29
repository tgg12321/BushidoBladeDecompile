"""Apply the func_800720FC landing to the tree (src/text1b.c, include/game.h,
undefined_syms_auto.txt). Body = memory/grind/func_800720FC/landing_body.c minus
its two leading table externs (they move to include/game.h, merge prong (d))."""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm

body = Path('memory/grind/func_800720FC/landing_body.c').read_text(encoding='utf-8')
HEAD = """/* Page origins for the three scroll pages 4..6 (the page's (x, y) pair; mode m
 * shows page m + 4). */
extern s16 D_8009BCC4[][2];
extern s16 D_8009BCD0[2];
"""
assert body.startswith(HEAD)
body = body[len(HEAD):]

src = Path('src/text1b.c').read_text(encoding='utf-8')
src = inlineasm.substitute_body(src, 'func_800720FC', body)
old1 = 'extern u16 D_800A3578;'
assert src.count(old1) == 2
src = src.replace(old1, 'extern s16 D_800A3578; /* s16: func_80070188, func_80070F78 and func_800720FC read it '
                  'with lh; func_8006EC0C reads its halves through u8/u16 values. */', 1)
src = src.replace(old1, 'extern s16 D_800A3578;', 1)
Path('src/text1b.c').write_bytes(src.encode('utf-8'))

gh = Path('include/game.h').read_text(encoding='utf-8')
anchor = '/* 0x8009BCF8: 40-byte table of 20 two-byte records'
assert gh.count(anchor) == 1
add = """/* 0x8009BCC4: the three scroll pages' origins, s16 (x, y) pairs for pages 4..6
 * (0x8009BCC4..0x8009BCCF). Object model evidence from the original binary:
 * func_800720FC reads (x, y) of entry `mode` through ONE index register
 * (`sll $a2,mode,2`, then `lhu %lo(D_8009BCC4)($at)` and `lhu %lo(D_8009BCC6)($at)`
 * with the same $a2), and its scroll loop reads page p's pair at
 * 0x8009BCB4 + p * 4 for p = 4..6 (asm lines 220-233), i.e. entries 0..2 of
 * this table. Replaces the splat
 * per-word symbols D_8009BCC4 / D_8009BCC6 (per-word splat symbol -> aggregate
 * merge family, owner ruling 2026-08-17). */
extern s16 D_8009BCC4[3][2];

/* 0x8009BCD0: the current scroll offset, one s16 per axis. func_800720FC walks
 * it with a 2-byte step bounded by &D_8009BCD0 + 4 (asm lines 220-288). Replaces
 * D_8009BCD0 / D_8009BCD2 (same merge family). */
extern s16 D_8009BCD0[2];

"""
gh = gh.replace(anchor, add + anchor)
Path('include/game.h').write_bytes(gh.encode('utf-8'))

us = Path('undefined_syms_auto.txt').read_text(encoding='utf-8')
for row in ('D_8009BCC6 = 0x8009BCC6;\n', 'D_8009BCD2 = 0x8009BCD2;\n'):
    assert us.count(row) == 1, row
    us = us.replace(row, '')
Path('undefined_syms_auto.txt').write_bytes(us.encode('utf-8'))
print('landed')
