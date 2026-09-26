"""model4 = current tree + the E58..E99 record merge spelled for real (no macros):
CdState D_80101E58 { CdlFILTER filter; s32 unk04; ReplayCamRec rec; } with ReplayCamRec extended through
0x80101E99 (unk38). E9A..EA7 stay split scalars. func_80036140 stays INCLUDE_ASM. -G0 in place.
usage: python3 tmp/func_80036940/mk_model4.py [outdir]   (default tmp/func_80036940/model4)"""
import re, sys
from pathlib import Path
out = Path(sys.argv[1] if len(sys.argv) > 1 else 'tmp/func_80036940/model4')
(out / 'include').mkdir(parents=True, exist_ok=True); (out / 'src').mkdir(parents=True, exist_ok=True)

h = Path('include/code6cac.h').read_text()
def rm(t, line):
    assert t.count(line + '\n') == 1, line
    return t.replace(line + '\n', '')
h = rm(h, 'extern u8 D_80101E59;')
h = rm(h, 'extern s32 D_80101E5C;')
old = """    CamPair pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14; /* 0x80101E74 */
} ReplayCamRec;

extern ReplayCamRec D_80101E60;
"""
new = """    CamPair pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14; /* 0x80101E74 */
    s32 unk18; /* 0x80101E78 */
    s32 unk1C; /* 0x80101E7C */
    s32 sectors_remaining; /* 0x80101E80 */
    s32 dest_buffer; /* 0x80101E84 */
    s32 unk28; /* 0x80101E88 */
    s32 unk2C; /* 0x80101E8C */
    u8 unk30; /* 0x80101E90 */
    s32 unk34; /* 0x80101E94 */
    s16 unk38; /* 0x80101E98 */
    s16 unk3A; /* 0x80101E9A */
} ReplayCamRec;

/* libcd CdlFILTER (CdlSetfilter parameter). */
typedef struct {
    u8 file;
    u8 chan;
    u16 pad;
} CdlFILTER;

typedef struct {
    CdlFILTER filter; /* 0x80101E58 */
    s32 unk04; /* 0x80101E5C */
    ReplayCamRec rec; /* 0x80101E60 .. 0x80101E99 */
} CdState;

extern CdState D_80101E58;
"""
assert h.count(old) == 1
h = h.replace(old, new)
for d in ['extern s32 D_80101E78;', 'extern s32 D_80101E7C;', 'extern s32 g_cdread_sectors_remaining;',
          'extern s32 g_cdread_dest_buffer;', 'extern s32 D_80101E88;', 'extern s32 D_80101E8C;',
          'extern u8 D_80101E90;', 'extern s32 D_80101E94;', 'extern s16 D_80101E98;', 'extern s16 D_80101E9A;']:
    h = rm(h, d)
(out / 'include/code6cac.h').write_text(h)

s = Path('src/code6cac_b2_post.c').read_text()
subs = [(r'\bD_80101E60\b', 'D_80101E58.rec'),
        (r'\bD_80101E78\b', 'D_80101E58.rec.unk18'), (r'\bD_80101E7C\b', 'D_80101E58.rec.unk1C'),
        (r'\bg_cdread_sectors_remaining\b', 'D_80101E58.rec.sectors_remaining'),
        (r'\bg_cdread_dest_buffer\b', 'D_80101E58.rec.dest_buffer'),
        (r'\bD_80101E88\b', 'D_80101E58.rec.unk28'), (r'\bD_80101E8C\b', 'D_80101E58.rec.unk2C'),
        (r'\bD_80101E90\b', 'D_80101E58.rec.unk30'), (r'\bD_80101E94\b', 'D_80101E58.rec.unk34'),
        (r'\bD_80101E98\b', 'D_80101E58.rec.unk38'), (r'\bD_80101E5C\b', 'D_80101E58.unk04')]
for a, b in subs:
    s = re.sub(a, b, s)
old_sa = """        u8 *base = (u8 *)s0 - 0xA;
        D_80101E58.rec.unk34 = 1;
        *base = 1;
        D_80101E59 = arg1;
        CdControlB(0xD, base, 0);
"""
new_sa = """        D_80101E58.rec.unk34 = 1;
        D_80101E58.filter.file = 1;
        D_80101E58.filter.chan = arg1;
        CdControlB(0xD, (u8 *)&D_80101E58.filter, 0);
"""
assert s.count(old_sa) == 1
s = s.replace(old_sa, new_sa)
assert 'D_80101E59' not in s
(out / 'src/code6cac_b2_post.c').write_text(s)
print('ok', out)
