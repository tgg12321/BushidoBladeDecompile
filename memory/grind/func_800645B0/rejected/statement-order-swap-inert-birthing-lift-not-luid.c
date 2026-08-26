/* REJECTED (session 13, 2026-08-25) — "WD-SWAP": the WD chassis with the two
 * inner-loop-top statements written in the opposite source order
 * (`val = 1;` before `idx = i + j;` instead of after).
 *
 * MEASURED 3 / 78 by `sandbox func_800645B0 --disable all` — BYTE-IDENTICAL to
 * plain WD (3 / 78), same three differing positions against
 * asm/funcs/func_800645B0.s (objdump-normalised, engine.score):
 *     11  OURS li v1,1          TGT addu s0,s3,a0
 *     12  OURS addu s0,s3,a0    TGT li v1,1
 *     65  OURS li v1,1          TGT addu s0,s3,a0
 *
 * WHY THIS IS WORTH BANKING (it is a mechanism kill, not just another 3).
 * Source statement order DOES move the compiler's internal state here and the
 * bytes still do not move.  With this order the instrumented cc1's sched model
 * (tools/sched_solver/extract.py text1b) reports, for func_800645B0 pass 1
 * block 2 (the inner-loop-top block, 6 insns):
 *     LUIDs   {val=1: 0, idx=i+j: 1, sllv: 2, lw: 3, and: 4, bne: 5}
 * i.e. the LUIDs follow the new source order, and the pass-1/pass-2 pick lists
 * change accordingly — yet the EMITTED order does not, because the pick is not
 * decided by the LUID tie-break at all.  The pass-1 `adjpri` records show
 * `birth: 1` (birthing_insn_p: reg_n_sets == 1 on a live destination) for the
 * `idx = i + j` insn and `birth: 0` for the `val = 1` insn — on this chassis
 * `idx` is written ONCE (the *3 sum goes to the fresh `wid`) while `val` is
 * written three times.  sched.c's adjust_priority therefore lifts the idx addu
 * to max_priority in EVERY spelling; the backward list scheduler picks it early,
 * which means it is emitted LATE, and `val = 1` lands at the loop top where the
 * target has the addu.  The priorities are never tied, so INSN_LUID — the only
 * thing C statement order controls here — never gets to decide.
 *
 * CONSEQUENCE FOR THE SEARCH.  Every "reorder these two statements" spelling of
 * this residual (including the ones this function's judge_constraints already
 * ban) is inert on the WD chassis, for a named reason.  The single real knob is
 * `reg_n_sets[idx]`: > 1 denies the lift and hands back the loop head (that is
 * the SB chassis, 1 / 78, which then loses the *3 operand order to optabs.c's
 * commutative swap — H24 / H58 / H59), == 1 wins the operand order and loses the
 * head (this chassis).  Do not re-derive a statement-order variant of either.
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
            val = 1;
            idx = i + j;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
