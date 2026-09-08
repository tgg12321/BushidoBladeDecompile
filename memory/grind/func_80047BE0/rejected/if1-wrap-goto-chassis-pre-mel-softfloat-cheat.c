void func_80047BE0(void) {
    s32 loc20;
    s32 loc24;
    s32 loc28;
    s32 loc2C;
    s32 loc30;
    s32 loc34;
    s32 loc38;
    s32 loc3C;
    s16 *s7val;
    s32 i;
    s32 *src;
    s16 *base;
    s32 *new_var;
    s16 *new_var2;
    s16 *dst16;
    s32 j;
    s32 *dst32;
    s16 *p;
    s16 vol;
    s32 *rp;
    s16 *wp;

    s7val = D_800A33D0;
    i = 0;
    if (1) {
        new_var = &D_800EF59C[0];
        src = new_var;
outer_loop:
        if (i & 1) {
            { base = &D_800EF168; dst32 = (s32 *)0x1F800068; dst16 = (s16 *)0x1F800134; }
        } else {
            base = &D_800EF0D8;
            dst32 = (s32 *)0x1F800020;
            dst16 = (s16 *)0x1F800110;
        }
        p = base;
        j = 0;
        vol = -0x2EE0;
        rp = src;
        wp = base + 2;
inner1:
        {
            s32 val;
            val = *rp - 0xFA0;
            rp++;
            j++;
            *p = (s16)val;
            *(wp - 1) = 0;
            *wp = vol;
            wp += 4;
            p += 4;
            vol += 0x7D0;
        }
        if (j < 0x11) goto inner1;
    }
    p = base;
    j = 0;
inner2:
    RotTransPers3(p, p + 4, p + 8, &loc20, &loc24, &loc28, &loc2C, &loc30);
    p += 12;
    ReadSZfifo3(&loc34, &loc38, &loc3C);
    j++;
    *dst32 = loc20;
    dst32++;
    *dst32 = loc24;
    dst32++;
    new_var2 = dst16;
    *dst32 = loc28;
    dst32++;
    *new_var2 = *(u16 *)&loc34;
    dst16++;
    *dst16 = *(u16 *)&loc38;
    dst16++;
    *dst16 = *(u16 *)&loc3C;
    dst16++;
    if (j < 6) goto inner2;
    if (i != 0) {
        s7val = func_8004BCC0(0x10, base, s7val, 0);
    }
    i++;
    src += 17;
    if (i < 9) goto outer_loop;
}
