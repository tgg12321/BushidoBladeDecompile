/* Q43 evidence model, NOT for main: CdState cut at 0x80101E6C.  The tail
 * 0x80101E6C..0x80101E99 is its own object (g_cd_loc is the existing linker
 * name at 0x80101E6C); the head is reached through D_80101E58 as on main.
 * sched/cse see only the base symbol_ref, so this is the two-object layout for
 * every access cdrom_StartAudio makes. */
typedef struct {
    CamPair pair;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 sectors_remaining;
    s32 dest_buffer;
    s32 unk28;
    s32 unk2C;
    u8 unk30;
    s32 unk34;
    s16 unk38;
} CdStateTail;
extern CdStateTail g_cd_loc;
s32 cdrom_StartAudio(s32 arg0, s32 arg1) {
    if (D_80101E58.rec.unk02 != 0) {
        return 0;
    }

    {
        extern u8 g_cd_file_table;
        CamPair *e;
        D_80101E58.rec.unk00 = arg0;
        e = (CamPair *)(&g_cd_file_table + D_80101E58.rec.unk00 * 8);
        g_cd_loc.pair.a = e->a;
        g_cd_loc.pair.b = e->b;
    }

    {
        extern u8 g_cd_file_table;
        g_cd_loc.unk14 = CdPosToInt((s32)(&g_cd_file_table + D_80101E58.rec.unk00 * 8)) + (*(u32 *)((u8 *)&g_cd_file_table_plus_0x4 + (D_80101E58.rec.unk00 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        g_cd_loc.unk34 = 0;
        g_cd_loc.unk30 = 5;
    } else {
        g_cd_loc.unk34 = 1;
        D_80101E58.file = 1;
        D_80101E58.chan = arg1;
        CdControlB(0xD, &D_80101E58.file, 0);
        g_cd_loc.unk30 = 0xC8;
    }

    D_80101E58.rec.unk04 = 0;
    D_80101E58.rec.unk08 = 0;
    D_80101E58.rec.unk0A = 0;
    D_80101E58.rec.unk02 = 0x10;

    return 1;
}
