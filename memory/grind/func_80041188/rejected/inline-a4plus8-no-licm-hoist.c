/* s7 REJECTED (score 63): no out2/out3 local at all - `a4 + 8` written inline at all four
 * call sites. PROVES GCC 2.7.2 does NOT hoist `a4 + 8` out of these goto-loops: the
 * expression materialises INSIDE loop1 (addiu s1,s8,32) and INSIDE loop2 (addiu s0,s8,32),
 * so neither `addiu $s6,$s7,0x20` (preamble) nor `addiu $s3,$s7,0x20` (between-loops) in
 * target is LICM output - both are source-level statements. Insn count stays 132 but the
 * whole callee-saved assignment scrambles. This is the measurement that motivated the
 * two-separate-locals chassis (candidate.c). */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *tbl = D_80094CFC;
    s32 i = 1;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 stptr;
    s32 stptr2;
    s32 offset;
    u16 *p;
    saved = base + 0x94;
    stptr = base + 0xFC;
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, a4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, a4 + 8);
    func_800523E0(a4, a4 + 8, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, a4 + 8);
    func_800523E0(a4, a4 + 8, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
