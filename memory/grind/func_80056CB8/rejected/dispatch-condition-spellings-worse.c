/* func_80056CB8 -- s60 (enumerate modality), 2026-09-16
 * Three untried spellings of the type-dispatch if/else block
 * (immediately after the obj/flags-0x1000 check, selecting between
 * the *(s16*)(obj+0x1CA) direct-angle read and the ratan2() fallback),
 * all measured WORSE than the 38/204 baseline on the s22-s59-banked
 * chassis:
 *
 *   (1) OR-operand swap: `== 6 || == 0x13` instead of `== 0x13 || == 6`
 *       -> 40/204 (build_insns 198, same insn count, worse RA residual)
 *
 *   (2) Hoist the repeated `*(u16 *)(arg0 + 0x6A)` read into a named
 *       local `u16 type = ...;` read twice instead of re-reading it
 *       -> 58/204 (build_insns 180, -18 insns: this FOLDS the compiler's
 *       real behavior, target re-reads arg0+0x6A twice, so caching it
 *       destroys required codegen, not just a cosmetic respelling)
 *
 *   (3) Negate the condition and swap the if/else arms (ratan2 branch
 *       first, direct-angle branch second)
 *       -> 46/204 (build_insns 199, +1 insn, branch-sense flip changes
 *       which arm gcc treats as the "then" path for scheduling)
 *
 * All three reverted; src/text1b.c returned to INCLUDE_ASM (per
 * asm-until-matched, no draft C committed to main). See
 * hypotheses.md/evidence.md [s60] for the full writeup.
 */
