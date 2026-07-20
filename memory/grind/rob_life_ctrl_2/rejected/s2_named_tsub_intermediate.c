/* REJECTED s2 — named `s32 tsub` intermediate for outer subu.
 * Form: { s32 tsub = 0x1000 - ((s * (0x1000 - f)) >> 12); t = (v * tsub) >> 12; }
 * Score: 2 (unchanged). Same 2-insn diff at 0xC4/0xCC.
 * GCC still emits mflo t0 + sra a1,t0,0xc; the block-local named intermediate
 * did not affect the priority tiebreaker for the mult-result pseudo. */
