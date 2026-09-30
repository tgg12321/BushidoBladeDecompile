"""Single-construct ablations and Ruling 11 per-value splits generated from final.c (only the named
construct changes; everything else byte-for-byte the landed text)."""
import re
from pathlib import Path
SRC = 'final.c'
t = Path(SRC).read_text()
def rep(u, a, b, n=1):
    c = u.count(a); assert c == n, (c, a[:70]); return u.replace(a, b)
def seg(u, start, end):
    a = u.index(start); b = u.index(end, a); return a, b
V = {}
# loop-1 rec
a, b = seg(t, '        if (D_800A3578 == 0) {\n            /* FAKE: named intermediate', '    for (i = 0; i < 1 + D_800A35B0 + (port_ofs')
blk = t[a:b]
blk2 = re.sub(r'            /\* FAKE: named intermediate.*?\*/\n            s32 rec = i \* 3;\n\n', '', blk, flags=re.S)
blk2 = blk2.replace('D_800A3560[rec', 'D_800A3560[i * 3')
V['rec1'] = t[:a] + blk2 + t[b:]
# loop-2 rec
u = rep(t, '        s32 rec; /* FAKE: record offset, mechanism at the loop-1 `rec` */\n', '')
u = rep(u, '        rec = i * 3;\n', '')
V['rec2'] = rep(u, 'if (D_800A3560[rec] != 5 && D_800A3560[rec] != 0x10) {', 'if (D_800A3560[i * 3] != 5 && D_800A3560[i * 3] != 0x10) {')
u = rep(t, '            s32 ofs; /* FAKE: record offset, mechanism at the loop-1 `rec` */\n', '')
u = rep(u, '            ofs = i * 3;\n', '')
V['ofs'] = rep(u, 'D_800A3560[ofs + 2]', 'D_800A3560[i * 3 + 2]', 2)
# idx at ==3 (first) and locked (second)
IDXD = '                    s32 idx; /* FAKE: record offset, mechanism at the loop-1 `rec` */\n'
def drop_idx(u, which):
    parts = u.split(IDXD); assert len(parts) == 3
    k = which + 1
    seg_ = parts[k]
    seg_end = seg_.index('DrawSync(0);')
    s = seg_[:seg_end]
    s = rep(s, '                    idx = i * 3;\n', '')
    s = s.replace('idx + ', 'i * 3 + ')
    parts[k] = s + seg_[seg_end:]
    return parts[0] + (IDXD if which == 1 else '') + parts[1] + (IDXD if which == 0 else '') + parts[2]
V['idx3'] = drop_idx(t, 0)
V['idxL'] = drop_idx(t, 1)
V['other'] = rep(rep(t, '''                s32 other = i == 0 ? 3 : 0; /* FAKE: the other player's record offset,
                                             * mechanism at the loop-1 `rec` */

''', ''), 'D_800A3560[other + 2]', 'D_800A3560[(i == 0 ? 3 : 0) + 2]')
TL = '''                    tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);
                    LoadImage(vram + id * 8, *tim);'''
INL = '''                    LoadImage(vram + id * 8, *(s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4));'''
p = t.split(TL); assert len(p) == 4
V['tim3'] = p[0] + INL + p[1] + TL + p[2] + TL + p[3]
V['timC'] = p[0] + TL + p[1] + INL + p[2] + TL + p[3]
V['timL'] = p[0] + TL + p[1] + TL + p[2] + INL + p[3]
# Ruling 11 per-value splits (each per-value local at the innermost scope enclosing its writes)
def vram_split(u):
    u = rep(u, '''                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    flag = 2;
                    vram = *(u8 **)(D_800A35A8 + 0x7C);
                    vram += i << 6;''', '''                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */
                    u8 *vram2;

                    func_8005C650(1, 0x7F, 0x7F);
                    flag = 2;
                    vram2 = *(u8 **)(D_800A35A8 + 0x7C);
                    vram2 += i << 6;''')
    parts = u.split('LoadImage(vram + id * 8, *tim);'); assert len(parts) == 4
    return parts[0] + 'LoadImage(vram + id * 8, *tim);' + parts[1] + 'LoadImage(vram + id * 8, *tim);' + parts[2] + 'LoadImage(vram2 + id * 8, *tim);' + parts[3]
def sheets_split(u):
    u = rep(u, '    s32 flag;\n', '    s32 *sheets2;\n    s32 flag;\n')
    return rep(u, '    sheets = *(s32 **)(D_800A35A8 + 0x60);\n    s->header = sheets[0];\n', '    sheets2 = *(s32 **)(D_800A35A8 + 0x60);\n    s->header = sheets2[0];\n')
def cells_split(u, which):
    if 'a' in which:
        u = rep(u, '''                s->has_color = 0;
                cells = s->header + 0xC;
                s->table = cells;''', '''                s->has_color = 0;
                cells_a = s->header + 0xC;
                s->table = cells_a;''')
        u = rep(u, '''            } else {
                if ((D_800A354C & (0x10 << (port * 16))) && D_800A3578 == 0 && D_800A35BC != 3) {''', '''            } else {
                s32 cells_a;

                if ((D_800A354C & (0x10 << (port * 16))) && D_800A3578 == 0 && D_800A35BC != 3) {''')
    if 'b' in which:
        u = rep(u, '''                cells = s->header + 0xC;
                s->table = cells;
                c = ''', '''                cells_b = s->header + 0xC;
                s->table = cells_b;
                c = ''')
        u = rep(u, '''            if (D_800A3578 != 3) {
                s->scale_x = 0x100;''', '''            if (D_800A3578 != 3) {
                s32 cells_b;

                s->scale_x = 0x100;''')
    if 'c' in which:
        u = rep(u, '    cells = s->header + 0x24;\n', '    cells_c = s->header + 0x24;\n')
        u = rep(u, '            s->table = cells;\n            s->x = (D_800A3590[i] << 6) + 0x80;\n', '            s->table = cells_c;\n            s->x = (D_800A3590[i] << 6) + 0x80;\n')
        u = rep(u, '    s32 flag;\n', '    s32 cells_c;\n    s32 flag;\n')
    return u
V['r11_vram'] = vram_split(t)
V['r11_sheets'] = sheets_split(t)
V['r11_cells_a'] = cells_split(t, 'a')
V['r11_cells_b'] = cells_split(t, 'b')
V['r11_cells_c'] = cells_split(t, 'c')
V['r11_cells_all'] = cells_split(t, 'abc')
V['r11_all'] = cells_split(sheets_split(vram_split(t)), 'abc')
Path('ablf').mkdir(exist_ok=True)
names = []
for k, u in V.items():
    Path(f'ablf/{k}.c').write_text(u, newline='\n'); names.append(f'tmp/func_80070F78/ablf/{k}.c')
Path('ablf.list').write_text(' '.join(names))
print(len(names))
