/* s9: KILLED - a FRESH intermediate does not buy a surviving block-0 insn.
 * `b2 = base; out2 = ...; saved = b2 + 0x94;` (a copy separated from its consumer
 * by one statement, i.e. exactly the H-s8-5 shape but with two pseudos instead of
 * one) is copy-propagated by cse1 and deleted: the .lreg table is bit-identical to
 * the plain form (tbl 6/47, i 8/95, a1/a2 16/99 - no insn added).  Refined cse1
 * survival rule: a two-step definition survives into flow ONLY when the second
 * step is a SELF-update of the SAME pseudo (`x = x + k`) with an intervening
 * statement.  Fresh-intermediate copies are propagated; constant two-steps are
 * constant-folded (see rejected/i-constant-splitinit-folded-no-9th-ref.c).
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = (s32 *) (((u8 *) D_80094CFC) - 0x10);
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    s32 b2;
    b2 = base;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    saved = b2 + 0x94;
    tbl = (s32 *) (((u8 *) tbl) + 0x10);
    stptr = base;
    stptr += 0xFC;
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr2 = saved + 0x750;
    out3 = out2;
    i = 0x12;
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
    func_8004A348(buf, pa4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(pa4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
