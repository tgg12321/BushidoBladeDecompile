void func_80026DA4(void) {
    u8 *rec;
    u8 *other;
    u8 *win;
    u8 *lose;
    u8 *p;
    s32 timer;
    s32 i;
    s32 idx;
    s32 dir;
    s32 kind;
    s32 pos[3];

    timer = func_8002BEA0();
    rec = (u8 *)&D_80101EC8;
    D_800A3824 = -1;
    if (D_80101F32 == 0x1C) {
        if (D_80101F08 != 4) goto tail;
        idx = D_800A3876;
        if (idx == -1) goto tail;
        win = (u8 *)&D_80101EC8 + idx * 0x44C;
        lose = (u8 *)&D_80101EC8;
        if (idx == 0) {
            lose += 0x44C;
        }
        *(s16 *)(win + 0x286) = 3;
        *(s16 *)(lose + 0x286) = 4;
    } else if (D_80101F32 != 0xF) {
        for (i = 0; i < 2; i++) {
            p = rec + i * 0x44C;
            if (*(s32 *)(p + 0x30) & 0x20) {
                *(s32 *)(p + 0x28C) += *(s16 *)(p + 0x20);
            }
            if (*(s32 *)(p + 0x30) & 0x40) {
                *(s32 *)(p + 0x28C) += *(s16 *)(p + 0x20);
            }
            if (*(u16 *)(p + 0x6A) == 0x1D || *(u16 *)(p + 0x6A) == 0x1E ||
                *(u16 *)(p + 0x6A) == 0x20) {
                if (*(s32 *)(p + 0x2C) & 0x1000) {
                    dir = 1;
                } else if (*(s32 *)(p + 0x2C) & 0x4000) {
                    dir = -1;
                } else {
                    dir = 0;
                }
                *(s32 *)(p + 0x134) -= ((&Judge)[(*(s16 *)(p + 0x1CA) + 0x400) & 0xFFF] * dir) / 256;
                *(s32 *)(p + 0x13C) += ((&Judge)[*(u16 *)(p + 0x1CA) & 0xFFF] * dir) / 256;
            }
        }
        rec = (u8 *)&D_80101EC8;
        other = rec + 0x44C;
        D_800A389C++;
        if ((s16)D_800A389C >= 0x2B) {
            if (D_80102154 > D_801025A0) {
                D_8010214E = 3;
                D_8010259A = 4;
                if (D_8010237E == 0x21) {
                    func_80027A58((s32 *)other);
                }
                func_80032854(1, 0x2D, rec + 0x540, 0);
            } else if (D_801025A0 > D_80102154) {
                D_8010259A = 3;
                D_8010214E = 4;
                if (D_80101F32 == 0x21) {
                    func_80027A58((s32 *)rec);
                }
                func_80032854(0, 0x2D, rec + 0xF4, 0);
            }
            *(s32 *)(other + 0x28C) = 0;
            *(s32 *)(rec + 0x28C) = 0;
            D_800A389C = 0;
        }
        if (timer > 200) {
            *(s16 *)(rec + 0x286) = 2;
            *(s16 *)(other + 0x286) = 2;
        } else {
            s32 diff = *(s32 *)(rec + 0xF8) - *(s32 *)(other + 0xF8);
            if (diff < 0) diff = -diff;
            if (diff >= 1000) {
                *(s16 *)(rec + 0x286) = 2;
                *(s16 *)(other + 0x286) = 2;
            }
        }
        if (*(u16 *)(rec + 0x6A) == 0x1D) {
            if (*(s32 *)(rec + 0x2C) & 0x8000) {
                if (*(s32 *)(other + 0x2C) & 0x8000) {
                    *(s16 *)(rec + 0x286) = 2;
                    *(s16 *)(other + 0x286) = 2;
                } else {
                    *(s16 *)(rec + 0x286) = 2;
                    *(s16 *)(other + 0x286) = 5;
                }
            } else if (*(s32 *)(other + 0x2C) & 0x8000) {
                *(s16 *)(rec + 0x286) = 5;
                *(s16 *)(other + 0x286) = 2;
            }
        }
    }
    rec = (u8 *)&D_80101EC8;
tail:
    other = rec + 0x44C;
    if (D_800A3910 == 0) {
        switch (D_80101F32) {
        case 0xF:
            kind = -1;
            break;
        case 0x1C:
            kind = 0;
            break;
        case 0x21:
            kind = 1;
            break;
        case 0x20:
            kind = 2;
            break;
        case 0x1D:
            kind = 3;
            break;
        case 0x1E:
            kind = 4;
            break;
        case 0x1F:
            kind = 5;
            break;
        }
        if (kind >= 0) {
            pos[0] = (*(s32 *)(rec + 0xF4) + *(s32 *)(other + 0xF4)) / 2;
            pos[1] = (*(s32 *)(rec + 0xDC) + *(s32 *)(other + 0xDC)) / 2;
            pos[2] = (*(s32 *)(rec + 0xFC) + *(s32 *)(other + 0xFC)) / 2;
            pos[0] += ((&Judge)[*(u16 *)(rec + 0x1D8) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            pos[1] += D_8008EB54[kind].unk2;
            pos[2] += ((&Judge)[(*(s16 *)(rec + 0x1D8) + 0x400) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            func_80032854(0, D_8008EB6C[kind], (u8 *)pos, 0);
        }
    } else {
        D_800A3910--;
    }
}
