/* REJECTED -- session s17 (2026-09-02, rederive modality).  Score 12 / 78 at 78
 * build insns.  Superseded anyway: the session closed the function at 0 with a
 * different form (see memory/grind/func_800645B0/candidate.c).
 *
 * THE IDEA.  The 16-session lock was: the *3 sum needs a destination pseudo
 * distinct from `idx` (expand_binop, tools/gcc-2.7.2/optabs.c:398-421, swaps a
 * commutative pair whenever `target == op1`), but a distinct destination leaves
 * `idx` single-set, which hands the inner-loop head to the const-1 `li` via
 * sched.c's birthing_insn_p max-priority lift.  This variant tries to get both
 * halves from ONE ordinary-C statement by folding the sum and the word->byte
 * scale together:
 *
 *     idx = (idx2 + idx) << 2;
 *
 * The hope was that expand_expr would hand the inner PLUS a subtarget of 0
 * (making `target != op1`, hence no swap) while the outer shift writes back
 * into `idx`, giving reg_n_sets[idx] == 2 and denying the lift.
 *
 * WHY IT IS DEAD.  It works exactly as predicted at the RTL level and is
 * NEVERTHELESS the "k" chassis verbatim: 78 insns, EVERY OPCODE AND EVERY
 * POSITION EXACT, twelve register-name differences.  `tmp/grind/func_800645B0/
 * s17/diff.py` output:
 *     11 OURS addu s1,s3,a0        TGT addu s0,s3,a0
 *     14 OURS sllv s2,v1,s1        TGT sllv s2,v1,s0
 *     19 OURS sll  s0,s1,0x1       TGT sll  s1,s0,0x1
 *     20 OURS addu v1,s0,s1        TGT addu s0,s1,s0
 *     21 OURS lw   a0,0(gp)        TGT lw   v1,0(gp)
 *     22 OURS sll  s1,v1,0x2       TGT sll  s0,s0,0x2
 *     23 OURS lw   v1,0(a0)        TGT lw   v1,0(v1)
 *     28/39/50 OURS addu at,at,s1  TGT addu at,at,s0
 *     57 OURS addu at,at,s0        TGT addu at,at,s1
 *     65 OURS addu s1,s3,a0        TGT addu s0,s3,a0
 * i.e. `idx` and `idx2` swap their callee-saved seats ($s1 / $s0) and the sum
 * lands in the caller-saved $v1 instead of coalescing into idx's seat.  This is
 * the same 12/78 permutation session s16 banked as
 * rejected/idx-second-set-byte-offset-seat-permutation-12of78.c from the
 * two-statement spelling (`wid = idx2 + idx; idx = wid << 2;`), and s16's
 * `inverse.py global` returned NEGATIVE for its full goal
 * {"74":16,"78":16,"73":3} and for every narrowed sub-goal.  So the compound
 * one-statement spelling is byte-equivalent to the two-statement one: folding
 * the scale into the sum statement is NOT a new lever on local-alloc.
 *
 * Generalisation worth keeping: making `idx` multi-set by giving it the BYTE
 * OFFSET costs 12; giving it the masked random (`idx = last & 7;`, the "h"
 * form) or the occupancy OR result costs 2.  All of them cost something,
 * because every value written into `idx` inherits idx's callee-saved seat.
 * The form that actually closed the function does not add a second write to
 * `idx` at all.
 */
#if 0
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = (idx2 + idx) << 2;
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
#endif
