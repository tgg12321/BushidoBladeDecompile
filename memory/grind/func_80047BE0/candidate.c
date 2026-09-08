

extern void RotTransPers3(SVECTOR *, SVECTOR *, SVECTOR *, s32 *, s32 *, s32 *, s32 *, s32 *);
extern void ReadSZfifo3(s32 *, s32 *, s32 *);
extern s16 *func_8004BCC0(s32, s16 *, s16 *, s32);
extern SVECTOR D_800EF0D8[17];
extern SVECTOR D_800EF168[17];
void func_80047BE0(void) {
    s32 sxy0, sxy1, sxy2, pflag, flag;
    s32 sz0, sz1, sz2;
    s16 *s7val;
    s32 i, j;
    s32 *src;
    SVECTOR *base;
    SVECTOR *v;
    s32 *dst32;
    s16 *dst16;
    s16 z;

    s7val = D_800A33D0;
    i = 0;
    src = D_800EF59C;
    while (i < 9) {
        if (i & 1) {
            base = D_800EF168;
            dst32 = (s32 *)0x1F800068;
            dst16 = (s16 *)0x1F800134;
        } else {
            base = D_800EF0D8;
            dst32 = (s32 *)0x1F800020;
            dst16 = (s16 *)0x1F800110;
        }
        v = base;
        j = 0;
        z = -0x2EE0;
        while (j < 17) {
            v->vx = src[j] - 0xFA0;
            v->vy = 0;
            v->vz = z;
            v++;
            z += 0x7D0;
            j++;
        }
        v = base;
        j = 0;
        while (j < 6) {
            RotTransPers3(&v[0], &v[1], &v[2], &sxy0, &sxy1, &sxy2, &pflag, &flag);
            v += 3;
            ReadSZfifo3(&sz0, &sz1, &sz2);
            *dst32++ = sxy0;
            *dst32++ = sxy1;
            *dst32++ = sxy2;
            *dst16++ = sz0;
            *dst16++ = sz1;
            *dst16++ = sz2;
            j++;
        }
        if (i != 0) {
            s7val = func_8004BCC0(0x10, (s16 *)base, s7val, 0);
        }
        i++;
        src += 17;
    }
}
