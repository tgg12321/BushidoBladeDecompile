/* REJECTED — per-block temporaries (va/vb/vc, ra/rb/rc, ca/cb/cc) instead of
 * three reused locals.  Honest sandbox 30 at 50 insns vs the floor-18 form's
 * 18 at 51.  This was the FIRST item on s3's "untried" list for frontier F1
 * (split the three blocks' temporaries into per-block locals); it is now
 * measured DEAD and 12 points worse.  Mechanism: nine distinct pseudos give
 * GCC's allocator nine independent live ranges instead of three reused ones,
 * so the flag byte no longer stays in one register across the join labels and
 * every block pays its own address materialisation. */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 va;
    s32 vb;
    s32 vc;
    u8 ra;
    u8 rb;
    u8 rc;
    s32 ca;
    s32 cb;
    s32 cc;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    ca = p[8] & 1;
    va = D_80106A73;
    ra = va | 1;
    if (!ca) {
        ra = va;
    }
    D_80106A73 = ra;

    cb = p[8] & 2;
    vb = D_80106A73;
    rb = vb | 2;
    if (!cb) {
        rb = vb;
    }
    D_80106A73 = rb;

    cc = p[8] & 4;
    vc = D_80106A73;
    rc = vc | 4;
    if (!cc) {
        rc = vc;
    }
    D_80106A73 = rc;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
