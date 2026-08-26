/* MATCHED [s13, structural modality] — sandbox --disable all = 0, 33/33 insns,
 * 0 rules, AND full-build link SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * (`wteng main build` printed MATCH this session, with this body in src/code6cac_c.c).
 *
 * This body is ordinary C with ZERO constructs: no pin, no __asm__, no volatile,
 * no dead store, no FAKE annotation, no sanctioned-exception family.  It is the
 * plain PsyQ memory-card file-count idiom.
 *
 * HOW IT DIFFERS FROM THE s10/s11 floor-6 body (which was a dead end proven by s12):
 *   - `var_s1 = 0;` sits AFTER the sprintf call (not hoisted above it), and
 *   - the counter increment `var_s1++` is at the TOP of the do/while body, ahead of
 *     the pointer advance, so there is no explicit source-level peel and no explicit
 *     `var_s1 -= 1;` tail.  reorg.c steals the loop-top increment into the `bnez`
 *     delay slot and emits its own compensating `addiu $s1,$s1,-1` after the loop —
 *     which is exactly what target has at 0x80037A80.  Writing that decrement in C
 *     duplicates reorg's compensation (see rejected/s13-source-level-decrement-*.c).
 *   - The cse1 REG_WAS_0 fold that produced `li $s1,1` on every earlier chassis does
 *     not fire here: the surviving increment sits after the loop's CODE_LABEL, which
 *     terminates cse's extended basic block (cse.c cse_end_of_basic_block), so the
 *     dominating `var_s1 = 0` is not propagated into it.
 *   - The three-pseudo vJ chassis (s12) is SUPERSEDED: no base/walking-pointer split
 *     is needed at all; the single walking pointer has reg_n_sets == 2, so sched.c's
 *     birthing_insn_p boost never fires and `la $s0` stays at the top of the entry
 *     block (GATE 1 dissolves rather than being defeated).
 *
 * SYMBOL NAMES ARE LOAD-BEARING FOR THE LINK (s13 correction): the callees must be
 * spelled `sprintf`, `firstfile`, `nextfile` — the names the linker resolves.  An
 * earlier draft used `func_80079A30` / `bios_firstfile_B` / `bios_nextfile_B`; those
 * still score sandbox 0 (relocations are masked at object level) but FAIL THE LINK
 * with "undefined reference".  A sandbox 0 is not a match until `build` prints MATCH.
 */
s32 func_80037A20(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&D_80102810;
    sprintf(sp10, (s32)(&g_str_memcard_fmt), arg0, arg1);
    var_s1 = 0;
    if (firstfile(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (nextfile(var_s0) != 0);
    }
    D_800A38C8 = var_s1;
    return var_s1;
}
