/* REJECTED s6 (forensics) — score 11, build_insns 143 != target 144. WORSE.
 * FORM: hoist `s32 result = 0;` above the discriminant computation and spell the
 * guard `if (disc >= 0) { ... }` (dropping the separate `disc < 0` arm and the
 * inner `result = 0;`).
 * WHY IT WAS TRIED: s5/E4 requirement 2 — `disc` (allocno 117) needs a NEW
 * conflict edge to an early-allocated $5 holder, and s5 recorded "requirement 2
 * has no current mechanism at all".
 * WHAT IT PROVED (this is the value — the edge IS creatable in ordinary C):
 *   `;; 117 conflicts: 108 116 117 123 2 29`  <- 123 (`result`) is now there.
 * WHY IT STILL FAILS: creating the edge requires lengthening `result`'s live
 * range across `disc`, which collapses pri(123) from 8889 (3 refs/9 insns,
 * position 7) to 811 (3 refs/37 insns, position 23 of 24) — i.e. 123 is now
 * allocated AFTER 117 and blocks nothing; it took $8, not $5. The conflict-edge
 * requirement and the allocation-order requirement are ANTI-CORRELATED.
 * Do not re-propose without a way to keep the $5 holder's live range short
 * while still overlapping disc's 6-insn range.
 */
/* (variant is base candidate.c with the two edits described above) */
