extern u8 D_8009A8C4[][8][4];
extern u8 D_8009A9B4[][2];
extern u32 file_GetFlag1(void);
extern s32 rand(void);
void func_80055138(s32 arg0, u16 *arg1, u16 *arg2) {
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    u8 *src;
    u8 *pair;
    u8 (*row)[4];
    s32 lv;
    s32 idx;
    u8 base;
    s32 i;
    s32 sec;
    u8 *rec;
    u16 *cursor;
    u8 *list;
    s32 mask;
    s32 bit;
    s32 lo, hi1, hi2;
    s32 v;
    s32 m;
    u32 cat;
    s32 bonus;
    s32 a, b, c;
    u8 *other;

    p[0x443] = *(u16 *)(p + 0xA);
    *(s16 *)(p + 0x438) = *(u16 *)(p + 8);
    switch (D_800A38DC) {
    case 1:
        *(s16 *)(p + 0x438) = (D_800A3783 - 1) / 5 * 0x300 + 0x400;
        if (*(s16 *)(p + 0x438) > 0xD00) {
            *(s16 *)(p + 0x438) = 0xD00;
        }
        break;
    case 0:
        if (D_800A3680 == D_800A3671) {
            func_8005509C(*(s16 *)(p + 4));
        }
        if (D_80099D88[p[0x443]].flags & 0x300) {
            row = D_8009A8C4[*(s16 *)(p + 0x86)];
            src = row[D_800A37A0];
            p[0x424] = src[0];
            p[0x3F6] = src[1];
        }
        break;
    case 2:
        if (D_800A389A) {
            v = D_800A37D2 / 5;
            *(s16 *)(p + 0x438) = v * 0x180 + 0x280;
            if (*(s16 *)(p + 0x438) > 0x1000) {
                *(s16 *)(p + 0x438) = 0x1000;
            }
            if ((u8)(D_800A37D2 % 5) == 0) {
                func_8005509C(*(s16 *)(p + 4));
            }
        } else {
            v = D_800A37D2 / 3;
            if (v >= 3) {
                D_800A37D2 = 0;
                v = 0;
            }
            p[0x443] = 0x19;
            *(s16 *)(p + 0x1C) = (v + 2) << 10;
            *(s16 *)(p + 0x438) = 0;
            p[0x424] = 0;
            p[0x3F6] = 0x3C - v * 15;
        }
        break;
    case 3:
        p[0x443] = cpu_practice_honmokuroku_data_tbl[D_800A38E2 - 1][0] + 0x1B;
        base = D_800A38E2 / 10;
        *(s16 *)(p + 0x438) = base * 16 + 0x80;
        if (D_80099D88[p[0x443]].flags & 0x3000) {
            *(s16 *)(p + 0x438) = base * 16 + 0x180;
        }
        if (D_80099D88[p[0x443]].flags & 0x4000) {
            *(s16 *)(p + 0x438) += 0x200;
        }
        if ((D_800A38E2 - 1) % 10 == 0) {
            func_8005509C(*(s16 *)(p + 4));
        }
        v = (u8)(D_800A38E2 / 10) * 2;
        if ((u8)(D_800A38E2 % 10) == 0) {
            v--;
        }
        pair = D_8009A9B4[v];
        p[0x424] = pair[0];
        p[0x3F6] = pair[1];
        break;
    }
    if (file_GetFlag1() && D_800A38DC != 3) {
        *(s16 *)(p + 0x438) = *(s16 *)(p + 0x438) * 11 >> 4;
    }
    if (!(D_80099D88[p[0x443]].flags & 0xFF00)) {
        p[0x424] = 0x11 - (*(s16 *)(p + 0x438) >> 8);
    }
    *(s16 *)(p + 0x39A) = 0x8000 / *(s16 *)(p + 0x1C);
    p[0x3BD] = 0x10 - (*(s16 *)(p + 0x438) >> 8);
    if (D_80099D88[p[0x443]].flags & 0x100) {
        D_80099D88[p[0x443]].unk3 = (rand() & 3) + 1;
    }
    for (i = 0; i < 8U; i++) {
        (p + i)[0x444] = 0;
    }
    *(u16 **)(p + 0x3A4) = arg1;
    for (i = 0; i < 2; i++) {
        if (i) {
            rec = *(u8 **)p;
            cursor = arg2;
            list = (u8 *)cursor;
            mask = *(s16 *)(rec + 0xA);
        } else {
            rec = p;
            cursor = arg1;
            list = (u8 *)cursor;
            mask = p[0x443];
        }
        *(s16 *)(rec + 0x40A) = (*(s16 *)(rec + 0x1A) - 0x1000) * 225 >> 11;
        for (sec = 0; sec < 3; sec++) {
            bit = 1 << mask;
            lo = 0xFFFF;
            hi2 = 0;
            hi1 = 0;
            if (i == 0) {
                *(u16 **)(p + 0x3A8 + sec * 4) = cursor;
            }
            while (*cursor != 0) {
                u8 *e = list + *cursor;
                if (e[4] == 0x40) {
                    v = (e[8] << 24) | (e[7] << 16) | (e[6] << 8) | e[5];
                    if (!(v & bit)) {
                        goto next;
                    }
                }
                if (e[1] != 0 && e[1] != 0xFF) {
                    v = e[1];
                    if (v < lo) {
                        lo = v;
                    }
                }
                if (e[2] != 0 && e[2] != 0xFF) {
                    v = e[2];
                    if (hi1 < v) {
                        hi1 = v;
                    }
                    cat = e[0] & 7;
                    if (hi2 < v && (e[3] & 0xF) * 4 < 0x10 && (cat < 2 || cat == 7)) {
                        hi2 = v;
                    }
                }
            next:
                cursor++;
            }
            if (*(s16 *)(rec + 0xE) >= 6) {
                a = 0;
                c = 0x7530;
                b = 0x7530;
            } else {
                c = *(s16 *)(rec + 0x40A) + 100;
                a = c + lo * 40;
                b = c + hi1 * 40;
                c += hi2 * 40;
            }
            cursor++;
            ((s16 *)(rec + 0x3F8))[sec] = a;
            ((s16 *)(rec + 0x3FE))[sec] = b;
            ((s16 *)(rec + 0x404))[sec] = c;
        }
    }
    other = *(u8 **)p;
    *(s8 *)(p + 0x40D) = -1;
    *(s8 *)(p + 0x40C) = -1;
    *(s16 *)(p + 0x428) = -1;
    p[0x425] = 0;
    p[0x426] = 0;
    *(s32 *)(p + 0x3B4) = 0;
    *(s32 *)(p + 0x3E0) = 0;
    *(s32 *)(p + 0x3DC) = 0;
    *(s32 *)(p + 0x3D8) = 0;
    other[0x440] = 0;
    p[0x440] = 0;
    other[0x441] = 0;
    p[0x441] = 0;
    *(s16 *)(other + 0x43C) = 0;
    *(s16 *)(other + 0x43A) = 0;
    *(s16 *)(p + 0x43C) = 0;
    *(s16 *)(p + 0x43A) = 0;
    p[0x362] = 0;
    p[0x39D] = 0;
    p[0x3F5] = 0;
    p[0x3F4] = 0;
    p[0x3F3] = 0;
    p[0x3F2] = 0;
    *(s16 *)(p + 0x3EE) = 0;
    *(s16 *)(p + 0x3F0) = 0;
    *(s16 *)(p + 0x3E8) = 0;
    p[0x39C] = 0;
    *(s32 *)(p + 0x394) = 0;
    *(s16 *)(p + 0x398) = 0;
    *(s32 *)(p + 0x3C4) = 0;
    *(s16 *)(p + 0x3C2) = 0;
    p[0x3C1] = 0;
    p[0x3C0] = 0;
    p[0x440] = 0;
    *(s32 *)(p + 0x430) = 0;
    *(s32 *)(p + 0x3E4) = -1;
}
