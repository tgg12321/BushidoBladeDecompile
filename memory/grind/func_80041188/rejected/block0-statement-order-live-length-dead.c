/* s7 REJECTED (score 18): out2s definition moved to the FIRST statement of the body, to
 * lengthen its reg_live_length via block-0 lead-in. Measured .lreg: live went 42 -> 43 only
 * (+1), i.e. block-0 statement POSITION is worth ~1 insn of liveness, not the ~8 a naive
 * source-order model predicts. A 7-permutation sweep of the block-0 statement order
 * (P0..P6 x {plain, out2-8}) produced exactly three score classes - 15 / 10 / 18 - with
 * dispositions identical inside each class. Block-0 statement ordering is a DEAD lever for
 * this function; the live-length lever that works is the between-loops read (candidate.c). */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4;
    s32 i;
    s32 *tbl;
    s32 base;
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    pa4 = a4;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    i = 1;
    tbl = D_80094CFC;
    base = D_800A9A10[a0];
    saved = base + 0x94;
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
    i = 0x12;
    out3 = (s32 *) (((u8 *) pa4) + 0x20);
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
