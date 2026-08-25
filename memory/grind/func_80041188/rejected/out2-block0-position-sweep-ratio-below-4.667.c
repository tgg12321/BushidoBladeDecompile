/* REJECTED (s23). pa4-free goto chassis, out2's definition swept to the LAST
 * statement of block 0 (shortest reachable live length with a block-0 def).
 * Sandbox 13 at 132/132.  Measured: out2 3/41 = 731, a4 7/188 = 744,
 * a3 4/99 = 808, tbl 4/45 = 1777.
 * THE RATIO LAW: with out2 at 3 references and a4 at 7, out2 outranks a4 iff
 * 30000/L(out2) > 140000/L(a4), i.e. iff L(a4) > 4.667 * L(out2).  The whole
 * block-0 position sweep moves BOTH lengths in the same direction, so the
 * ratio barely moves: C0 (first) 192/43 = 4.465, B0 (natural) 190/42 = 4.524,
 * C3/C4 (last) 188/41 = 4.585.  All three fall short of 4.667 and the sweep is
 * monotone, so no block-0 position of out2's definition can seat it above a4
 * on the pa4-free chassis. */

void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    stptr = base;
    stptr += 0xFC; /* FAKE: F1 chain-extender */
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    tbl = D_80094CFC;
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
    func_8004A348(buf, out2);
    func_800523E0(a4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = (s32 *) (((u8 *) a4) + 0x20);
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
    func_8004A348(buf, out3);
    func_800523E0(a4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
