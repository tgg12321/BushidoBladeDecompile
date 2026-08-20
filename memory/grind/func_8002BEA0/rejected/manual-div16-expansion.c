/* REJECTED (s1, 2026-08-20) — hand-expanded signed /16 in the else arm.
 *
 * This was the prior candidate's else-arm spelling:
 *
 *     s32 v1_2 = 0x44C - ((s32) var_t0);
 *     var_a0 = v1_2 >> 4;
 *     if (v1_2 < 0) {
 *         var_a0 = (v1_2 + 0xF) >> 4;
 *     }
 *
 * Emitted bytes for the arm itself are IDENTICAL to plain
 * `var_a0 = (0x44C - (s32)var_t0) / 16;` (GCC 2.7.2's own sdiv-by-16
 * expansion), but the RTL/basic-block shape differs, and that shape is
 * what kept the function at floor 2: with the manual expansion, reorg's
 * mark_target_live_regs put $a0 (bit 4 of oppregs=0x20000f90) in the
 * opposite-thread needs of the `slti/beqz 0x44C` branch, so
 * fill_slots_from_thread REJECTED insn 186 (`lui $a0, hi(0x51EB851F)`,
 * setsopp=1 -> LOSE) and stole insn 99 (`li $v1, 0x44C`) instead —
 * exactly the 2-insn delay-slot/materialization-order residual.
 * Proof: tmp/grind/func_8002BEA0/s1/dbr_debug.log lines 744-750
 * (BB2_DBR_DEBUG on instrumented cc1).
 *
 * With plain `/ 16` (the COMPLETED-C sibling func_8002BC68's spelling),
 * $a0 drops out of the opposite-thread needs, trial insn 186 WINS the
 * delay slot, and the function reaches sandbox --disable all == 0.
 * The plain division is also the human-natural form; never re-introduce
 * the manual expansion. */
