s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    u8 sound;
    u8 *base;
    register s32 delta asm("$6");
    register u8 *p asm("$3");
    register u8 *q asm("$5");
    register s32 i asm("$4");

    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    {
        s32 idx = *(s16 *)(temp_v0 + 8);
        D_800A33E0 = arg0;
        sound = (&D_80099BCC)[idx];
    }
    if (sound == 0xFF) return 0;
    base = (u8 *)snd_LoadBgm(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    q = p + 0xA;
    D_800A33E4 = (s32)p;
    i = 0;
    do {
        *(s16 *)(q - 8) = i;
        *(s16 *)(q - 6) = 9;
        *p = 0xF;
        *(s8 *)(q - 9) = 0;
        *(s16 *)q = (s16)arg0;
        q += 0x68;
        i += 1;
        p += 0x68;
    } while (i < 0x11);
    snd_PlayBgm(delta + 0x6E8);
    return 1;
}
