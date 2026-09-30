s32 cdrom_StartAudio(s32 arg0, s32 arg1) {
    if (D_80101E58.rec.unk02 != 0) {
        return 0;
    }

    {
        extern u8 g_cd_file_table;
        D_80101E58.rec.unk00 = arg0;
        D_80101E58.rec.pair = *(CamPair *)(&g_cd_file_table + D_80101E58.rec.unk00 * 8);
    }

    {
        extern u8 g_cd_file_table;
        D_80101E58.rec.unk14 = CdPosToInt((s32)(&g_cd_file_table + D_80101E58.rec.unk00 * 8)) + (*(u32 *)((u8 *)&g_cd_file_table_plus_0x4 + (D_80101E58.rec.unk00 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        D_80101E58.rec.unk34 = 0;
        D_80101E58.rec.unk30 = 5;
    } else {
        D_80101E58.rec.unk34 = 1;
        D_80101E58.file = 1;
        D_80101E58.chan = arg1;
        CdControlB(0xD, &D_80101E58.file, 0);
        D_80101E58.rec.unk30 = 0xC8;
    }

    D_80101E58.rec.unk04 = 0;
    D_80101E58.rec.unk08 = 0;
    D_80101E58.rec.unk0A = 0;
    D_80101E58.rec.unk02 = 0x10;

    return 1;
}
