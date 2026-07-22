/* s10 KILLED (score 12, build 80, from floor 2). Region A' attempt via the Region-B
 * trick: make the inner accumulate index-based (bp -> strength-reduced giv) to
 * restructure the preheader like Region B's k-loop.
 *   do { sum = 0; } while (0);
 *   j = 0;
 *   do { sum += base[offset + j]; j++; } while (j < 0x24U);
 * RESULT: `base[offset + j]` has TWO varying indices (offset changes per OUTER
 * iteration, j per inner) so GCC cannot reduce it to a single clean walking pointer
 * like the k-loop's base+k*stride -> emits offset+j recompute (+1 insn, build 80) and
 * diverges. Target's inner loop uses an explicit bp=base+offset walker (byte-identical
 * to candidate). The index trick works ONLY where the loop base is loop-invariant
 * (the k-loop), NOT the accumulate (base+offset is outer-variant). Region B-specific.
 */
