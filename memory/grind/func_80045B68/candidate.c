extern s16 D_800993FC[];
extern void func_800480C0(s32, s32, s16, s16, s16, s16);
typedef void (*F433E4)(s32, s32, s16, s16, s16, s16);
extern s32 func_80044378(s32, s32 *, s16 *);
extern s32 func_8004428C(s32 *, s16 *);
extern void func_80044010(s32, s32);
extern s32 func_80049C24(s32, s32);
extern void func_80044F50(s32, s32, s32);
extern s32 *func_800455AC(s32);
extern void func_80045230(s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, s32);
extern void func_80046048(s32, s32);

void func_80045B68(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 sp18[120];
    s16 sp108[32];
    s32 *p;
    s32 *hdr;
    s32 last;
    s32 prev;
    s32 dl;
    s32 n;
    s32 i;
    s32 y;
    s16 *q;
    s16 *r;
    u16 t;
    s32 *dst;

    p = func_800455AC(6);
    if (arg3 != 0) {
        hdr = (s32 *)arg3;
    } else {
        hdr = p;
    }
    func_80044F50(arg0, arg1, (s32)hdr);

    last = (s32)hdr + (((u32)hdr[1] >> 2) << 2);
    arg1 = hdr[0];
    dl = (s32)hdr + (((u32)hdr[arg1] >> 2) << 2);
    if (arg1 >= 3) {
        prev = (s32)hdr + (((u32)hdr[arg1 - 1] >> 2) << 2);
    } else {
        prev = 0;
    }

    for (i = 31; i >= 0; i--) {
        sp108[i] = -1;
    }

    q = arg2;
    t = *q++;
    i = 0;
    while ((s16)t != -2) {
        if ((s16)t >= 0) {
            sp108[D_800993FC[i]] = 1;
        }
        i++;
        t = *q++;
    }

    n = 0;
    for (i = 0; i < 32; i++) {
        if (sp108[i] >= 0) {
            switch (n) {
            case 0:
                func_800480C0(dl, i, 0, 0, -0x180, 0xF0);
                break;
            case 1:
                func_800480C0(dl, i, 0, 0x40, -0x180, 0xF1);
                break;
            case 2:
                func_800480C0(dl, i, 0x80, 0, -0x160, 0xF0);
                break;
            case 3:
                func_800480C0(dl, i, 0x80, 0x40, -0x160, 0xF1);
                break;
            }
            sp108[i] = n;
            n++;
        }
    }

    DrawSync(0);

    q = sp18;
    if (arg0 == 0) {
        r = arg2;
    } else {
        r = arg2 + 0x33;
    }
    while (*r != -2) {
        if (*r >= 0) {
            *q++ = 1;
            *q++ = 1;
        } else {
            *q++ = -1;
            *q++ = -1;
        }
        r++;
    }
    *q = -2;

    if (arg3 != 0) {
        dst = (s32 *)((s32)p + 4);
        dl = func_80044378(last, dst, sp18);
        func_80044010((s32)dst, 6);
    } else {
        dl = func_8004428C((s32 *)last, sp18);
        func_80044010(last, 6);
    }
    func_80045230(dl);

    q = sp18;
    y = 0;
    i = 0;
    while (*q != -2) {
        if (*q >= 0) {
            switch (sp108[D_800993FC[i]]) {
            case 0:
                ((F433E4)func_800433E4)(6, y, 0, 0, -0x180, 0xE8);
                break;
            case 1:
                ((F433E4)func_800433E4)(6, y, 0, 0x40, -0x180, 0xE9);
                break;
            case 2:
                ((F433E4)func_800433E4)(6, y, 0x80, 0, -0x160, 0xE8);
                break;
            case 3:
                ((F433E4)func_800433E4)(6, y, 0x80, 0x40, -0x160, 0xE9);
                break;
            }
            y += 2;
        }
        q += 2;
        i++;
    }

    if (prev != 0) {
        *p = dl - (s32)p;
        dl = func_80049C24(prev, dl);
        func_80045230(dl);
    } else {
        *p = 0;
    }
    func_80045600(6, dl);
    func_80045694(6, (s32)&func_80046048);
}
