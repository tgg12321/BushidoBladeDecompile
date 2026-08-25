/* REJECTED (s9, 2026-08-25) -- score 4, not 0.
 * Collapsing the staged xor of the two 16-bit head reads into a single expression
 * inside the unsigned compare costs 4 instructions. The distance-0 form keeps the
 * xor as its own statement writing the named local before the compare consumes it:
 *     top = top ^ cur;
 *     idx = (u32)top < 1;
 * This is a statement-granularity fact only; it is recorded as a measurement, NOT
 * as a justification for any construct (see self_vet.md T3/T4).
 */
    top = *(s16 *)(a0 + 0x332);
    cur = *(s16 *)(a0 + 0x14);
    idx = (u32)(top ^ cur) < 1;   /* sandbox --disable all = 4 (76/76) */
