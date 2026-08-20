void func_80073060(s32 arg0) {
    GameObj *p;
    s32 i;
    s32 j;
    p = *(GameObj **)((s32)arg0 + 0x14);
    j = 0x6F;
    do {
        if (D_800A3580) {}
        *(s16 *)((s32)p + 0x8) = j;
        j += 0x20;
        *(s16 *)((s32)p + 0xA) = 0x32;
        *(s16 *)((s32)p + 0xC) = 2;
        *(s16 *)((s32)p + 0xE) = 0x3F;
        p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    } while (j < 0x210);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x8) = 0x51;
    *(s16 *)((s32)p + 0xA) = 0x41;
    *(s16 *)((s32)p + 0xC) = 0x1E0;
    *(s16 *)((s32)p + 0xE) = 1;
    p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x8) = 0x51;
    *(s16 *)((s32)p + 0xA) = 0x61;
    *(s16 *)((s32)p + 0xC) = 0x1E0;
    *(s16 *)((s32)p + 0xE) = 1;
    p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0xA) = 0x50;
    *(s16 *)((s32)p + 0x8) = 0x51;
    *(s16 *)((s32)p + 0xC) = 0x1E0;
    *(s16 *)((s32)p + 0xE) = 2;
    p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    i = 0;
    j = 0x6A;
    do {
        s32 v;
        *(s16 *)((s32)p + 0x8) = j;
        j += 0x21;
        if (D_800A3580) {}
        v = 5 - i;
        i += 1;
        *(s16 *)((s32)p + 0xA) = 0x32;
        *(s16 *)((s32)p + 0xC) = v;
        *(s16 *)((s32)p + 0xE) = 0x3F;
        p = (GameObj *)func_80072FCC((s32)D_800A3580, (u8 *)p);
    } while (i < 5);
    i = 0;
    j = 0x211;
    do {
        s32 v;
        *(s16 *)((s32)p + 0x8) = j;
        j -= 0x20;
        if (D_800A3580) {}
        v = 5 - i;
        i += 1;
        *(s16 *)((s32)p + 0xC) = v;
        *(s16 *)((s32)p + 0xA) = 0x32;
        *(s16 *)((s32)p + 0xE) = 0x3F;
        p = (GameObj *)func_80072FCC((s32)D_800A3580, (u8 *)p);
    } while (i < 5);
    *(GameObj **)((s32)arg0 + 0x14) = p;
}
