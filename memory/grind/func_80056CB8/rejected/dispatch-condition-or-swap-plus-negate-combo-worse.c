/* func_80056CB8 -- s70 (enumerate modality), 2026-09-16
 * Closes the one combination s60 left untried in the type-dispatch
 * if/else block (the `*(u16*)(arg0+0x6A)==0x13||==6` check selecting
 * between the direct-angle read and the ratan2() fallback): the OR-operand
 * swap AND the if/else arm negate/swap applied TOGETHER (s60 tried each
 * independently: OR-swap alone -> 40/204, arm-negate alone -> 46/204).
 *
 *   if (*(u16 *)(arg0 + 0x6A) != 6 && *(u16 *)(arg0 + 0x6A) != 0x13) {
 *       flags += ratan2(...);
 *   } else {
 *       flags += *(s16 *)(obj + 0x1CA);
 *   }
 *
 * -> 48/204 (build_insns 199, +1 vs baseline's 198) -- WORSE than either
 * individual mutation and worse than the 38/204 baseline. Compounding two
 * independently-regressing respellings does not cancel; it stacks.
 *
 * Fresh baseline re-confirmation this session (mandatory kill re-audit,
 * candidate.c body unmodified): 38/204 (198 build insns), exact match to
 * the s22-s69-banked floor, zero drift. func_80053614 void->s32
 * return-type prerequisite + array-form D_8009A820/D_8009A821 externs
 * (scalar externs at old src/text1b.c:2183-2184 temporarily dropped for
 * the measurement only) reproduce the ledger's recorded chassis exactly.
 *
 * src/text1b.c reverted to INCLUDE_ASM("asm/funcs", func_80056CB8); before
 * session end (asm-until-matched; no draft C on main). This closes the
 * s37/s38/s60-flagged interior-if/else tooling gap's remaining hand-derived
 * combination -- three individual spellings (s60) + this one combined
 * spelling (s70) now cover every obvious reading of that block's dispatch-
 * condition spelling space. See hypotheses.md/evidence.md [s70].
 */
