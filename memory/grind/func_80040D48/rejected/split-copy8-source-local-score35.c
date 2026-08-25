typedef void (*FuncPtr_40D48)(s16 *, s16 *);
typedef struct { s32 a, b, c, d, e, f, g, h; } Copy8_40D48;
extern s32 D_800A9A10[];
extern s32 D_80094CFC[];
extern s32 D_800A3820;
extern FuncPtr_40D48 D_800F66A0[];
extern void func_800417D0(s32 *);
extern void func_800400B0(s32 *, s32);
extern void func_8003F62C(s32 *);
extern void func_800420E8(s32, s32);
void func_80040D48(s32 a0, s32 a1, s32 *a2, s16 *a3, s16 *arg4, s32 arg5) {
    u8 *s4;
    u8 *s5;
    u8 *s3;
    u8 *s2;
    s32 s0;
    s16 *s1;
    s32 ent;

    ent = D_800A9A10[a0];
    if (ent == 0) {
        return;
    }
    s4 = (u8 *)ent;

    *(s16 *)(s4 + 0x3C) = a3[0];
    *(s16 *)(s4 + 0x3E) = a3[1];
    *(s16 *)(s4 + 0x40) = a3[2];

    *(s32 *)(s4 + 0x78) = a2[0];
    s5 = s4 + 0x2C;
    *(s32 *)(s4 + 0x7C) = a2[1];
    s3 = s4 + 0x94;
    *(s32 *)(s4 + 0x80) = a2[2];

    s2 = s4 + 0x7E4;

    switch (a1) {
    case 0: {
        s32 *tbl;
        u8 *p;
        FuncPtr_40D48 *s0_fn;
        s0 = 1;
        tbl = D_80094CFC;
        s1 = arg4;

        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x50) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;

        do {
            s32 idx;
            u8 *a4p;
            a4p = s3 + s0 * 0x68;
            idx = *tbl;
            *(s16 *)(a4p + 0x10) = *(u16 *)((u8 *)s1 + idx * 6);
            idx = *tbl;
            *(s16 *)(a4p + 0x12) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 2);
            idx = *tbl;
            *(s16 *)(a4p + 0x14) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 4);
            s0++;
            tbl++;
        } while (s0 < 0x12);

        s0 = 0x11;
        p = s3 + 0x6E8;
        do {
            *(s16 *)(p + 6) = 0;
            s0--;
            p -= 0x68;
        } while (s0 >= 0);

        *(s32 *)(s2 + 0x4C) = *(s16 *)((u8 *)s1 + 0x6C);
        *(s32 *)(s2 + 0x50) = -(s32)*(s16 *)((u8 *)s1 + 0x6E);
        *(s32 *)(s2 + 0x54) = -(s32)*(s16 *)((u8 *)s1 + 0x70);
        *(s16 *)(s2 + 0x10) = *(u16 *)((u8 *)s1 + 0x72);
        s0_fn = D_800F66A0;
        *(s16 *)(s2 + 0x12) = -(s16)*(u16 *)((u8 *)s1 + 0x74);
        *(s16 *)(s2 + 0x14) = -(s16)*(u16 *)((u8 *)s1 + 0x76);

        (*s0_fn)((s16 *)(s2 + 0x10), (s16 *)(s2 + 0x38));

        *(s32 *)(s2 + 0xB4) = *(s16 *)((u8 *)s1 + 0x78);
        *(s32 *)(s2 + 0xB8) = -(s32)*(s16 *)((u8 *)s1 + 0x7A);
        *(s32 *)(s2 + 0xBC) = -(s32)*(s16 *)((u8 *)s1 + 0x7C);
        *(s16 *)(s2 + 0x78) = *(u16 *)((u8 *)s1 + 0x7E);
        *(s16 *)(s2 + 0x7A) = -(s16)*(u16 *)((u8 *)s1 + 0x80);
        *(s16 *)(s2 + 0x7C) = -(s16)*(u16 *)((u8 *)s1 + 0x82);

        (*s0_fn)((s16 *)(s2 + 0x78), (s16 *)(s2 + 0xA0));
        break;
    }
    case 1:
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;
        *(s16 *)(s3 + 0x06) = 0;
        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        break;
    case 2: break;
    case 3: break;
    case 4: break;
    case 5: break;
    case 6: break;
    }

    {
        s32 scaled;
        s32 *s1p;
        scaled = (*(s32 *)(s3 + 0x50) * *(s16 *)(s4 + 0x12)) >> 12;
        s0 = 0;
        s1p = (s32 *)s3;
        *(s32 *)(s3 + 0x50) = scaled;
        *(s16 *)(s5 + 6) = 0;
        do {
            func_800417D0(s1p);
            s0++;
            s1p = (s32 *)((u8 *)s1p + 0x68);
        } while (s0 < 0x12);
    }

    s0 = 1;
    *s3 = 0xA;
    *(s32 *)(s3 + 0x58) = (s32)(s4 + 0x18F4);
    {
        s32 *list;
        u8 *a4p;
        list = (s32 *)D_800A3820;
        a4p = s3 + 0x68;
        *(s16 *)(s3 + 2) = 0;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)s3;

        do {
            if (*(s16 *)(a4p + 2) >= 0) {
                s32 *list2;
                list2 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list2 + 1);
                *list2 = (s32)a4p;
            }
            s0++;
            a4p += 0x68;
        } while (s0 < 0x12);
    }

    {
        u8 *a2p;
        u8 *a3p;
        a2p = s4 + 0x10D4;
        a3p = s4 + 0x10EC;
        for (;;) {
            s32 *list3;
            u8 *src;
            src = *(u8 **)(a3p + 0x40);
            if (src == 0) {
                break;
            }
            *(Copy8_40D48 *)a3p = *(Copy8_40D48 *)(src + 0x18);
            list3 = (s32 *)D_800A3820;
            a3p += 0x68;
            D_800A3820 = (s32)(list3 + 1);
            *list3 = (s32)a2p;
            a2p += 0x68;
        }

        a2p = s4 + 0x8B4;
        if (*(s16 *)(s4 + 0x8B6) != -1) {
            do {
                s32 *list4;
                list4 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list4 + 1);
                *list4 = (s32)a2p;
                a2p += 0x68;
            } while (*(s16 *)(a2p + 2) != -1);
        }
    }

    func_800404A0((s16 *)(s4 + 0x8B4), arg5);
    *(s16 *)(s4 + 0x1A84) = (s16)arg5;
    func_800400B0((s32 *)s4, arg5);
    func_8003F62C((s32 *)s4);
    func_800420E8(a0, (s32)(s3 + 0x2C));
}
