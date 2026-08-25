/* s19 REJECTED-BUT-LOAD-BEARING (sandbox 11, 132 build insns). W4 chassis (loop1 a
   real do-while, address derived from the index inline) with target's block-2 spelling
   `out3 = (s32 *)((u8 *)pa4 + 0x20)`. Measured allocno table (s19/W6/cc1.err):
     giv 9/40=6750 $s3 | i 11/97=3402 $s4 | tbl 7/47=2978 $s5 | pa4 9/94=2872 $s6
     | stptr2 6/48=2500 $s0 | out2 5/41=2439 $s7 | a3 5/99=1010 $fp | out3 3/47 $s3
   out2 and pa4 are SWAPPED versus target and everything else is target's. out2's live
   length is already exactly 41 -- the value the window needs -- so the entire residual
   of this chassis is ONE extra flow-counted reference on out2 (6/41 = 2926 lands
   between pa4 2872 and tbl 2978). Confirmed by W8. */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, pa4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, out2);
        func_800523E0(pa4, out2, a3, ents + i * 0x68 + 0x38);
        *((s16 *) (ents + i * 0x68 + 6)) = 2;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
    out3 = (s32 *) (((u8 *) pa4) + 0x20);
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
