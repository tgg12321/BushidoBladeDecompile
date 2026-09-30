import sys, os
# hdr2d.py <tree>: rewrite the D_8009BCF8 comment for the 2 x 10 page model (run after apply2d.py)
T = sys.argv[1]
h = os.path.join(T, 'include/game.h')
s = open(h, newline='').read()
a = s.index('/* 0x8009BCF8: 40-byte table of 20 two-byte records')
b = s.index('typedef struct {\n    u8 unk0;\n    u8 unk1;\n} Unk8009BCF8Record;')
new = '''/* 0x8009BCF8: 2 pages x 10 two-byte records (0x8009BCF8..0x8009BD1F; D_8009BD20
 * follows), one page per character-select page, 2 rows x 5 columns of cells.
 * Object model evidence from the original binary: func_80075F80 (0x800761B4-
 * 0x800761E4) and func_800759D0 (0x80075BC4-0x80075BF0) address it as
 * page * 20 + cell * 2 -- the cell (row * 5 + col) shifted left 1, the page times
 * 5 shifted left 2, added, then `lbu %lo(D_8009BCF8)($at)` -- two-level array
 * indexing of [page][cell]; func_800759D0's cell loop steps through the records
 * flat from the table base (`lui $s6,%hi(D_8009BCF8); addiu $s6,$s6,%lo(D_8009BCF8)`,
 * a 2-byte step), and func_80076D74 reads byte 1 (`lbu %lo(D_8009BCF9)($at)`)
 * through a flat shift-1 index. Data: the unk1 column is 0x00..0x09 (page 0) then
 * 0x0C..0x15 (page 1). Replaces the splat per-word scalars D_8009BCF8 / D_8009BCF9
 * (per-word splat symbol -> aggregate merge family, owner ruling 2026-08-17). */
'''
s = s[:a] + new + s[b:]
open(h, 'w', newline='\n').write(s)
print('hdr comment updated')
