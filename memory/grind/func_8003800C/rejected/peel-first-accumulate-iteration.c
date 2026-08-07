/* REJECTED (s9, rederive) — score 12 (from floor 6).
 * Idea: peel the first accumulate iteration to give `sum` a real, non-bracketed
 * depth-1 reference (sum=*bp) at low LUID — a value-identical rederivation of the
 * byte checksum (sum over the same 0x24 bytes) that would, in theory, raise sum's
 * weighted refs WITHOUT the do-while(0) def-relocation (which couples Region A').
 *   bp = base + offset;
 *   sum = *bp; bp++; j = 1;
 *   do { sum += *bp; bp++; j++; } while (j < 0x24U);
 * RESULT 12: peeling emits an extra pre-loop lbu+addu and changes j's init to 1
 * and the loop trip count (0x23 in-loop), diverging structurally from target's
 * clean 0x24-iteration do-while with a plain `sum=0` init. The extra sum ref
 * cannot be sourced by peeling without changing the emitted bytes — reconfirms
 * (from the rederive angle) that a byte-neutral +1 sum ref is only available via
 * sum's own def bracket (do-while0, which couples A') per s6/s7. Dead.
 */
