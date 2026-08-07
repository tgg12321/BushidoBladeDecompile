/* s2 REJECTED — pure spelling V7: offset re-association — second pointer
 * offset spelled as a mask instead of the shift pair:
 *   p = (u32 *) (saved + (s32) (v_off & ~3));
 * NO FAKE store, floor-10 chassis. sandbox --disable all = 14, build
 * insns 52 vs target 53 — WORSE: target carries the srl/sll two-shift
 * pair; the mask spelling emits andi (one insn fewer) and cascades.
 * DEAD (and the spelling family is byte-diverging, not just RA-neutral).
 */
