import sys, re
from pathlib import Path
p = Path(sys.argv[1])
t = p.read_text()
old_struct = """    CamPair pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14; /* 0x80101E74 */
} ReplayCamRec;"""
new_struct = """    CamPair pair; /* 0x80101E6C .. 0x80101E73 */
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
    s16 unk3C; /* 0x80101E9C */
    u16 unk3E; /* 0x80101E9E */
    s32 expected_pos; /* 0x80101EA0 */
    s32 unk44; /* 0x80101EA4 */
} ReplayCamRec;"""
assert old_struct in t
t = t.replace(old_struct, new_struct)
for d in ["extern s32 D_80101E78;", "extern s32 D_80101E7C;", "extern s32 g_cdread_sectors_remaining;",
          "extern s32 g_cdread_dest_buffer;", "extern s32 D_80101E88;", "extern s32 D_80101E8C;",
          "extern u8 D_80101E90;", "extern s32 D_80101E94;", "extern s16 D_80101E98;", "extern s16 D_80101E9A;",
          "extern s16 D_80101E9C;", "extern u16 D_80101E9E;", "extern s32 g_cdread_expected_pos;", "extern s32 D_80101EA4;"]:
    assert t.count(d + "\n") == 1, d
    t = t.replace(d + "\n", "")
open(p, 'w', newline='
').write(t)
