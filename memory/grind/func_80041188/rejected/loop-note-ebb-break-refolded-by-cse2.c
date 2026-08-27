/* REJECTED (s34, 2026-08-27) -- form L1: alt_D5_alltargetseats_score2_s27.c with
 * block 2's first statement wrapped in `do { a1 += 0x6C; } while (0);` purely to put
 * a NOTE_INSN_LOOP_END between loop1's back-branch and block 2's
 * `out3 = (s32 *)((u8 *)pa4 + 0x20);`. INSTRUMENT ONLY -- never a candidate.
 *
 * WHY IT IS DEAD. cse.c:8055 ends a cse basic block at a NOTE_INSN_LOOP_END only
 * when `after_loop == 0`. cse1 has after_loop == 0, so the instrument WORKED there:
 * tmp/grind/func_80041188/s34/L1/red.i.cse insn 190 is still
 * `(set (reg/v:SI 87) (plus (reg/v:SI 77) (const_int 32)))`, i.e. cse1 did not fold
 * block 2 -- the first time in the grind the fold was switched off without paying
 * for a CODE_LABEL. But cse2 (-frerun-cse-after-loop, on at -O2) runs with
 * after_loop == 1, the note is inert there, and red.i.cse2 insn 190 is
 * `(set (reg/v:SI 87) (reg/v:SI 86))` -- the fold, re-done, still before flow counts.
 * Measured sandbox --disable all = 2 at 133 insns, bit-identical to D5, object still
 * emits `move $s3,$s6`.
 *
 * CONSEQUENCE: the entire loop-note route to preserving target's block-2
 * `addiu $s3,$s7,0x20` is closed, independently of s32's strength-reduction
 * foreclosure. Only a CODE_LABEL terminates both cse passes, and E-s31-3 measured
 * that at +2 instructions. See evidence.md E-s34-1.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
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
    stptr = base;
    stptr += 0xFC;
        loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl += 2;
    tbl -= 1;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i += 2;
    i -= 1;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    do {
        a1 += 0x6C;
    } while (0);
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
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
