/* hirahira_w_ctrl - s3 candidate. sandbox --disable all == 1 (prev best 5).
 * 132/132 insns, frame 72 == target 0x48. Pure C: no pin, no asm, no
 * volatile, no dead code, no FAKE constructs.
 *
 * s3 BREAKTHROUGH (on top of the r3-r16 six levers, see git history):
 *  7. loop2's out-pointer goes through the REUSED loop1 `stptr` local
 *     (dead after loop1), re-initialized by READING out2
 *     (`stptr = (s32) out2;`) instead of recomputing pa4+0x20, and
 *     lever-4 is REVERTED (loop2's func_800523E0 takes pa4 again).
 *     Effects (all measured in tmp/grind/func_80041188/s3/):
 *       - stptr's pseudo (merged loop1-walker + loop2-out) lands s3 =
 *         target (it shares dead loop1 stptr's register naturally);
 *       - the `stptr = out2` read is out2's 4th flow-counted ref and
 *         extends its live range into the loop2 preamble, lifting out2's
 *         global.c priority above the pa4 carrier's => out2 keeps s6,
 *         carrier keeps s7, a3 keeps fp (all callee-saved == target);
 *       - reverting lever-4 restores `move a0,s7` at loop2's 523E0
 *         (the lw a0,88(sp) residual class is gone).
 *     5-insn residual -> 1: insn 71 `move s3,s6` vs target
 *     `addiu s3,s7,32`. Spelling the re-init from pa4 instead
 *     (`stptr = (s32)((u8*)pa4+0x20);`) emits the right bytes but loses
 *     out2's 4th ref -> out2 falls to pri 714 and the 3-cycle returns
 *     (see s3 ledger for the measurement).
 */
void hirahira_w_ctrl(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
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
    stptr2 = saved + 0x750;
    stptr = (s32) out2;
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
    func_8004A348(buf, (s32 *) stptr);
    func_800523E0(pa4, (s32 *) stptr, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
