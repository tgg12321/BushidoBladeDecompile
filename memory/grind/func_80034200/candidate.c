/* MATCH for func_80034200 (session s3, structural).
 * sandbox --disable all => 0, build_insns 40 == target_insns 40,
 * full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (oracle).
 * 100% pure C: no register-asm pins, no __asm__, no volatile, no dead stores,
 * no unused locals, no padding array, no FAKE-family construct, 0 regfix/asmfix.
 *
 * Floor history: s1 21 -> s2 11 -> s3 10 -> 2 -> 0.
 *
 * THE THREE s3 LEVERS (all structural, each measured):
 *
 * L4. `u8 n` (NOT `s32 n`) as the loop-bound variable makes the outer loop a
 *     REAL do-while loop while KEEPING the target's outer-loop-tail re-read of
 *     D_800A389B.  s2 had banked "outer-LICM and the tail reload are mutually
 *     exclusive" -- that corollary is FALSE, and the reason is the exact width
 *     of `n`.  loop.c:scan_loop (lines 688-701 of the frozen source) needs ONE
 *     of three conditions to treat the tail load as a movable:
 *       (1) !maybe_never && !loop_reg_used_before_p
 *       (2) !REG_USERVAR_P(dest) && !REG_LOOP_TEST_P(dest)
 *       (3) reg_in_basic_block_p(insn, dest)
 *     With `s32 n` the u8 load needs a widening conversion, so it lands in a
 *     FRESH COMPILER TEMP (then a separate copy into n).  A temp is not a user
 *     var, so (2) passes and the load is hoisted -- this is what s2 measured
 *     and generalised too far.  With `u8 n` the load's destination IS the user
 *     variable, so:
 *       (2) fails -- REG_USERVAR_P(n) is true;
 *       (3) fails -- regno_first_uid[n] is the PRE-LOOP `n = D_800A389B;`,
 *           not this insn, so reg_in_basic_block_p returns 0 immediately;
 *       (1) fails -- maybe_never is 1 at the loop tail, because loop.c sets it
 *           past ANY label or jump (loop.c:921-930) and the inner loop supplies
 *           plenty of both.
 *     => the tail `lbu D_800A389B` is not a movable and survives, in a real
 *     loop.  That in turn restores outer-loop LICM, which is what finally puts
 *     `addiu $t2,$zero,3` in the PREHEADER at target's slot (s2's residual (a))
 *     with no constant-holder local.  Floor 11 -> 10.
 *
 * L5. `base = &D_800F65F8 + i * 2;` INSIDE the loop (instead of an
 *     initialisation before it plus `base += 2;` in the tail) makes base a
 *     strength-reduced induction variable.  Two effects:
 *       - loop.c runs move_movables BEFORE strength_reduce, so the giv's
 *         preheader initialisation is emitted AFTER the hoisted movables.
 *         Preheader order becomes [lbu D_800A3874, li 3, la D_800F65F8] --
 *         exactly target's.  No source-statement ordering can achieve this,
 *         because a plain `base = &D_800F65F8;` statement is emitted before
 *         the movables, not after them.
 *       - the giv is a fresh high-priority allocno that takes $a1, resolving
 *         s2's residual (b), the base<->shift swap, WITHOUT any ref-count
 *         mutation.  Floor 10 -> 2.
 *     The `&GLOBAL + i * 2` idiom is corroborated as original: the immediately
 *     preceding, already-matched function in the same file writes
 *     `*(&D_800F65F8 + (D_800A3874 * 2)) = D_800A3898;` (src/code6cac_b.c:3846).
 *
 * L6. `p = base;` BEFORE `end_p = base + 2;` in the loop head.  With the giv in
 *     place these two independent insns are emitted in source order, and target
 *     has `addu $v1,$a1,$zero` then `addiu $t0,$a1,2`.  Floor 2 -> 0.
 *     (The reverse order also produced the correct registers but the wrong
 *     emission order; a 12-form ordering sweep separated the two effects --
 *     4 of the 12 orderings lose the target register triple entirely because
 *     they change which pseudo the giv is folded into.)
 *
 * Carried over from s2 and still load-bearing:
 *   - OR straight into `acc` in both arms (no `v0` temp): keeps acc's allocno
 *     priority top of the list so acc lands in $a0; jump2's find_cross_jump
 *     re-merges the two `or` tails into target's single `or $a0,$a0,$v0`.
 *   - The -8 phantom frame (vars=8) is free with this shape; no coercion.
 *
 * `s32 useReal;` is declared and assigned separately only because C89 forbids a
 * declaration after the `base = ...` statement; the assignment must follow the
 * base computation to keep the giv folding.
 */
void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    u8 n;
    s32 innerBound;
    u8 *base;
    u8 *p;
    u8 *end_p;

    g_disp_enable = DISP_LOADING;
    n = D_800A389B;
    if (i < n) {
        innerBound = D_800A3874;
        do {
            s32 useReal;
            base = &D_800F65F8 + i * 2;
            useReal = (i < innerBound);
            p = base;
            end_p = base + 2;
            do {
                if (useReal) {
                    acc |= ((s32)*p) << shift;
                } else {
                    acc |= 3 << shift;
                }
                p++;
                shift += 2;
            } while ((s32)p < (s32)end_p);
            i++;
            n = D_800A389B;
        } while (i < n);
    }

    D_800A3784 = acc;
}
