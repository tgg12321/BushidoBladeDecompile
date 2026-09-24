extern u8 *D_800A3708;
extern s32 D_800A336C;
extern s32 D_800A322C;
extern s16 D_800F6656;
extern s32 D_800927C0[64][32];
extern s32 D_80090740[64][32];
extern s32 D_80094840[];
extern s32 D_800A7EF0[];
extern s16 D_80101E02;
extern s16 D_80101E04;
typedef struct { void (*init)(void); void (*update)(void); } StageFuncs;
extern StageFuncs g_stage_init_tbl[];
extern s32 *func_8003EB84(s32, s32, s32 *);
extern s16 *camera_CalcAngles(void);
extern void func_80042A88(u8 *, s32 *);
extern s32 ratan2(s32, s32);
extern s32 stage_GetId(void);
extern void func_800620B8(s16 *, s16 *);
void func_8003E6D8(s32 arg0) {
    s32 mat[8];
    s16 vec[4];
    s32 res[3];
    s16 pos[4];
    s32 cx;
    s32 cz;
    s16 x;
    s16 z;
    s32 *mask;
    s32 *out;
    s32 i;
    s32 j;
    s32 bits;
    s16 row;
    s16 col;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    u8 *e;
    u8 *e2;
    s32 *list;
    s32 ang;
    s32 idx;

    cx = (*(s32 *)(D_800A3708 + 0x4C) + 0x7D00) / 2000;
    cz = (*(s32 *)(D_800A3708 + 0x54) + 0x7D00) / 2000;
    camera_CalcAngles();
    func_80042A88(D_800A3708 + 0x10, mat);
    vec[2] = 0x1000;
    vec[0] = 0;
    vec[1] = 0;
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $0, 0($12)\n"
        "lwc2   $1, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "memory");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(res) : "$12", "memory");
    D_800A336C = ang = ratan2(res[0], res[2]);
    x = cx - 0xF;
    z = cz - 0xF;
    idx = (ang >> 6) & 0x3F;
    if (D_800A322C != 0) {
        mask = D_800927C0[idx];
    } else if (D_800F6656 & 1) {
        mask = D_80090740[idx];
    } else {
        s32 *src2;
        s32 *dst;
        src2 = D_80094840;
        mask = D_80090740[idx];
        dst = (s32 *)0x1F800004;
        for (i = 0; i < 0x1F; i++) {
            *dst++ = *src2++ & *mask++;
        }
        mask = (s32 *)0x1F800004;
    }

    out = D_800A7EF0;
    for (i = 0; i < 0x1F; i++) {
        row = z + i;
        if (row < 0) {
            mask++;
            continue;
        }
        if (row >= 0x20) {
            break;
        }
        j = 0;
        bits = *mask++;
        if (bits == 0) {
            continue;
        }
        for (bits <<= 1; j < 0x1F; j++, bits <<= 1) {
            col = x + j;
            if (col < 0) {
                continue;
            }
            if (col >= 0x20) {
                break;
            }
            if (bits < 0) {
                vidx = D_800A7FE0[row][col];
                if (vidx >= 0) {
                    a3 = D_800A8FB0[row * 0x20 + col];
                    do {
                        t0 = D_800A87E0[vidx++];
                        v1 = t0 & 0x7FFF;
                        if (v1 < D_800A3368) {
                            e = &D_800A4750[v1 * 0x10];
                            e[6] = a3 & 3;
                            if ((a3 & 8) || ((a3 & 4) && (e[7] & 8))) {
                                e[7] |= 1;
                            } else {
                                e[7] &= 0xFE;
                            }
                            list = (s32 *)D_800A3820;
                            D_800A3820 = (s32)(list + 1);
                            *list = (s32)e;
                        } else {
                            e2 = &D_800A6690[(v1 - D_800A3368) * 0x68];
                            if (e2[0x58] == 0) {
                                *out++ = (s32)e2;
                                e2[0x58] = 1;
                            }
                        }
                    } while (!(t0 & 0x8000));
                }
            }
        }
    }

    if ((D_800F6656 & 1) && D_800A322C == 0) {
        out = func_8003EB84(x, z, out);
    }
    while (out != D_800A7EF0) {
        out--;
        *(s32 *)D_800A3820 = *out;
        (*(u8 **)D_800A3820)[0x58] = 0;
        D_800A3820 += 4;
    }
    if (g_stage_init_tbl[stage_GetId()].update != 0) {
        g_stage_init_tbl[stage_GetId()].update();
    }
    {
        s16 *p = &D_80101E00;
        pos[0] = -p[0];
        pos[1] = -D_80101E02;
        pos[2] = -D_80101E04;
        func_800620B8(pos, p + 0xE);
    }
}
