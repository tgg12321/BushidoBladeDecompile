/* REJECTED s30 (structural). Score 9, identical floor fingerprint
 * (target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150).
 *
 * Probe: hoist the second debug_printf's three independent global reads into
 * tightly-scoped temps declared in REVERSE source order immediately before the
 * call (minimal register pressure, temps live across zero calls). A distinct
 * live-range/LUID regime from s29 (block-top hoist -> score 35) and s23
 * (permuter chassis-7 block-top temps). Goal: test whether axis-A BF68[0]
 * materialization is sensitive to sibling-read evaluation order.
 *
 * Result: KILLED. BF68[0] stays combine-folded to the 2-insn form regardless of
 * sibling-read source order; the combine offset-0 single-use fold (combine.c:1458
 * added_sets_2, s6/s25) is per-expression and independent of the evaluation order
 * of neighbouring independent reads. Axis B unchanged. Confirms axis A is
 * insensitive to sibling-read ordering at minimal register pressure -- a
 * placement not previously measured.
 */
        (void)new_var;
        {
            s32 r3 = D_8009BF70;
            s32 r2 = D_8009BF6C;
            s32 r1 = D_8009BF68[0];
            debug_printf(&D_80016044, r1, r2, r3);
        }
        temp_v0 = motion_make_table(0);
