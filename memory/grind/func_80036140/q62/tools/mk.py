"""Build scratch sources/headers for the Q62 re-measurement of func_80036140.
Variants:
  ext : CdState's ReplayCamRec gains unk3A (E9A, inside the 0x44 object) and runs on through 0x80101EA7
        (unk3C s16 E9C, unk3E u16 E9E, expected_pos s32 EA0, unk44 s32 EA4); every consumer respelled.
  sep : unk3A member only; E9C / EA4 separate externs (D_80101E9C s16, D_80101EA4 s32).
  sep12: unk3A member; a separate 12-byte record at 0x80101E9C.
Inputs are HEAD's files (git show). Writes tmp/func_80036140/v/<variant>/{inc/code6cac.h, code6cac_b5.c, code6cac_b4_post.c, code6cac_b5_post.c}.
usage: python3 tmp/func_80036140/mk.py <variant> <body.c>"""
import sys, re
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm

var, body_path = sys.argv[1], sys.argv[2]
out = Path(f'tmp/func_80036140/v/{var}'); (out / 'inc').mkdir(parents=True, exist_ok=True)
import subprocess
def head(path):
    return subprocess.run(['git', 'show', 'HEAD:' + path], capture_output=True, text=True, check=True).stdout
hdr = head('include/code6cac.h')
body = Path(body_path).read_text()
b5 = head('src/code6cac_b5.c')
b4p = head('src/code6cac_b4_post.c')
b5p = head('src/code6cac_b5_post.c')

def rep(text, old, new, count=1):
    assert old in text, old
    return text.replace(old, new, count)

rec_tail_old = "    s16 unk38; /* 0x80101E98 */\n} ReplayCamRec;"
if var == 'ext':
    hdr = rep(hdr, rec_tail_old,
              "    s16 unk38; /* 0x80101E98 */\n    s16 unk3A; /* 0x80101E9A */\n    s16 unk3C; /* 0x80101E9C */\n"
              "    u16 unk3E; /* 0x80101E9E */\n    s32 expected_pos; /* 0x80101EA0 */\n    s32 unk44; /* 0x80101EA4 */\n"
              "} ReplayCamRec;")
    hdr = rep(hdr, "extern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\n", "")
    b4p = b4p.replace('g_cdread_expected_pos', 'D_80101E58.rec.expected_pos')
    b5 = b5.replace('g_cdread_expected_pos', 'D_80101E58.rec.expected_pos')
    b5p = b5p.replace('D_80101E9E', 'D_80101E58.rec.unk3E')
elif var in ('ext48', 'ext4C'):
    # CdState through 0x80101E9F (ext48) or 0x80101EA3 (ext4C); the rest separate
    if var == 'ext48':
        hdr = rep(hdr, rec_tail_old,
                  "    s16 unk38; /* 0x80101E98 */\n    s16 unk3A; /* 0x80101E9A */\n    s16 unk3C; /* 0x80101E9C */\n"
                  "    u16 unk3E; /* 0x80101E9E */\n} ReplayCamRec;")
        hdr = rep(hdr, "extern u16 D_80101E9E;\n", "extern s32 D_80101EA4;\n")
        b5p = b5p.replace('D_80101E9E', 'D_80101E58.rec.unk3E')
    else:
        hdr = rep(hdr, rec_tail_old,
                  "    s16 unk38; /* 0x80101E98 */\n    s16 unk3A; /* 0x80101E9A */\n    s16 unk3C; /* 0x80101E9C */\n"
                  "    u16 unk3E; /* 0x80101E9E */\n    s32 expected_pos; /* 0x80101EA0 */\n} ReplayCamRec;")
        hdr = rep(hdr, "extern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\n", "extern s32 D_80101EA4;\n")
        b4p = b4p.replace('g_cdread_expected_pos', 'D_80101E58.rec.expected_pos')
        b5 = b5.replace('g_cdread_expected_pos', 'D_80101E58.rec.expected_pos')
        b5p = b5p.replace('D_80101E9E', 'D_80101E58.rec.unk3E')
    body = body.replace('D_80101E58.rec.unk44', 'D_80101EA4')
elif var in ('sep', 'sep12'):
    hdr = rep(hdr, rec_tail_old, "    s16 unk38; /* 0x80101E98 */\n    s16 unk3A; /* 0x80101E9A */\n} ReplayCamRec;")
    if var == 'sep':
        hdr = rep(hdr, "extern u16 D_80101E9E;\n", "extern s16 D_80101E9C;\nextern u16 D_80101E9E;\nextern s32 D_80101EA4;\n")
        body = body.replace('D_80101E58.rec.unk3C', 'D_80101E9C').replace('D_80101E58.rec.unk44', 'D_80101EA4')
    else:
        hdr = rep(hdr, "extern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\n",
                  "typedef struct { s16 unk0; u16 unk2; s32 expected_pos; s32 unk8; } CdRead12;\nextern CdRead12 D_80101E9C;\n")
        body = body.replace('D_80101E58.rec.unk3C', 'D_80101E9C.unk0').replace('D_80101E58.rec.unk44', 'D_80101E9C.unk8')
        b4p = b4p.replace('g_cdread_expected_pos', 'D_80101E9C.expected_pos')
        b5 = b5.replace('g_cdread_expected_pos', 'D_80101E9C.expected_pos')
        b5p = b5p.replace('D_80101E9E', 'D_80101E9C.unk2')
else:
    raise SystemExit('unknown variant')

# b5: tentative definitions (Q62 COMMON model), transcribed jtbl removed, body substituted.
b5 = re.sub(r"/\* func_80036140's jump table.*?\};\n", "", b5, flags=re.S)
b5 = rep(b5, "extern s32 CdReady(s32, u8 *);\n",
         "extern s32 CdReady(s32, u8 *);\nCdlATV g_cd_atv;\nCdlATV D_800A36B8;\nu8 g_cd_result[8];\n")
b5 = inlineasm.substitute_body(b5, 'func_80036140', body)
(out / 'inc' / 'code6cac.h').write_text(hdr)
(out / 'code6cac_b5.c').write_text(b5)
(out / 'code6cac_b4_post.c').write_text(b4p)
(out / 'code6cac_b5_post.c').write_text(b5p)
print('wrote', out)
