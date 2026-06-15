/* func_8001F938 (code6cac.c) — WIP NOTE. HEAD body is the candidate; the cheat
 * is 13 regfix rules (NOT in src), so there is nothing to paste — resume by
 * editing src/code6cac.c's func_8001F938 directly and retiring the rules.
 *
 * Honest sandbox distance 11 (107 == 107 insns). The 13 rules:
 *   - $3 <-> $5 reg-swap @ 1,7,11,15 (a register-pair allocation mismatch)
 *   - insert "andi $3,$5,0xFFFF" @3 (a folded kind&0xFFFF mask)
 *   - subst/delete/insert_label @74-80 synthesizing the
 *     `(probe < 4) ? (u16)probe : 3` ternary -> a `slt;bne .LfuncF938_join;sll`
 *     branch form + a stable join label.
 *
 * The gap is the BRANCH GENERATION for the two saturating ternaries
 * (`raw_or_3 = (probe<4)?(u16)*p:3; idx=((raw_or_3<<16)>>15)` and the sum
 * variant) plus the coupled $3/$5 swap. GCC emits a different compare/branch
 * shape than target; the regfix rewrites it. See switch-vs-ifchain-branch-sense
 * and compare-operand-order-register.
 */
