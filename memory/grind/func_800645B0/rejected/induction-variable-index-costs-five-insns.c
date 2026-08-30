/* REJECTED — func_800645B0, grind session 14 (escalation/disposition, 2026-08-30).
 *
 * THE FORM (frontier item 1 of the s13 ledger, verbatim: "re-derive the inner
 * loop with the slot index as a real induction variable that is both
 * initialised and updated inside the loop body, not `idx = i + j` recomputed
 * from two counters, apply the WD fresh-destination sum on top").  This is
 * variant IV1: the slot index is initialised once per group (`idx = i;`) and
 * STRENGTH-REDUCED at the inner loop's bottom (`idx += 1;`), so
 * reg_n_sets[idx] == 2 by construction with no dead, staged or self-assigning
 * store anywhere; the *3 sum takes its own fresh destination `wid`, which is
 * what optabs.c:398-421 requires for the target's `addu $s0,$s1,$s0` operand
 * order (H58/H59).  On paper this is the exact shape the frontier asked for.
 *
 * MEASURED (sandbox func_800645B0 --disable all, this session's tree):
 *   IV1 (this file)                       16 / 78 at 83 build insns
 *   IV2 (index-only inner loop,
 *        `for (idx = i; idx < i + 4; idx++)`, j deleted)
 *                                         25 / 78 at 89 build insns
 *   SB control (the standing floor)        1 / 78 at 78 build insns
 *
 * WHY IT IS DEAD — structural, not a tie-break.  The TARGET recomputes the
 * slot index from the two counters with a REGISTER-REGISTER add, twice:
 * `addu $s0,$s3,$a0` peeled above the inner-loop label at 0x800645DC and again
 * in the back-edge delay slot at 0x800646B4 (asm/funcs/func_800645B0.s).  An
 * induction-variable index emits `addiu $s0,$s0,0x1` instead and must still
 * carry `j` (or a computed `i + 4` bound) for the trip count, so the group
 * prologue and the loop latch both grow: +5 insns for IV1, +11 for IV2.  The
 * data model is therefore falsified by the target's own instruction stream —
 * the original C computes `i + j` inside the inner loop, and no re-derivation
 * of the index as a maintained induction variable can reproduce 78 insns.
 * (Distinct from the banked IA/OA "maintained index" chassis, whose tail
 * update is a recomputed `idx = i + j;` — those stay at 1/78 and 3/79.)
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        idx = i;
        for (j = 0; j < 4; j++) {
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
            idx += 1;
        }
    }
    return 1;
}
