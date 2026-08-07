/* s8 rederive: s16-typed hword walker.
 *   s16 *hp = (s16*)((s32)p + 4);
 *   a1v = *hp++; a2v = *hp++; ... p = (u32*)hp;
 * Instead of u32 *p incremented by byte casts. m2c-suggested shape for
 * the hword reads, but restructures the loop's pointer traversal enough
 * that GCC assigns hp to a different register than $s0 and the whole
 * load-block cadence diverges from target.
 * Sandbox --disable all: score=31 (WORSE than baseline 1; 30 new diffs).
 * KILLED — the u32-with-byte-casts walker shape is load-bearing for the
 * target's $s0 register-cadence across the whole hword-load block.
 */
