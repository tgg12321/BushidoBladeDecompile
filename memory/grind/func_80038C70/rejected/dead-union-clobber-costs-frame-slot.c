/* REJECTED (s2, 2026-07-17): dead union LOCAL with constructor init — the
 * clobber WORKS but the dead pseudo costs a frame slot.
 *
 *   case 9: case 11:
 *       sel = 0xD;
 *       { union { s32 a; } u = { 0 }; }   // u1
 *       goto sel_dispatch;
 *
 * Measured: the (clobber (reg u)) DOES survive to jump2 and DOES block the
 * merge (363 insns, case-9/11 block preserved as li 13) — first positive
 * confirmation of the F2 suffix-breaker mechanism. But u's pseudo has no real
 * refs, gets no hard reg, and reload assigns it a stack slot: frame 40 -> 48,
 * prologue/epilogue offsets all shift. Not byte-neutral; dead-local policy
 * family anyway.
 *
 * SUPERSEDED by the accepted spelling (see candidate.c): make the clobber
 * target sel's OWN allocated pseudo via sel-as-union + cast-to-union assign —
 * zero frame growth, no dead object. */
