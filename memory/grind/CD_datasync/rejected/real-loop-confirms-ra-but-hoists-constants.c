/* REJECTED AS A FORM — but this is the session's KEY POSITIVE RESULT.
 * Session 1.  Score 29 (baseline 18); 98 build insns vs 91 target.
 *
 * ============================ WHY IT MATTERS ============================
 * This form CONFIRMS the mechanism behind cluster A (the 3-way callee-save
 * rotation that regfix.txt papers over with `saEft01Init: $16 <-> $18` +
 * `$16 <-> $17`).  Do not treat the score regression as a kill of the
 * mechanism — only of this particular spelling.
 *
 * Baseline (goto-loop) cc1 -da .greg for saEft01Init:
 *     ;; 5 regs to allocate: 73 72 77 76 75
 *     72 in 16   77 in 17   76 in 18   75 in 19
 *   i.e. 72(param)->$s0, 77(tbl_125c)->$s1, 76(idx_1494)->$s2,
 *        75(tbl_11dc)->$s3.
 *
 * Target requires:
 *        77(tbl_125c)->$s0, 76(idx_1494)->$s1, 72(param)->$s2,
 *        75(tbl_11dc)->$s3   ==> allocation order 77 76 72 75.
 *
 * With the loop written as a REAL loop (this file), the .greg becomes:
 *     ;; 7 regs to allocate: 73 77 76 72 109 84 75
 *     77 in 16   76 in 17   72 in 18   84 in 20   75 in 21
 *   The param drops from allocation rank 1 to rank 3 and lands in $s2 —
 *   EXACTLY the target permutation for the three pseudos that matter.
 *
 * Mechanism (read from tools/gcc-2.7.2/global.c:allocno_compare):
 *     priority = floor_log2(n_refs) * n_refs / live_length   (size 1)
 *   and flow.c weights each reference by the enclosing loop_depth.  A
 *   `goto` back-edge emits NO NOTE_INSN_LOOP_BEG, so loop_depth stays 1
 *   everywhere and the param (2 refs) ties/outranks the table pointers
 *   (3 refs) on live_length.  A real loop construct emits the loop notes,
 *   the in-loop references get depth-weighted, and the two 2-use table
 *   pointers cross the floor_log2 step that the 1-use param cannot.
 *
 * ============================ WHY THIS FORM FAILS ============================
 * Emitting the loop notes also turns on loop.c's invariant hoisting, and it
 * hoists the two loop-invariant COMPARE CONSTANTS into fresh callee-saves:
 *     lui $s4, 0x3c     (0x3C0000, the cnt threshold)
 *     lui $s3, 0x100    (0x1000000, the D_800A14C0 bit mask)
 * That adds two allocnos (84, 109), pushes tbl_11dc from $s3 to $s5, and
 * costs +4 prologue/epilogue save/restore insns +2 hoisted `lui` +1 branch
 * restructure = 98 insns against target's 91.  Target materialises BOTH
 * constants inline inside the loop (`lui $v0,(0x3C0000>>16)` at target idx
 * 38, `lui $v1,(0x1000000>>16)` at idx 77) and uses exactly $s0-$s3.
 *
 * ============================ THE NEXT PROBE ============================
 * Find a spelling that emits NOTE_INSN_LOOP_BEG (so the allocno priorities
 * flip) WITHOUT letting loop.c hoist the two compare constants.  Directions,
 * in preference order:
 *   1. Read tools/gcc-2.7.2/loop.c `scan_loop`/`move_movables` and find the
 *      condition that suppresses hoisting for these two `set (reg) (const)`
 *      movables (the `maybe_never` flag after a conditional jump, the
 *      `threshold` savings test, `m->savings`).  Then find the C statement
 *      placement that puts each compare in a position where the constant is
 *      not a hoistable movable.
 *   2. Shape the loop so the constants are not plain `set reg const`
 *      movables (e.g. an unsigned-compare / different compare spelling for
 *      the 0x3C0000 threshold that folds into the existing `slt` operand
 *      chain, and a bit test for 0x1000000 that combine keeps inline).
 *   3. The variant in this file already fixed the exit structure (single
 *      shared `ret` + `while (a0 == 0)` back-edge, matching target's
 *      converge-at-.L80081CFC tail); a plain `do {...} while (1);` with
 *      inline `return`s measured 30/99 — very slightly worse — so keep the
 *      shared-exit shape when re-probing.
 * A `do { ... } while (0);` wrapper around the loop BODY would also emit the
 * notes without a real back-edge, but that is the strictly-gated sanctioned
 * exception ([[do-while-zero-exception]]) and must not be used until the
 * ordinary levers above are measured dead.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    do {
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    tslTm2LoadImage_2(&D_800161B8);
    {
        s32 arg4, arg5;
        arg4 = tbl_125c[idx_1494[0]];
        arg5 = tbl_125c[idx_1494[1]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
    }
    cdrom_ClearIrq();
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        ret = -1;
        break;
    }
    if (!(*D_800A14C0 & 0x1000000)) {
        ret = 0;
        break;
    }
    ret = 1;
    } while (a0 == 0);
    return ret;
}
