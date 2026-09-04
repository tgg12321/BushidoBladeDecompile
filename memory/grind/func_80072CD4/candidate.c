/* func_80072CD4 - s14f (2026-09-04, synthesis). Measured THIS session with this exact text
 * in src/text1b.c: `sandbox func_80072CD4 --disable all` = 2, build_insns 79 == target_insns 79,
 * rules_dropped 0 (tmp/grind/func_80072CD4/s14f/sandbox_G2_armE_wrap.txt). The previous
 * candidate (score 0) is retired to rejected/s14e_armcarrier_mergewrap_score0_banned_by_ruling15.c
 * because judge_constraints entry 15 bans its arm carrier local.
 *
 * WHAT THIS BODY IS. Each arm writes its own three per-arm colour components AND its own
 * vertex-1 blue directly at the destination - the plainest possible C, no carrier local of any
 * kind, no per-arm wrap. jump.c's cross_jump then merges the two arms' identical trailing
 * `sb v0,0xE` into the join, leaving `li v0,0x32` in the delay slot of arm 1's `j` and
 * `li v0,0x46` at the end of arm 2 - which is byte-for-byte the target's arm shape, including
 * the single-register ($v0) constant economy that thirteen sessions had only ever reached with
 * a multi-write carrier local. The one remaining device is the merge-head do-while(0) that
 * judge_constraints entry 15 explicitly leaves available.
 *
 * THE RESIDUAL IS EXACTLY 2 INSTRUCTIONS, and it is a store ORDER in the join block:
 * this body emits `sb v0,0xE / sb v1,4 / sb v1,0xC`, the target has `sb v1,4 / sb v1,0xC /
 * sb v0,0xE`. The 0xE store is the cross-jumped arm tail, so the join label sits in front of
 * it and no merge-block-resident spelling of the r0/r1 stores can get ahead of it.
 * Disassembly: tmp/grind/func_80072CD4/s14f/G2.dis (everything else is byte-identical).
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        /* FAKE: do-while(0) wrap around the two r0/r1 stores, mechanism: GCC 2.7.2
         * sched.c schedule_select() - at equal INSN_PRIORITY it picks the ready insn with
         * the largest potential_hazard (sched.c:2706-2721), so a store always outranks a
         * plain li and producer-less stores are driven to the block tail; the wrap makes
         * these two stores their own scheduling region whose tail is the merge-block head.
         * lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md [s9 H3, s14e H2/H3,
         * s14f] - register-anti-dependence, memory-alias and statement-order deferral are
         * all measured dead (12/79, 5/79, 4/79, 4/79, 4/79, 14/79). */
        do {
        *(u8 *)((s32)(arg1) + 4) = red;
        *(u8 *)((s32)(arg1) + 0xC) = red;
        } while (0);
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
