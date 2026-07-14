/* REJECTED (Judge regression diagnosis, closed s2 2026-07-13): this was the
 * byte-correct form committed on main before the re-audit patrol flagged it.
 * Both flagged constructs are UNNECESSARY — the natural form (candidate.c:
 * no alias, no wrap, p derived as base + i*2) measures sandbox 0.
 * Do not re-propose: (1) `new_var` pointer alias — codegen-steering, fails
 * checklist Tests 1/2/3/6; (2) unannotated do{}while(0) wrap; (3) `new_var2`
 * constant-holder for the literal 1. All three are inert given the derived-p
 * structure, so no FAKE annotation is needed at all. */
s16 *func_8003D7B4(s32 arg0) {
    s32 i = 0;
    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);
    s32 new_var2;
    u8 *new_var;
    u8 *p = base;
    do {
        s32 nbits;
        s16 val;
        s32 sign_bit;
        s32 sval;
        do {
            nbits = func_8003D888((s32 *)base, 4);
            if (nbits == 0) {
                nbits = 16;
            }
            new_var = p;
            val = (s16)func_8003D888((s32 *)base, nbits);
        } while (0);
        sval = val;
        sign_bit = nbits - 1;
        new_var2 = 1;
        if ((sval >> sign_bit) & new_var2) {
            val = val | (0xFFFF << sign_bit);
        }
        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(new_var + 0xC) + val);
        i++;
        p += 2;
    } while (i < 6);
    return (s16 *)(base + 0xC);
}
