/* REJECTED (s2) -- any REAL outer loop (while / for / do-while) loses the
 * target's tail re-read of D_800A389B.
 *
 * Mechanism (measured + read out of the frozen compiler source):
 *   A real loop carries NOTE_INSN_LOOP_BEG/END, so loop.c:scan_loop runs on it
 *   and the loop-tail `lbu D_800A389B` is a single-set movable whose src is
 *   invariant_p (no stores and no calls in the loop => loop_store_mems is empty).
 *   The three escape hatches in scan_loop's gate all fail to fire here:
 *     - cond (3) reg_in_basic_block_p is TRUE (load + its slt sit in one BB),
 *     - the may_trap_p guard does not bite: rtx_addr_can_trap_p(SYMBOL_REF)
 *       returns 0, so `maybe_never` cannot block a plain global load,
 *     - n_times_set != 1 does not help either (see the sibling rejected file
 *       variable-reuse-does-not-defeat-licm.c): the load always lands in its
 *       own fresh temp pseudo before any user-variable copy.
 *   move_movables' cost test (threshold * savings * lifetime >= insn_count)
 *   passes trivially because threshold = 2 * (1 + n_non_fixed_regs).
 *   => the read is hoisted into the preheader and cse2 folds it to a `move`
 *      off the duplicated entry-guard value.  Target has TWO reads.
 *
 * Measured: sandbox --disable all, reload absent, floor >= 14 for every
 * real-loop spelling tried (while / for / do-while, bound inline or via a
 * local, tail test reordered).  The goto-form outer loop in ../candidate.c is
 * the only shape that reproduces the tail reload.
 *
 * Corollary banked for future sessions: outer-LICM (which would place the
 * preheader `li $t2,3`) and the tail reload are MUTUALLY EXCLUSIVE under this
 * compiler -- they cannot both come from loop.c.
 */
void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    s32 innerBound;
    u8 *base;
    u8 *p;
    u8 *end_p;

    g_disp_enable = DISP_LOADING;
    if (i < D_800A389B) {
        innerBound = D_800A3874;
        base = &D_800F65F8;
        do {
            s32 useReal = (i < innerBound);
            p = base;
            end_p = base + 2;
            do {
                s32 v0;
                if (useReal) {
                    v0 = ((s32)*p) << shift;
                } else {
                    v0 = 3 << shift;
                }
                acc |= v0;
                p++;
                shift += 2;
            } while ((s32)p < (s32)end_p);
            base += 2;
            i++;
        } while (i < D_800A389B);
    }

    D_800A3784 = acc;
}
