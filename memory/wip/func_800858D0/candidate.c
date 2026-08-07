void title_mv_exec2(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;
    s32 t;

    var_s0 = 0;
    buf[1] = 0x60093;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    if (D_80101BCC != 0) {
        ff = 0xFF;
        do {
            offset = (s16)var_s0 * 54;
            t = 0x18;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = t;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            t = 1;
            buf[0] = t << (s16)var_s0;
            saTan1MainJump(buf);
            D_8010280A = var_s0;
            motutil_GetAngTableNum(1);
            var_s0 = var_s0 + 1;
        } while ((s16)var_s0 < (s32)D_80101BCC);
    }
}
