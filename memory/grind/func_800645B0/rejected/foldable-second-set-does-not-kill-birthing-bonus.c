/* REJECTED — session 2 (structural), sweep 5.  Score 3 / 78 insns (no gain).
 *
 * WHY IT IS DEAD: a second assignment to `idx` only removes sched.c's
 * birthing_insn_p bonus if it SURVIVES to the scheduler as a real set.  Every
 * spelling whose second set is foldable is folded away before flow/sched
 * recomputes reg_n_sets, and the .sched dump is then byte-identical to the
 * single-set form:
 *
 *   ;; ready list at T-6: 31 (1) 28 (7f000001), now 28 31
 *
 * (verbatim in both tmp/grind/func_800645B0/s2/dump_A_base/f_sched.txt and
 * dump_U_half/f_sched.txt).  Four foldable spellings measured, all 3 / 78:
 *   T_split  `idx = i; idx += j;`            — combine re-merges the pair
 *   U_half   `idx = idx << 1;` after the stores, s16 store uses `idx`
 *            — CSE finds the value already in the halfword-offset pseudo,
 *              so the set degrades to a copy and copy-prop deletes it
 *   V_both   T_split + U_half
 *   W_late   `idx = idx2 + idx;` AFTER stores that compute the same sum
 *            inline — again a copy of the CSE temp
 * Sweep 11 (FA/FB/FC/FD: the halfword offset re-assigned into `idx` as
 * `idx << 1`, `idx + idx`, `idx * 2`) is the same kill: all 3 / 78.
 *
 * The variant below is U_half.  The lesson generalises: for a reg_n_sets
 * lever the second set must be a computation whose VALUE is not already
 * available in another live pseudo.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                *((s32 *)(((s32)(&D_800F0D78)) + (((idx << 1) + idx) << 2))) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (((idx << 1) + idx) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (((idx << 1) + idx) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                idx = idx << 1;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
