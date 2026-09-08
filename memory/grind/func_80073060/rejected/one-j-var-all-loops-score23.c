typedef struct TileXy {
    s32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TileXy;
void func_80073060(s32 arg0) {
    TileXy *p;
    s32 i;
    s32 j;
    p = *(TileXy **)((s32)arg0 + 0x14);
    for (j = 0x6F; j < 0x210; j += 0x20) {
        p->x0 = j;
        p->y0 = 0x32;
        p->w = 2;
        p->h = 0x3F;
        p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    }
    p->x0 = 0x51;
    p->y0 = 0x41;
    p->w = 0x1E0;
    p->h = 1;
    p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    p->x0 = 0x51;
    p->y0 = 0x61;
    p->w = 0x1E0;
    p->h = 1;
    p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    p->x0 = 0x51;
    p->y0 = 0x50;
    p->w = 0x1E0;
    p->h = 2;
    p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    for (i = 0, j = 0x6A; i < 5; i++, j += 0x21) {
        p->x0 = j;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = (TileXy *)func_80072FCC(D_800A3580, (u8 *)p);
    }
    for (i = 0, j = 0x211; i < 5; i++, j -= 0x20) {
        p->x0 = j;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = (TileXy *)func_80072FCC(D_800A3580, (u8 *)p);
    }
    *(TileXy **)((s32)arg0 + 0x14) = p;
}
