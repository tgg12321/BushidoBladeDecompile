/* SESSION 16 (2026-09-01, forensics) -- KILLED, 3 / 78, mechanism dump-proven.
 * Second REAL write to idx via a call return (`idx = rand();`) does NOT raise
 * reg_n_sets at schedule time: the insn is a hard-reg copy `(set (reg/v:SI 74)
 * (reg:SI 2 v0))` (insn 118) and COMBINE.C DELETES IT.  Dump trail (this
 * session, tmp/grind/func_800645B0/s15b/): g.rtl.txt / g.cse.txt / g.cse2.txt /
 * g.flow.txt all carry insn 118 with 2 sets of reg 74; g.combine.txt and
 * g.sched.txt carry ONE set and no insn 118.  With reg_n_sets[idx] back to 1,
 * sched.c birthing_insn_p lifts the loop-top addu to max_priority and the WD
 * 11/12/65 residual returns verbatim (operand order at 20 stays exact).
 * GENERALISATION (with the s15 cse.c finding): a second set of the carrier that
 * is a REG-REG COPY is deleted -- by cse.c when it is pseudo->pseudo, by
 * combine.c when it is hardreg->pseudo.  The second write must compute
 * something (see the `idx = last & 7;` form banked as
 * both-halves-idx-second-real-nonco*.c, which survives and scores 2 / 78).
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                idx = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = idx & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
