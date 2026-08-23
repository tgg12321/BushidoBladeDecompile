/* s10 REJECTED as a SHIPPABLE FORM (semantic no-op arithmetic), but the most
 * informative measurement of the session. K1_L0_ABOPI (sym-K tbl + i=1 first +
 * the s8 separated between-loops out2 restore that emits target's
 * `addiu $s3,$s7,0x20`) with ONE extra flow-counted reference to `i`, spelled
 * `stptr2 = saved + 0x750 + (i - 0x12);` placed BEFORE `i = 0x12;` so cse1
 * cannot constant-fold it.
 *   i 9 refs / live 99 = 2727  >  tbl 6/47 = 2553  -> i keeps $s4, tbl $s5.
 *   SEATS ALL-TARGET - the FIRST time the i-FIRST (target preamble emission)
 *   order and the O-chassis (target's slot-72 addiu) have ever co-existed with
 *   every callee-saved seat equal to target.
 * sandbox --disable all = 3, build 133 insns vs target 132: the ENTIRE residual
 * is the one extra insn that computes the (i - 0x12) zero. A byte-free 9th
 * flow-counted reference to `i` would put this form at distance 0.
 * NOT shippable: `(i - 0x12)` has no observable effect (T1 fails), no human
 * would write it (T2), and its only justification is the flow.c:2081 ref count
 * (T3). Banked as the proof that the 9th-ref axis PAYS, not as a candidate. */
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
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
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
    stptr2 = saved + 0x750 + (i - 0x12);
    i = 0x12;
    out3 = out2;
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
