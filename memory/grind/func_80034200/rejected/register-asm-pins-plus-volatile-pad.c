void func_80034200(void) {
    register s32 a2 asm("a2");
    register s32 a3 asm("a3");
    register s32 a0 asm("a0");
    register u8 *a1 asm("a1");
    register u8 *v1 asm("v1");
    register u8 *t0 asm("t0");
    register s32 t1 asm("t1");
    register s32 t3 asm("t3");
    register s32 t2 asm("t2");
    s32 count;
    volatile char _pad[8];
    (void)_pad;

    a2 = 0;
    a3 = 0;
    count = D_800A389B;
    g_disp_enable = DISP_LOADING;
    a0 = (count & 0) + 0;
    if (count <= 0) {
        goto end;
    }
    t3 = D_800A3874;
    t2 = 3;
    a1 = &D_800F65F8;

outerloop:
    t1 = (a3 < t3);
    v1 = a1;
    t0 = a1 + 2;
innerloop:
    {
        s32 v0;
        if (t1) {
            v0 = ((s32)*v1) << a2;
        } else {
            v0 = t2 << a2;
        }
        a0 |= v0;
        v1++;
        a2 += 2;
    }
    if ((s32)v1 < (s32)t0) goto innerloop;
    {
        s32 v0;
        v0 = D_800A389B;
        a3++;
        if (a3 < v0) { a1 += 2; goto outerloop; }
    }

end:
    D_800A3784 = a0;
}
