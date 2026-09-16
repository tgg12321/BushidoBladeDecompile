/* REJECTED (session 5, enumerate). Block 2's tail written as two separate
 * fresh named locals instead of full inlining:
 *
 *   s32 o18;
 *   s32 o1c;
 *   o18 = *(s32 *)(arg1 + 0x18);
 *   *(s16 *)(tile + 8) = o18;
 *   *(s16 *)(tile + 0xC) = 0x78;
 *   *(s16 *)(tile + 0xE) = 1;
 *   o1c = *(s32 *)(arg1 + 0x1C) + 0xE;
 *   *(s16 *)(tile + 0xA) = o1c;
 *
 * Measured: sandbox score DID improve (45 -> 42) but build_insns DROPPED
 * to 194 vs target's 199 -- combine folded something away that the
 * fully-inlined form (candidate.c) keeps at exact 199==199 parity. A
 * lower masked score with broken insn-count parity is a worse chassis to
 * build on (it means some OTHER real diff opened up elsewhere in the
 * function), so this form was reverted in favor of the full-inline
 * spelling. Re-test if a future session finds the missing 5 instructions
 * are recoverable elsewhere -- until then this exact two-local spelling on
 * this exact chassis is a KILLED instance.
 */
