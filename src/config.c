#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern s32 func_80046798(void);
extern void func_80017F98(s32, s32, s32);
extern void *func_8004153C(void);
extern void func_80017714(void);
extern void func_80017F90(void);
extern void func_80017E8C(s32);

/* Externs for globals */
extern s32 D_800A336C;
extern s32 D_800A322C;
extern s32 D_800A3374;
extern s32 D_800A3370;
extern s32 D_800948BC;
extern s16 D_800F665C;
extern void func_8001924C(s32 *, s32);
extern void func_80045A28(s32, s32);
extern void func_80052A20(s32 *, s32 *, s16 *);
extern void func_80052C10(void *);

/* Externs for globals */
extern u8 D_800A6690;
extern s16 D_800F6656;
extern s16 D_800F6658;
extern u8 D_800A8FB0[];
extern s32 D_800A93B0;
extern s32 D_800A93B4;
extern s32 D_800A93B8;
extern s32 D_800A93BC;
extern s32 D_800A93C0;
extern s32 D_800A93C4;
extern s32 D_800A3708;
extern s32 D_80094A6C;
extern s32 func_8003F268(void);
extern s32 func_8003FA24(void *, s16 *, s32);
extern void func_8003FECC(s32 *, s32 *, s16 *);
extern void func_80040068(u8 *);
extern void func_8004001C(u8 *);
extern s32 func_80017738(s32, s32);
extern s32 func_80017D84(void *);
extern void func_80045230(s32);
extern s32 *D_80103608[];
extern s16 D_80094AEC[];
extern char D_80010D8C[];

/* --- Functions 0x8003F168 - 0x8004019C --- */
void func_8003F168(void) {
    s32 v0 = *(s32 *)((u32)&D_800948BC + (func_80046798() << 3));
    if (v0) {
        (*(void (**)(void))((u32)&D_800948BC + (func_80046798() << 3)))();
    }
}
s32 func_8003F1C8(void) {
    return D_800A336C;
}

void *func_8003F1D4(void) {
    return &D_800A6690;
}

void func_8003F1E4(s32 a0) {
    if (a0) {
        D_800F6656 = 3;
        D_800F6658 = 2;
    } else {
        D_800F6656 = 0;
        D_800F6658 = 1;
    }
}

void func_8003F218(s32 a0) {
    if ((u32)a0 >= 2) {
        return;
    }
    if (a0 == D_800A322C) {
        return;
    }
    D_800A322C = a0;
    if (!a0) {
        func_8003F1E4(0);
    }
    D_800F665C = (s16)D_800A322C;
}
s32 func_8003F268(void) {
    return D_800A322C;
}
void func_8003F274(void) {
    s32 *var_v0;
    s32 var_a2;
    s32 t2;
    s32 t5;
    s32 *t1;
    s32 a1val;
    s32 a2val;

    var_v0 = (s32 *)&D_800A8FB0;
    var_a2 = 0xFF;
    do {
        *var_v0 = 0;
        var_a2 -= 1;
        var_v0++;
    } while (var_a2 >= 0);
    func_8003F268();
    {
        void *p = *(void **)&D_800A3708;
        a1val = *(s32 *)((u8 *)p + 0x4C) + 0x7D00;
        a2val = *(s32 *)((u8 *)p + 0x54) + 0x7D00;
    }
    t2 = a1val / 2000;
    t5 = a2val / 2000;
    var_a2 = 0;
    t1 = &D_80094A6C;
    do {
        s32 row = t5 + (var_a2 - 8);
        if ((u32)row < 0x20U) {
            s32 t0 = *t1;
            s32 var_a1 = 0;
            s32 col_base = -8;
            do {
                s32 col = t2 + col_base;
                if ((u32)col < 0x20U) {
                    u8 *cell = (u8 *)&D_800A8FB0 + (row << 5) + col;
                    *cell |= (t0 >> ((0xF - var_a1) * 2)) & 3;
                }
                var_a1++;
                col_base = var_a1 - 8;
            } while (var_a1 < 0x10);
        }
        var_a2++;
        t1++;
    } while (var_a2 < 0x10);
}
void func_8003F388(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        D_800A8FB0[y * 32 + x] |= 0x4;
    }
}
void func_8003F3D4(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        D_800A8FB0[y * 32 + x] |= 0x8;
    }
}
void func_8003F420(s32 a0, s32 a1) {
    s32 s3, s2, s1;
    int new_var;
    s32 s0;
    a0 += 0x7D00;
    a1 += 0x7D00;
    s3 = a0 / 2000;
    s1 = a0 - s3 * 2000;
    s0 = a1 / 2000;
    s2 = s0;
    s0 = a1 - s2 * 2000;
    if (s1 < 1000) {
        s1 = -1;
    } else {
        s1 = 1;
    }
    if (s0 < 1000) {
        s0 = (new_var = -1);
    } else {
        s0 = 1;
    }
    func_8003F52C(s3, s2, 2);
    func_8003F52C(s3 + s1, s2, 2);
    func_8003F52C(s3, s2 + s0, 2);
    func_8003F52C(s3 + s1, s2 + s0, 2);
}

void func_8003F52C(s32 a0, s32 a1, s32 a2) {
    D_800A8FB0[a1 * 32 + a0] = a2 & 3;
}

u32 func_8003F54C(s32 a0, s32 a1) {
    return D_800A8FB0[a1 * 32 + a0];
}

void func_8003F568(void) {
    D_800A3374 = 0;
    D_800A3370 = 0;
    D_800A93B8 = 0;
    D_800A93B4 = 0;
    D_800A93B0 = 0;
    D_800A93C4 = 0;
    D_800A93C0 = 0;
    D_800A93BC = 0;
}

void func_8003F5A8(s32 a0, s32 a1, s32 a2) {
    (&D_800A93B0)[a2] = a0;
    (&D_800A93BC)[a2] = a1;
}

void func_8003F5CC(void) {
    func_80017F98(D_800A93B0, D_800A93BC, 0);
    func_80017F98(D_800A93B4, D_800A93C0, 1);
    func_80017F98(D_800A93B8, D_800A93C4, 2);
}

void func_8003F62C(s32 *a0) {
    s16 *s0;
    s32 *s1 = a0;
    s0 = (s16 *)s1[9];
    if (s0 == 0) return;
    if (s0[3]) {
        func_8004016C(*(s16 *)((u8 *)s1 + 4));
        func_8003F824(s1, 0);
    }
    if (s0[1]) {
        func_8004001C((u8 *)s0);
    }
    func_8003F6D8(s0);
    func_8001924C((s32 *)((u8 *)s0 + 0x418), s0[0]);
    if (s0[1]) {
        func_80040068((u8 *)s0);
        s0[1] = 0;
    }
}
void func_8003F6D8(s16 *a0) {
    s16 *sp10;
    s32 sp28;
    s32 var_fp;
    s32 var_s7;
    s32 var_s5;
    s32 var_s6;
    s32 *var_s2;
    s32 *var_s3;
    s32 *var_s4;
    s32 *temp_v0;

    sp10 = a0;
    var_fp = 0;
    if (*a0 > 0) {
        sp28 = 8;
        do {
            var_s7 = 0;
            temp_v0 = (s32 *)((u8 *)sp10 + sp28);
            var_s4 = (s32 *)((u8 *)temp_v0 + 0x1C);
            if (*(s32 *)((u8 *)temp_v0 + 0x1C) > 0) {
                var_s6 = 0x18;
                var_s5 = 0x84;
                var_s3 = var_s4;
                var_s2 = (s32 *)((u8 *)var_s4 + 0x84);
                do {
                    s32 *temp_s1 = (s32 *)((u8 *)var_s4 + var_s6);
                    s32 temp_s0 = *(s32 *)((u8 *)var_s3 + 4);
                    var_s6 += 0x20;
                    var_s5 += 0x10;
                    temp_s0 += 0x18;
                    func_80052A20((void *)temp_s0, var_s2, temp_s1);
                    func_80052A20((void *)temp_s0, (s32 *)((u8 *)var_s2 + 8), (s32 *)((u8 *)temp_s1 + 0x10));
                    var_s3 = (s32 *)((u8 *)var_s3 + 4);
                    var_s7++;
                    var_s2 = (s32 *)((u8 *)var_s4 + var_s5);
                } while (var_s7 < *(s32 *)((u8 *)temp_v0 + 0x1C));
            }
            sp28 += 0xD0;
            var_fp++;
        } while (var_fp < *sp10);
    }
}
void func_8003F7F4(void) {
    func_80017714();
    func_80017F90();
    D_800A3370 = 0;
    D_800A3374 = 0;
}
void func_8003F824(s32 *a0, s32 a1) {
    s16 *s3;
    s16 *s1;
    s16 v1;
    s32 s5;
    s32 s7;
    u16 a0val;
    s32 s0;
    s32 *s2;
    s32 *temp_v1;
    s8 *temp_s0;

    s3 = (s16 *)a0[9];
    if (s3 == NULL) {
        return;
    }
    s1 = (s16 *)a0[10];
    s0 = (s32)s3 + 0x468;
    if (*s1 == -3) {
        a0[9] = 0;
        return;
    }
    s3[0] = 0;
    s3[1] = 0;
    s3[2] = 0;
    s3[3] = 0;
    a0val = (u16)*s1;
    s5 = 0;
    s7 = 0;
    if (*s1 != -3) {
        do {
            if ((s16)a0val != -2) {
                if (s3[0] >= 5) {
                    func_80052C10((void *)s1);
                }
                s2 = (s32 *)((u8 *)s3 + (s32)(s3[0] * 0xD0 + 8));
                temp_s0 = *(s8 **)((u8 *)a0 + (s5 * 4) + 0x1A34);
                *(s32 *)((u8 *)s2 + 0x18) = s0;
                *(s32 *)((u8 *)s2 + 0x14) = (s32)temp_s0;
                func_8003FA24(s2, s1, s0);
                temp_v1 = (s32 *)((u8 *)s3 + (s32)(s3[0]) * 0x10);
                *(s32 *)((u8 *)temp_v1 + 0x418) = s2[1];
                *(s32 *)((u8 *)temp_v1 + 0x41C) = s2[2];
                *(s32 *)((u8 *)temp_v1 + 0x420) = s2[3];
                *(s32 *)((u8 *)temp_v1 + 0x424) = s2[4];
                *temp_s0 = 0xD;
                s3[0] = (s16)((u16)s3[0] + 1);
                v1 = *s1;
                s0 = (s32)func_8003FA24;
                if (v1 >= 0) {
                    s1 += 2;
                    do {
                        s1 += 2;
                    } while (*s1 >= 0);
                    s1 -= 2;
                    v1 = *s1;
                }
                s7 = 1;
                if (v1 == -1) {
                    s1 += 2;
                    func_8003FECC(a0, s2, s1);
                }
            }
            s1 += 2;
            if (*s1 != -2) {
                do {
                    s1 += 2;
                } while (*s1 != -2);
            }
            a0val = (u16)*s1;
            s5++;
        } while (*s1 != -3);
    }
    if (a1 != 0) {
        func_80045A28((s32)a0[1], s0 - a0[7]);
    }
}
s32 func_8003FA24(s32 *a0, s16 *a1, s32 a2) {
    u16 sp10;
    s16 sp12;
    s16 *sp14;
    s32 sp18;
    s32 sp1C;
    s16 *sp20;
    u16 *s0;
    s16 *s1;
    void *s3;
    s16 *s4;
    s32 var_a1;
    s32 var_a3;
    s32 var_v0;
    u16 var_a3_u;
    u16 var_a2_u;
    s32 temp_a1;
    s32 var_a3_inner;
    s16 *var_a0;
    s32 temp_v1;
    s32 neg1;

    s1 = a1;
    s3 = *(void **)((u8 *)a0 + 0x14);
    {
        s16 idx1 = *(s16 *)((u8 *)s3 + 0x4);
        s16 idx2 = *(s16 *)((u8 *)s3 + 0x2);
        s32 *row = D_80103608[idx1];
        s0 = (u16 *)(row[idx2]);
    }
    var_a3 = (s16)(*s0 - 1);
    sp10 = *s0;
    s0 += 2;
    sp14 = s1;
    if (var_a3 != -1) {
        s16 *a1ptr = s1;
        s16 *a0ptr = s1 + 4;
        do {
            *a1ptr = (s16)*s0;
            s0++;
            var_a3--;
            *(s16 *)((u8 *)a0ptr - 2) = (s16)*s0;
            s0++;
            *(s16 *)a0ptr = (s16)*s0;
            s0++;
            a1ptr += 8;
            a0ptr += 8;
        } while (var_a3 != -1);
    }
    if ((u32)(s32)s0 & 3) {
        s0++;
    }
    var_a1 = 0;
    s4 = (s16 *)s0;
    var_a3_u = *(u16 *)s0;
    s0++;
    if ((s32)(var_a3_u << 16) != 0) {
        s16 *a0tab = (s16 *)&D_80094AEC;
        do {
            var_a2_u = *(u16 *)s0;
            s0++;
            if (var_a2_u & 8) {
                var_v0 = (s16)var_a3_u * 6;
            } else {
                var_v0 = (s16)var_a3_u * 3;
            }
            var_a1 += var_v0;
            {
                s32 tidx = ((u32)var_a2_u >> 2) & 6;
                s32 stride = a0tab[tidx] * (s16)var_a3_u;
                s0 += stride;
            }
            var_a3_u = *(u16 *)s0;
            s0++;
        } while ((s32)(var_a3_u << 16) != 0);
    }
    sp20 = s1;
    s1 = (s16 *)((u8 *)s1 + func_80017738((s16)sp10, var_a1));
    if ((u32)(s32)s1 & 3) {
        s1 += 2;
    }
    var_a0 = (s16 *)0x1F800000;
    sp18 = 0x1F800000;
    {
        s32 outer = (s16)(*(s16 *)s4);
        s16 *s0b = s4 + 2;
        var_a3 = outer;
        if (outer != 0) {
            neg1 = -1;
            do {
                s16 *cur = s0b + 2;
                temp_v1 = (s32)(*(u16 *)s0b << 16) >> 19;
                temp_a1 = temp_v1 & 3;
                if (temp_v1 & 1) {
                    s32 cnt = var_a3 - 1;
                    var_a3_inner = cnt;
                    if (cnt != neg1) {
                        s16 *a0tab = (s16 *)&D_80094AEC;
                        do {
                            s32 merged;
                            if (temp_v1 & 2) {
                                s32 hi = *(u16 *)((u8 *)cur + 0x12);
                                s32 lo = *(u16 *)((u8 *)cur + 0x10);
                                merged = (hi << 16) | lo;
                            } else {
                                s32 hi = *(u16 *)((u8 *)cur + 0x10);
                                s32 lo = *(u16 *)((u8 *)cur + 0xE);
                                merged = (hi << 16) | lo;
                            }
                            var_a0[0] = 4;
                            var_a0[1] = 2;
                            var_a0[2] = (s16)(merged & 0xFF);
                            var_a0[3] = (s16)((merged >> 8) & 0xFF);
                            var_a0[4] = (s16)((merged >> 16) & 0xFF);
                            var_a0[5] = (s16)((u32)merged >> 24);
                            var_a0 += 6;
                            var_a3_inner--;
                            {
                                s32 stride = a0tab[temp_a1 * 2] * 2;
                                cur = (s16 *)((u8 *)cur + stride);
                            }
                        } while (var_a3_inner != neg1);
                        var_a0[0] = 0;
                        var_a0++;
                    } else {
                        goto end_outer;
                    }
                } else {
                    s32 cnt = var_a3 - 1;
                    var_a3_inner = cnt;
                    if (cnt != neg1) {
                        s16 *a0tab = (s16 *)&D_80094AEC;
                        do {
                            s32 hi = *(u16 *)((u8 *)cur + 0xE);
                            s32 lo = *(u16 *)((u8 *)cur + 0xC);
                            s32 merged = (hi << 16) | lo;
                            var_a0[0] = 3;
                            var_a0[0] = 2;
                            var_a0 += 1;
                            var_a0[0] = (s16)(merged & 0xFF);
                            var_a0 += 1;
                            var_a0[0] = (s16)((merged >> 8) & 0xFF);
                            var_a0[0] = (s16)((merged >> 16) & 0xFF);
                            var_a0 += 2;
                            var_a0++;
                            var_a3_inner--;
                            {
                                s32 stride = a0tab[temp_a1 * 2] * 2;
                                cur = (s16 *)((u8 *)cur + stride);
                            }
                        } while (var_a3_inner != neg1);
                    }
end_outer:
                    var_a0[0] = 0;
                    var_a0++;
                }
                outer = *(s16 *)cur;
                s0b = cur + 2;
                var_a3 = outer;
            } while (outer != 0);
        }
    }
    {
        s16 *sv1 = s1;
        if ((u32)(s32)s1 & 3) {
            sv1 = s1 + 2;
        }
        s1 = sv1;
    }
    func_80045230((s32)s1);
    if (*(u16 *)s0 != 0) {
        func_80052C10((void *)&D_80010D8C);
    }
    func_8003FE40(sp14, (s16)sp10, a1);
    {
        s16 *s3p18 = (s16 *)((u8 *)s3 + 0x18);
        sp1C = (s32)s3p18;
        sp12 = 0xE00;
        *(u16 *)a0 = (u16)func_80017D84(&sp10);
        *(s32 *)((u8 *)s3 + 0x60) = (s32)sp20;
        a0[7] = 0;
        a0[12] = 0;
        {
            s16 *sa0 = s1;
            if ((u32)(s32)s1 & 3) {
                sa0 = s1 + 2;
            }
            s1 = sa0;
        }
        a0[2] = (s32)s3p18;
        *(u16 *)((u8 *)a0 + 0x4) = *(u16 *)a0;
        *(s32 *)((u8 *)a0 + 0x10) = (s32)((u8 *)a0 + 0x2C);
        *((u8 *)a0 + 0x6) = 0;
        *((u8 *)a0 + 0x7) = 0;
        *(s32 *)((u8 *)a0 + 0xC) = *(s32 *)((u8 *)s3 + 0x60);
    }
    return (s32)s1;
}
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2) {
    register s32 i asm("a3");
    i = 0;
    if (a1 > i) {
        register s16 t0 asm("t0") = -256;
        register s16 *v1 asm("v1") = a0;
        for (i = 0; i < a1; i++) {
            *(s16 *)((u8 *)v1 + 6) = t0;
            v1 = (s16 *)((u8 *)v1 + 8);
        }
    }

    {
        register s32 count asm("v1");
        count = a2[0];
        a2++;
        if (count >= 0) {
            register s32 neg1 asm("t1") = -1;
            do {
                register int val asm("a1");
                val = a2[0];
                do { count--; } while(0);
                a2++;
                if (count != neg1) {
                    register s32 neg1b asm("t0") = -1;
                    do {
                        register s32 addr asm("v0");
                        i = a2[0];
                        a2++;
                        count--;
                        addr = (i << 3) + (s32)a0;
                        *(s16 *)(addr + 6) = val;
                    } while (count != neg1b);
                }
                count = a2[0];
                a2++;
            } while (count >= 0);
        }
    }
    return (s16 *)a2;
}

void func_8003FECC(s32 *a0, s32 *a1, s16 *a2) {
    register s16 v1 asm("v1");
    register s32 t2 asm("t2");
    register u16 t0 asm("t0");
    register s32 *a3 asm("a3");
    register s32 *t1 asm("t1");

    t2 = *(s32 *)((u8 *)a1 + 0x1C);
    t0 = (u16)a2[0];
    a1 = (s32 *)((u8 *)a1 + 0x1C);
    if ((s16)t0 == -2) {
        goto end;
    }
    a3 = (s32 *)((u8 *)a1 + t2 * 0x10);
    t1 = (s32 *)((u8 *)a1 + t2 * 4);
    do {
        a2++;
        t1[1] = (s32)((u8 *)a0 + (s32)((s16)t0 * 0x68 + 0x94));
        t2++;
        *(u16 *)((u8 *)a3 + 0x84) = (u16)a2[0];
        a2++;
        *(u16 *)((u8 *)a3 + 0x86) = (u16)a2[0];
        a2++;
        *(u16 *)((u8 *)a3 + 0x88) = (u16)a2[0];
        a2++;
        *(u16 *)((u8 *)a3 + 0x8C) = (u16)a2[0];
        a2++;
        *(u16 *)((u8 *)a3 + 0x8E) = (u16)a2[0];
        a2++;
        *(u16 *)((u8 *)a3 + 0x90) = (u16)a2[0];
        a2++;
        a3 = (s32 *)((u8 *)a3 + 0x10);
        *(s32 *)((u8 *)t1 + 0x78) = (s32)a2[0];
        v1 = (s16)a2[0];
        t0 = (u16)a2[0];
        t1 = (s32 *)((u8 *)t1 + 4);
    } while (v1 != (s16)-2);
end:
    a1[0] = t2;
    a1[5] = t2;
}
s32 func_8003FFA8(s32 a0) {
    if (a0 & 3) {
        a0 = (a0 + 3) & ~3;
    }
    return a0;
}
void func_8003FFC4(s32 *a0) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        v1[3] = 1;
    }
}
void func_8003FFE0(void) {
    s32 *v0 = (s32 *)func_8004153C();
    if (v0) {
        s16 *v1 = (s16 *)v0[9];
        if (v1) {
            v1[1] = 1;
        }
    }
}
void func_8004001C(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 1;
        a0[0xE + i * 0xD0] = 1;
    }
}
void func_80040068(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 0;
        a0[0xE + i * 0xD0] = 0;
    }
}
void func_800400B0(s32 *a0, s32 a1) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        s32 i;
        for (i = 0; i < v1[0]; i++) {
            *(s32 *)((u8 *)v1 + i * 0xD0 + 0x34) = a1;
        }
    }
}
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            s0 = 0;
            do {
                func_80017E8C(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}

void func_8004016C(void) {
    void *v0 = func_8004153C();
    if (v0) {
        func_800400F8(v0);
    }
}

void func_8004019C(s32 *a0, s32 a1) {
    s32 *v1 = (s32 *)a0[9];
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0[9] = (s32)v1;
        a0[10] = a0[10] + a1;
        *(s16 *)((s32)v1 + 6) = 1;
    }
}
