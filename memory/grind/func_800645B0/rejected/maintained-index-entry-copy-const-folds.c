/* REJECTED (at 1 point — a SIBLING of the current floor, not worse) —
 * func_800645B0, grind session 3 (2026-08-12).
 *
 * THE FORM: maintain the slot index instead of recomputing it at the loop top.
 * `idx` is set once before the inner loop and again at the loop's tail, so it
 * has TWO genuinely different, non-foldable sets.  That frees the 12-byte-
 * stride sum to take its own destination `wid`, which is what the target's
 * `addu $s0,$s1,$s0` requires (optabs.c expand_binop only swaps a commutative
 * pair when the expansion target IS op1).  This is the FIRST form measured
 * that satisfies BOTH of the constraints the session-2 ledger called mutually
 * exclusive.
 *
 * MEASURED (sandbox --disable all, target 78 insns):
 *   IA  entry `idx = i + j;`, tail `idx = i + j;`, sum in `wid`     1 / 78
 *   IC  entry `idx = i;`                                            1 / 78
 *   IE  IA + byte offset folded back into `wid`                     1 / 78
 *   IB  IA but the sum written back into `idx`                      2 / 78
 *   RB  IA with the entry copy spelled `idx = j + i;`               1 / 78
 *   ID  tail written `idx += 1;`                                   19 / 83
 *   LA  `j` reset at the OUTER loop tail instead of its head        5 / 79
 *
 * WHY IT IS DEAD AS A ROUTE TO ZERO: IA's single remaining unmasked diff is
 * instruction 11 —
 *      target: addu s0,s3,a0        build: move s0,s3
 * — the LOOP-ENTRY copy.  GCC knows `j == 0` there (the assignment is in the
 * same basic block) and constant-folds `i + j` to a copy.  In the target that
 * insn is NOT source-level at all: asm/funcs/func_800645B0.s contains
 * `addu $s0,$s3,$a0` TWICE, once immediately before .L800645E0 and once in the
 * back-edge delay slot at .L800646B0, which is the signature of reorg.c
 * stealing the loop-top insn into the delay slot and duplicating it on the
 * loop-entry path.  So the original C recomputes the index at the loop TOP and
 * any hand-written entry copy is folded.
 *
 * Attempts to defeat the fold: LA (reset `j` at the outer loop tail so the
 * entry copy cannot see a constant) costs a real instruction — 5 / 79.
 * RB (operand order) is byte-identical.
 *
 * Sweeps: tmp/grind/func_800645B0/s3/sweep13.py (IA..IE),
 *         sweep14.py (LA/LB), sweep16.py (RA/RB).
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        idx = i + j;          /* <- emitted `move s0,s3`; target has `addu s0,s3,a0` */
        do {
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;   /* correct operand order: addu s0,s1,s0 */
                /* ... three s32 stores at (wid << 2), s16 store at idx2 ... */
                val = D_800A3444;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
            idx = i + j;      /* the second, non-foldable set; lands in the
                                 back-edge delay slot exactly as target */
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
