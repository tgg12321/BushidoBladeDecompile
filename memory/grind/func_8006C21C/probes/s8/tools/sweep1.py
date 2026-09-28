"""Single-site spelling mutations of base.c -> tmp/c21c/sw1/<n>.c ; prints list file."""
import os
src = open('tmp/c21c/base.c').read()
os.makedirs('tmp/c21c/sw1', exist_ok=True)
R = 'rcos((D_800A3518 << 7) & 0xF80)'
muts = {
    # rcos angle
    'ang1': (R, 'rcos((D_800A3518 & 0x1F) << 7)'),
    'ang2': (R, 'rcos((D_800A3518 * 128) & 0xF80)'),
    'ang3': (R, 'rcos((D_800A3518 & 31) * 128)'),
    'ang4': (R, 'rcos(D_800A3518 * 128 & 0xFFF)'),
    'ang5': (R, 'rcos((D_800A3518 << 7) & 0xFFF)'),
    'ang6': (R, 'rcos((D_800A3518 % 32) << 7)'),
    # pulse
    'pul1': ('((' + R + ' * 32) >> 12) + 0xD0', '((' + R + ' << 5) >> 12) + 0xD0'),
    'pul2': ('((' + R + ' * 32) >> 12) + 0xD0', '(' + R + ' * 32 >> 12) + 208'),
    'pul3': ('((' + R + ' * 32) >> 12) + 0xD0', '0xD0 + ((' + R + ' * 0x20) >> 12)'),
    'pul4': ('((' + R + ' * 32) >> 12) + 0xD0', '((' + R + ' * 32) / 4096) + 0xD0'),
    # mask
    'msk1': ('((1 << i) << (pl * 4))', '(1 << i << pl * 4)'),
    'msk2': ('((1 << i) << (pl * 4))', '((1 << i) << (pl << 2))'),
    'msk3': ('*(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17) &\n                    ((1 << i) << (pl * 4))',
             '(*(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17) >> (pl * 4) >> i) & 1'),
    'msk4': ('*(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17) &\n                    ((1 << i) << (pl * 4))',
             '((1 << i) << (pl * 4)) & *(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17)'),
    'msk5': ('((1 << i) << (pl * 4))', '((1 << i) << pl * 4)'),
    # level reads
    'lv1': ('*(s16 *)(D_800A34FC + pl * 2 + 0x28) < 3', '((s16 *)(D_800A34FC + 0x28))[pl] < 3'),
    'lv2': ('*(s16 *)(D_800A34FC + pl * 2 + 0x28) < 3', '*(s16 *)(D_800A34FC + 0x28 + pl * 2) < 3'),
    'lv3': ('*(s16 *)(D_800A34FC + pl * 2 + 0x28) < 3', '*(s16 *)(pl * 2 + D_800A34FC + 0x28) < 3'),
    'lv4': ('*(s16 *)(D_800A34FC + pl * 2 + 0x28) < 3', '*(s16 *)(D_800A34FC + pl * 2 + 0x28) <= 2'),
    'lv5': ('D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17', 'D_800A3524 + 0x17 + *(s16 *)(D_800A34FC + pl * 2 + 0x28)'),
    'lv6': ('i = *(s16 *)(D_800A34FC + j * 2 + 0x28);', 'i = ((s16 *)(D_800A34FC + 0x28))[j];'),
    'lv7': ('i = *(s16 *)(D_800A34FC + j * 2 + 0x28);', 'i = *(s16 *)(D_800A34FC + 0x28 + j * 2);'),
    # rec base
    'rb1': ('rec = &recs[i + 1];', 'rec = recs + i + 1;'),
    'rb2': ('rec = &recs[i + 1];', 'rec = &recs[i] + 1;'),
    'rb3': ('rec = &recs[i + 1];', 'rec = &recs[1 + i];'),
    # x offsets
    'xo1': ('s.x = j ? 280 : 0;', 's.x = j * 280;'),
    'xo2': ('s.x = j ? 280 : 0;', 'if (j) s.x = 280; else s.x = 0;'),
    'xo3': ('s.x = j ? 280 : 0;', 's.x = j == 0 ? 0 : 280;'),
    'xo4': ('s.x = j ? 280 : 0;', 's.x = (j != 0) ? 280 : 0;'),
    'xo5': ('s.x = pl * 280;', 's.x = 280 * pl;'),
    'xo6': ('tile->x0 = tile_rec->x + j * 280;', 'tile->x0 = j * 280 + tile_rec->x;'),
    'xo7': ('        x += 280;\n', '        x = x + 280;\n'),
    # w*row
    'wr1': ('rec->x + rec->w * row + x', 'rec->w * row + rec->x + x'),
    'wr2': ('rec->x + rec->w * row + x', 'x + rec->x + rec->w * row'),
    'wr3': ('rec->x + rec->w * row + x', 'rec->x + x + rec->w * row'),
    'wr4': ('rec->x + rec->w * row + x', 'rec->x + row * rec->w + x'),
    # ot adds
    'ot1': ('g_gpu_ot_ptr + 0x20, (s32)poly', '(s32)((u32 *)g_gpu_ot_ptr + 8), (s32)poly'),
    'ot2': ('AddPrim(g_gpu_ot_ptr + 0x30, (s32)tile);', 'AddPrim((s32)((u32 *)g_gpu_ot_ptr + 12), (s32)tile);'),
    # tile fields
    'tl1': ('tile->x0 = tile_rec->x + j * 280;', 'tile->x0 = tile_rec->x + (s16)(j * 280);'),
    # j loop inits
    'hd1': ('    s.header = (u8 *)table[13];\n', '    s.header = *(u8 **)&table[13];\n'),
}
names = []
for n, (a, b) in muts.items():
    if a not in src:
        print('MISSING', n); continue
    open(f'tmp/c21c/sw1/{n}.c', 'w', newline='\n').write(src.replace(a, b, 1 if n.startswith('lv') and 'i =' not in a else -1))
    names.append(f'tmp/c21c/sw1/{n}.c')
open('tmp/c21c/sw1/list.txt', 'w', newline='\n').write(' '.join(names))
print(len(names))
