/* CLOSER 2026-07-09 — cpu_side_move_dir_4 (= libcd CD_sync) volatile-family probes.
 * ALL KILLED vs the non-volatile h5 floor (masked 2). Baselines: HEAD src = masked 7.
 *
 * P2 direct-member volatile struct access — masked 38, build 159/160. KILL.
 *   typedef struct { u8 sync; u8 ready; u8 c; } CD_intr;
 *   extern volatile CD_intr D_800A1494;
 *   ... arg4 = tbl_125c[D_800A1494.sync]; arg5 = tbl_125c[D_800A1494.ready];
 *   Mechanism: direct %lo(sym) addressing per access — loses the two-pointer
 *   callee-saved web the target shows (target stages BOTH &Intr (s2) AND
 *   &Intr+1 (s4) in the prologue: addiu s4,s2,1).
 *
 * P3 volatile-pointer chassis + honest idx_1495 = idx_1494+1 — masked 28, 160/160. KILL.
 *   volatile u8 *idx_1494 = &D_800A1494.sync; volatile u8 *idx_1495 = idx_1494+1;
 *   Window diff: both lbu's place correctly (adjacent, like target) but the
 *   name-load (lui/lw a1 of D_800F19C0) stays late (target: right after lbu's),
 *   lw a3 early (target: last), plus a full s-reg permutation
 *   {s3,s4,s5,s1,s6,s2} vs target {s5,s6,s3,s2,s4,s1} (masked but shows the
 *   allocation web rotated — volatile nrefs weighting differs).
 *
 * P4 = P3 + volatile Alarm globals (D_800F19B8/BC/C0) — masked 28, BYTE-IDENTICAL
 *   to P3. Alarm volatility is INERT on this chassis. KILL.
 *
 * Conclusion: unlike the CD_ready twin (where volatile idx_1496 fixed region-3
 * and set a new floor 2), CD_sync has no idx_1496-analog usage, and making the
 * window loads volatile REGRESSES it. csmd4's endgame lever is NOT volatility
 * of the window reads; the non-volatile pointer chassis (h5-adjacent) stays the
 * floor. The honest replacement for the h5 cross-symbol initializer is still
 * open (Ruling 1: SEPARATE objects confirmed — h5 = fakematch).
 */
