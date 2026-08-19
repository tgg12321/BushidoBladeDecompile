/* REJECTED [s1] — measurement, not policy: both spellings of a plain fresh
 * local copy of disc FOLD (GCC coalesces the copy back into disc's register;
 * no `addu $a0,$a2,$zero` delay-slot copy materializes, floor stays 30).
 * Target needs the copy to SURVIVE to reorg. Do not re-propose these two
 * spellings verbatim; see hypotheses.md H2 for the attribution-first plan.
 *
 * Spelling A (asm + srlv both read ud):
 *     u32 ud = disc;
 *     if (disc >= 0) { __asm__ volatile(... : "=m"(sp_tmp) : "r"(ud) : "$12"); ... }
 *     ... (&D_8008D118)[ud >> shift] ...
 *
 * Spelling B (asm reads disc, only srlv reads ud):
 *     u32 ud = disc;
 *     if (disc >= 0) { __asm__ volatile(... : "=m"(sp_tmp) : "r"(disc) : "$12"); ... }
 *     ... (&D_8008D118)[ud >> shift] ...
 *
 * Both produce byte-identical output at floor 30 (the ud pseudo is folded
 * away; delay slot at build 0x1a60 is a nop where target 8002D680 has
 * addu $a0,$a2,$zero).
 */
