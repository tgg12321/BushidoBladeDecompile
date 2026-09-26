extern s32 D_800A33F0;

s32 func_80053754(s32 arg0, s32 arg1) {
    s32 n;
    s32 data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 hdr;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    W->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (W->unkE0 == 0xFFFF) {
        return 0;
    }
    data = D_800A33F0 + W->unkE0;
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    W->unk4C = W->unk8 - x;
    W->unk4E = W->unkC;
    W->unk50 = W->unk10 - z;
    W->unk54 = W->unk18 - x;
    W->unk56 = W->unk1C;
    W->unk58 = W->unk20 - z;

    count = *(s16 *)data;
    data += 2;
    while (--count != -1) {
        W->unkD0 = *(s16 *)data;
        data += 2;
        W->unkD4 = *(s16 *)data;
        data += 2;
        W->unkD8 = *(s16 *)data;
        data += 2;
        W->unkDC = *(u16 *)data;
        data += 2;
        W->unkDC = (*(s16 *)data << 16) | W->unkDC;
        data += 2;
        W->unkE4 = W->unkD0 * W->unk4C + W->unkD4 * W->unk4E + W->unkD8 * W->unk50 + W->unkDC;
        W->unkE8 = W->unkD0 * W->unk54 + W->unkD4 * W->unk56 + W->unkD8 * W->unk58 + W->unkDC;

        W->unkE4 = (W->unkE4 < 0 ? -1 : 1) * ((W->unkE4 < 0 ? -W->unkE4 : W->unkE4) >> 10);
        W->unkE8 = (W->unkE8 < 0 ? -1 : 1) * ((W->unkE8 < 0 ? -W->unkE8 : W->unkE8) >> 10);

        if (W->unkE4 >= 0 && W->unkE8 < 0) {
            W->unkE0 = W->unkE4 - W->unkE8;
            W->unkE4 *= 2;
            W->unkA8 = (W->unk54 - W->unk4C) * W->unkE4 / W->unkE0;
            W->unkAC = (W->unk56 - W->unk4E) * W->unkE4 / W->unkE0;
            W->unkB0 = (W->unk58 - W->unk50) * W->unkE4 / W->unkE0;
            W->unkA8 = (W->unkA8 < 0 ? -1 : 1) * ((W->unkA8 >= 0 ? W->unkA8 + 1 : -W->unkA8 + 1) >> 1);
            W->unkAC = (W->unkAC < 0 ? -1 : 1) * ((W->unkAC >= 0 ? W->unkAC + 1 : -W->unkAC + 1) >> 1);
            W->unkB0 = (W->unkB0 < 0 ? -1 : 1) * ((W->unkB0 >= 0 ? W->unkB0 + 1 : -W->unkB0 + 1) >> 1);
            W->unkA8 += W->unk4C;
            W->unkAC += W->unk4E;
            W->unkB0 += W->unk50;
            W->unkE0 = *(s16 *)data;
            data += 2;
            W->unkE4 = *(s16 *)data;
            data += 2;
            W->unkE8 = *(s16 *)data;
            data += 2;
            W->unk9C = *(s16 *)data;
            data += 2;
            W->unkA0 = *(s16 *)data;
            data += 2;
            W->unkA4 = *(s16 *)data;
            data += 2;
            W->unkC4 = ((W->unkA8 - W->unkE0) * W->unk9C + (W->unkAC - W->unkE4) * W->unkA0 + (W->unkB0 - W->unkE8) * W->unkA4) >> 14;
            W->unk9C = *(s16 *)data;
            data += 2;
            W->unkA0 = *(s16 *)data;
            data += 2;
            W->unkA4 = *(s16 *)data;
            data += 2;
            W->unkC8 = ((W->unkA8 - W->unkE0) * W->unk9C + (W->unkAC - W->unkE4) * W->unkA0 + (W->unkB0 - W->unkE8) * W->unkA4) >> 14;
            hdr = *(u16 *)data;
            data += 2;
            n = hdr;
            W->unkCC = n >> 8;
            n &= 0xFF;
            W->unkB4 = *(s16 *)data;
            data += 2;
            y = *(s16 *)data;
            data += 2;
            W->unkE0 = 1;
            W->unkB8 = y;
            while (--n != -1) {
                W->unkBC = *(s16 *)data;
                data += 2;
                W->unkC0 = *(s16 *)data;
                data += 2;
                if ((W->unkC4 - W->unkB4) * (W->unkC0 - W->unkB8)
                    - (W->unkC8 - W->unkB8) * (W->unkBC - W->unkB4) > 0) {
                    W->unkE0 = 0;
                    break;
                }
                W->unkB4 = W->unkBC;
                W->unkB8 = W->unkC0;
            }
            if (n > 0) {
                data += n * 4;
            }
            if (W->unkE0 != 0) {
                if ((W->unkE0 = gte_SumSquares3(W->unkA8 - W->unk4C, W->unkAC - W->unk4E, W->unkB0 - W->unk50)) < W->unk0) {
                    W->unk48 = arg0;
                    W->unk4A = arg1;
                    W->unk38 = W->unkA8;
                    W->unk3C = W->unkAC;
                    W->unk40 = W->unkB0;
                    W->unk28 = W->unkD0;
                    W->unk2C = W->unkD4;
                    W->unk30 = W->unkD8;
                    W->unk34 = W->unkDC;
                    W->unk0 = W->unkE0;
                    W->unk4 = W->unkCC;
                }
            }
        } else {
            data += 18;
            n = *(s16 *)data;
            data += 2;
            n &= 0xFF;
            data += (n + 1) * 4;
        }
    }
    return W->unk0 != 0x7FFFFFFF;
}
