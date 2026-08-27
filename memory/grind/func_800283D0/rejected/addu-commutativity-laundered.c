/* REJECTED (s2, 2026-08-26) — floor stayed 30, addu operand order UNCHANGED.
 * Frontier item 3 from s1: target emits `addu s3,v0,s0` (X + arg0) where we
 * emit `addu s2,s0,v0` (arg0 + X). Hypothesis was that the C-level operand
 * order of the plus feeds the RTL plus operand order. It does not: GCC's
 * pointer-arithmetic expansion canonicalizes `int + ptr` back to
 * `plus(ptr, int)` before RTL, so the emitted operand order is laundered.
 * The addu operand-order diff is therefore DOWNSTREAM of the register
 * assignment (s2 vs s3), not an independent C-spelling lever. */
                        u8 *temp_s3 = (temp_a1_2 * 2) + arg0;
