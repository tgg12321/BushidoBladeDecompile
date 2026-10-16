/* func_80052C10 — candidate frontier, session 1 (recon).
 * Score 4 (target 6 insns, build 5 insns): 1 operand-only hunk (register seat
 * $t9 vs our $v0 for the scratchpad address), 1 source-level hunk (target has
 * one extra trailing `nop` after `jr $ra` that this body does not produce).
 * NOT byte-matching yet — do not land. See evidence.md / hypotheses.md for
 * the reg-seat anomaly (target picks the highest-numbered scratch reg $t9,
 * contrary to MIPS gcc-2.7.2's lowest-first allocation order) and the
 * scan_hand_coded LOW/0 finding that argues against this being hand asm.
 */
void func_80052C10(void)
{
    *(vu32 *)0x1F800400 = 0;
}
