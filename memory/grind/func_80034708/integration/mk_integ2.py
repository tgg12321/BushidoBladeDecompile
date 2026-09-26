"""Stage 2 scratch integration: on top of mk_integ.py (0x8010277C aggregate), merge the
0x80106A50 file record (0x24 bytes, checksummed as one block by func_80037F40)."""
import os, re, subprocess
R = 'tmp/func_80034708/integ'
subprocess.run(['python3', 'tmp/func_80034708/mk_integ.py'], check=True)

def rd(p):
    return open(p, encoding='utf-8').read()

def wr(p, s):
    open(p, 'w', encoding='utf-8', newline='\n').write(s)

# --- header: system.h gets the record, code6cac.h loses D_80106A70[3]
h = rd('include/system.h')
old = 'extern u8 g_file_disc_type;\n'
assert old in h
new = '''/* The 0x24-byte file record at 0x80106A50 (func_80037F40 checksums it as one
 * block; func_800167EC initialises it). */
typedef struct {
    u8 unk_0;
    u8 unk_1;
    s32 unk_4;
} FileTimeRec;
typedef struct {
    s32 unk_00;             /* 0x80106A50 */
    u8 unk_04;              /* 0x80106A54 */
    u8 unk_05[3];
    FileTimeRec times[3];   /* 0x80106A58 */
    u8 color[3];            /* 0x80106A70 */
    u8 flags;               /* 0x80106A73: bits 0/1/2 = file_GetFlag0/1/2 */
} FileRecord;
extern FileRecord D_80106A50;
'''
h = h.replace(old, new)
wr(R + '/include/system.h', h)
h = rd(R + '/include/code6cac.h')
assert 'extern u8 D_80106A70[3];\n' in h
h = h.replace('extern u8 D_80106A70[3];\n', '')
wr(R + '/include/code6cac.h', h)

DROP = re.compile(r'^extern (s32|u32|u8) (D_80106A50|D_80106A58|D_80106A5C|D_80106A73|g_file_flags|g_file_disc_size);\s*$')
subs = [
    (r'\(u8 \*\)&g_file_disc_size', '(u8 *)&D_80106A50'),
    (r'\(Quad \*\)&g_file_disc_size', '(Quad *)&D_80106A50'),
    (r'&g_file_disc_size', '&D_80106A50.unk_00'),
    (r'\bg_file_disc_size\b', 'D_80106A50.unk_00'),
    (r'&g_file_disc_type', '&D_80106A50.unk_04'),
    (r'\bg_file_disc_type\b', 'D_80106A50.unk_04'),
    (r'\(u8 \*\)&D_80106A58', '(u8 *)D_80106A50.times'),
    (r'\bD_80106A5C\b', 'D_80106A50.times[0].unk_4'),
    (r'\(Quad \*\)&D_80106A70', '(Quad *)D_80106A50.color'),
    (r'\bD_80106A70\[', 'D_80106A50.color['),
    (r'&D_80106A73\b', '&D_80106A50.flags'),
    (r'\bD_80106A73\b', 'D_80106A50.flags'),
    (r'\bg_file_flags\b', 'D_80106A50.flags'),
    (r'\(struct HitRec \*\)&D_80106A50', '(struct HitRec *)&D_80106A50'),
    (r'\(CopyBlock \*\)&D_80106A50', '(CopyBlock *)&D_80106A50'),
    (r'(?<![&.\w])D_80106A50(?![.\w])', 'D_80106A50.unk_00'),
]
stems = ['ings', 'code6cac', 'code6cac_b', 'code6cac_b2_pre', 'code6cac_b2_post', 'code6cac_c2',
         'code6cac_c_ab', 'code6cac_c_mid', 'replay_camera_rob_back_loose2']
ONLY_TYPE = {'ings'}  # stage3 overwrites ings.c
for stem in stems:
    p = f'{R}/src/{stem}.c'
    s = rd(p) if os.path.exists(p) else rd(f'src/{stem}.c')
    out = []
    for line in s.split('\n'):
        if stem not in ONLY_TYPE and DROP.match(line):
            continue
        st = line.lstrip()
        if not re.match(r'(/\*|\*\s|\*/|\*$)', st):
            for a, b in (subs if stem not in ONLY_TYPE else subs[4:6]):
                line = re.sub(a, b, line)
        out.append(line)
    wr(p, '\n'.join(out))
print('ok')
