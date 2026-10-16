/* func_80052C10 — candidate frontier, session 1 (recon).
 * Score 4 (target 6 insns, build 5 insns): 1 operand-only hunk (register seat
 * $t9 vs our $v0 for the scratchpad address), 1 source-level hunk (target has
 * one extra trailing `nop` after `jr $ra` that this body does not produce).
 * NOT byte-matching yet — do not land. See evidence.md / hypotheses.md for
 * the reg-seat anomaly (target picks the highest-numbered scratch reg $t9,
 * contrary to MIPS gcc-2.7.2's lowest-first allocation order) and the
 * scan_hand_coded LOW/0 finding that argues against this being hand asm.
 * s5: the trailing nop is the asmpsx-module signature (only 11 functions in the
 * binary end jr;nop;nop, and the other 10 are Sony asm modules); see evidence.md s5.
 * s5b: the Judge's TU-boundary shape (8-aligned next object) is falsified: 102
 * objects start at 4 mod 8 in the original link; the module-relative 16-pad fits
 * a module starting at 0x80052A88. Still score 4; do not land.
 * s6 (solver): $t9 seat class-killed for frame-free C (ascending RA, no $25 RTL
 * producer under -mno-abicalls; ladder reaches $t9 only at N=23, framed).
 */
void func_80052C10(void)
{
    *(vu32 *)0x1F800400 = 0;
}
