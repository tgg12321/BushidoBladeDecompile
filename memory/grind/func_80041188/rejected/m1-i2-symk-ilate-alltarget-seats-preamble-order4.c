/* s9 (structural, 2026-08-23). THE ALL-TARGET-SEATS FORM ON THE TWO-LOCALS
 * CHASSIS. = I2 (the s8 separated between-loops restore of out2, which emits
 * target's slot-72 `addiu $s3,$s7,0x20`) + the s4/s5 tbl sym-K split-init ref
 * lift (tbl 4 -> 6 refs) + `i = 1` moved out of the declaration to the LAST
 * statement of block 0.  .lreg: stptr 7/41=3414, stptr2 6/48=2500,
 * tbl 6/48=2500, i 8/96=2500, out2 5/42=2380, pa4 7/95=1473, a3 4/99=808,
 * out3 3/47=638, saved 2/47=425 (spilled).  tbl and i tie EXACTLY at 2500 and
 * the tie breaks our way on allocno 78 < 79.  Every callee-saved disposition
 * equals target (checked mechanically against the seat map).
 * 
 * sandbox --disable all = 4, 132/132 insns.  REJECTED: the residual is the
 * 5-insn preamble emission ORDER and nothing else -
 *    OURS    sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO / sw s4,48(sp) / li s4,1
 *    TARGET  sw s4,48(sp) / li s4,1 / sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO
 * i.e. moving `i = 1` after tbl's definition is what BUYS the seats (it is the
 * only mutation in the whole ledger that moves tbl's reg_live_length off 47) and
 * is simultaneously what LOSES the emission order, because flow.c:1685 counts
 * live length over the RTL insn order and sched.c rank_for_schedule falls back to
 * `INSN_LUID (y) - INSN_LUID (x)` for the two priority-1 / ref_count-0 leaf defs.
 * One variable, two consequences, anti-correlated.  See evidence.md s9.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i;
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
    i = 1;
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
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr2 = saved + 0x750;
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
