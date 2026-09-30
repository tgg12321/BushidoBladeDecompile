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

    W->unk60 = W->unk8 + 32000;
    W->unk64 = W->unk10 + 32000;
    W->unk68 = W->unk18 + 32000;
    W->unk0 = 0x7FFFFFFF;
    W->unk6C = W->unk20 + 32000;
    W->unk70 = W->unk68 - W->unk60;
    W->unk88.c.x = W->unk60 / 2000;
    W->unk88.c.z = W->unk64 / 2000;
    W->unk8C.c.x = W->unk68 / 2000;
    W->unk8C.c.z = W->unk6C / 2000;
    W->unk74 = W->unk6C - W->unk64;
    if (W->unk88.w == W->unk8C.w) {
        if (W->unk8 == W->unk18 && W->unkC == W->unk1C && W->unk10 == W->unk20) {
            return 0;
        }
        W->unk5C(W->unk88.c.x, W->unk88.c.z);
    } else {
        W->unk80 = W->unk88.c.x * 2000 + 1000;
        W->unk84 = W->unk88.c.z * 2000 + 1000;
        W->unk60 -= W->unk80;
        W->unk64 -= W->unk84;
        W->unk68 -= W->unk80;
        W->unk6C -= W->unk84;
        if (W->unk70 < 0) {
            xdir = -1;
            W->unk70 = -W->unk70;
            W->unk60 = -W->unk60;
            W->unk68 = -W->unk68;
        } else {
            xdir = 1;
        }
        if (W->unk74 < 0) {
            zdir = -1;
            W->unk74 = -W->unk74;
            W->unk64 = -W->unk64;
            W->unk6C = -W->unk6C;
        } else {
            zdir = 1;
        }
        if (W->unk74 > W->unk70) {
            swapped = 1;
            W->unk80 = W->unk70;
            W->unk70 = W->unk74;
            W->unk74 = W->unk80;
            W->unk80 = W->unk60;
            W->unk60 = W->unk64;
            W->unk64 = W->unk80;
            W->unk80 = W->unk68;
            W->unk68 = W->unk6C;
            W->unk6C = W->unk80;
        } else {
            swapped = 0;
        }
        W->unk68 += 1000;
        W->unk60 += 1000;
        W->unk64 += 1000;
        W->unk90 = W->unk68 / 2000 - W->unk60 / 2000 + 1;
        W->unk7C = (W->unk74 << 12) / W->unk70;
        W->unk64 -= (W->unk60 * W->unk7C) >> 12;
        W->unk78 = (W->unk7C * 2000) >> 12;
        while (--W->unk90 != -1) {
            if ((W->unk80 = W->unk5C(W->unk88.c.x, W->unk88.c.z)) != 0) {
                break;
            }
            W->unk64 %= 2000;
            W->unk64 += W->unk78;
            if (W->unk90 == 0) {
                break;
            }
            if (W->unk64 > 2000) {
                if (swapped) {
                    if (xdir < 0) {
                        W->unk88.c.x--;
                    } else {
                        W->unk88.c.x++;
                    }
                } else {
                    if (zdir < 0) {
                        W->unk88.c.z--;
                    } else {
                        W->unk88.c.z++;
                    }
                }
                if ((W->unk80 = W->unk5C(W->unk88.c.x, W->unk88.c.z)) != 0) {
                    break;
                }
            }
            if (swapped) {
                if (zdir < 0) {
                    W->unk88.c.z--;
                } else {
                    W->unk88.c.z++;
                }
            } else {
                if (xdir < 0) {
                    W->unk88.c.x--;
                } else {
                    W->unk88.c.x++;
                }
            }
        }
        if (W->unk80 == 0 && W->unk88.w != W->unk8C.w) {
            W->unk5C(W->unk8C.c.x, W->unk8C.c.z);
        }
    }
    return func_80053694((s32 *)arg0, (s16 *)arg1);
}
