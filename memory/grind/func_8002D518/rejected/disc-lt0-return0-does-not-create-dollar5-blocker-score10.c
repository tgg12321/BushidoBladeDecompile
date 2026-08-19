/* REJECTED s6 (forensics) — score 10, build_insns 142 != target 144. WORSE.
 * (Re-measurement of the s3 form, this time WITH dumps, as the s5 frontier asked.)
 * FORM: `if (disc < 0) return 0;` instead of the `result = 0;` arm.
 * WHAT THE DUMPS PROVED — the s5 frontier's hope that "solving slot 88 may be
 * what creates the $5 blocker" is FALSE:
 *   .greg: `;; 117 conflicts: 108 116 117 2 29`  — UNCHANGED, no 123 edge.
 *   .lreg: 123 goes 4 refs/9 insns (pri 8889, position 7) -> 3 refs/7 insns
 *          (pri 4286, position 13). It moves DOWN the allocno order, i.e.
 *          FURTHER from being able to block $5 for disc. 117 itself is bit-for-
 *          bit unchanged (5 refs/6 insns, pri 16667, position 1, preference 3, $3).
 * So frontier items (a) and (b) are NOT one problem; slot 88 is allocation-neutral.
 */
/* (variant is base candidate.c with `if (disc < 0) return 0;` replacing the result=0 arm) */
