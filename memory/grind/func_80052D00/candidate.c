extern s32 D_800A33F4;
extern s32 func_80053694(s32 *, s16 *);

typedef union {
    struct {
        s16 x;
        s16 z;
    } c;
    s32 w;
} Cell_80052D00;

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
    u8 unk24[0x38];
    s32 (*unk5C)(s32, s32);
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 unk6C;
    s32 unk70;
    s32 unk74;
    s32 unk78;
    s32 unk7C;
    s32 unk80;
    s32 unk84;
    Cell_80052D00 unk88;
    Cell_80052D00 unk8C;
    s16 unk90;
} Work_80052D00;

#define WK ((Work_80052D00 *)D_800A33F4)

/* Walks the 32x32 grid of 2000-unit cells (origin -32000) along the XZ
 * segment from the start point (+0x8/+0x10) to the end point (+0x18/+0x20),
 * DDA style: +0x88/+0x8C are the current and end cells, +0x70/+0x74 the
 * major/minor extents (swapped when Z dominates), +0x7C the 4.12 slope and
 * +0x90 the major-axis steps left. Each cell crossed goes to the per-cell
 * test at +0x5C (func_80053E9C or func_80053754) until one reports a hit;
 * func_80053694 then reads the result back out. */
s32 func_80052D00(s32 arg0, s32 arg1) {
    s32 xdir;
    s32 zdir;
    s32 swapped;

    WK->unk60 = WK->unk8 + 32000;
    WK->unk64 = WK->unk10 + 32000;
    WK->unk68 = WK->unk18 + 32000;
    WK->unk0 = 0x7FFFFFFF;
    WK->unk6C = WK->unk20 + 32000;
    WK->unk70 = WK->unk68 - WK->unk60;
    WK->unk88.c.x = WK->unk60 / 2000;
    WK->unk88.c.z = WK->unk64 / 2000;
    WK->unk8C.c.x = WK->unk68 / 2000;
    WK->unk8C.c.z = WK->unk6C / 2000;
    WK->unk74 = WK->unk6C - WK->unk64;
    if (WK->unk88.w == WK->unk8C.w) {
        if (WK->unk8 == WK->unk18 && WK->unkC == WK->unk1C && WK->unk10 == WK->unk20) {
            return 0;
        }
        WK->unk5C(WK->unk88.c.x, WK->unk88.c.z);
    } else {
        WK->unk80 = WK->unk88.c.x * 2000 + 1000;
        WK->unk84 = WK->unk88.c.z * 2000 + 1000;
        WK->unk60 -= WK->unk80;
        WK->unk64 -= WK->unk84;
        WK->unk68 -= WK->unk80;
        WK->unk6C -= WK->unk84;
        if (WK->unk70 < 0) {
            xdir = -1;
            WK->unk70 = -WK->unk70;
            WK->unk60 = -WK->unk60;
            WK->unk68 = -WK->unk68;
        } else {
            xdir = 1;
        }
        if (WK->unk74 < 0) {
            zdir = -1;
            WK->unk74 = -WK->unk74;
            WK->unk64 = -WK->unk64;
            WK->unk6C = -WK->unk6C;
        } else {
            zdir = 1;
        }
        if (WK->unk74 > WK->unk70) {
            swapped = 1;
            WK->unk80 = WK->unk70;
            WK->unk70 = WK->unk74;
            WK->unk74 = WK->unk80;
            WK->unk80 = WK->unk60;
            WK->unk60 = WK->unk64;
            WK->unk64 = WK->unk80;
            WK->unk80 = WK->unk68;
            WK->unk68 = WK->unk6C;
            WK->unk6C = WK->unk80;
        } else {
            swapped = 0;
        }
        WK->unk68 += 1000;
        WK->unk60 += 1000;
        WK->unk64 += 1000;
        WK->unk90 = WK->unk68 / 2000 - WK->unk60 / 2000 + 1;
        WK->unk7C = (WK->unk74 << 12) / WK->unk70;
        WK->unk64 -= (WK->unk60 * WK->unk7C) >> 12;
        WK->unk78 = (WK->unk7C * 2000) >> 12;
        while (--WK->unk90 != -1) {
            if ((WK->unk80 = WK->unk5C(WK->unk88.c.x, WK->unk88.c.z)) != 0) {
                break;
            }
            WK->unk64 %= 2000;
            WK->unk64 += WK->unk78;
            if (WK->unk90 == 0) {
                break;
            }
            if (WK->unk64 > 2000) {
                if (swapped) {
                    if (xdir < 0) {
                        WK->unk88.c.x--;
                    } else {
                        WK->unk88.c.x++;
                    }
                } else {
                    if (zdir < 0) {
                        WK->unk88.c.z--;
                    } else {
                        WK->unk88.c.z++;
                    }
                }
                if ((WK->unk80 = WK->unk5C(WK->unk88.c.x, WK->unk88.c.z)) != 0) {
                    break;
                }
            }
            if (swapped) {
                if (zdir < 0) {
                    WK->unk88.c.z--;
                } else {
                    WK->unk88.c.z++;
                }
            } else {
                if (xdir < 0) {
                    WK->unk88.c.x--;
                } else {
                    WK->unk88.c.x++;
                }
            }
        }
        if (WK->unk80 == 0 && WK->unk88.w != WK->unk8C.w) {
            WK->unk5C(WK->unk8C.c.x, WK->unk8C.c.z);
        }
    }
    return func_80053694((s32 *)arg0, (s16 *)arg1);
}
