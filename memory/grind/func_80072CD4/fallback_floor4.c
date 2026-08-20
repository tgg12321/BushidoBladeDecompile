/* func_80072CD4 — best CLEAN (reviewer-passable) form: sandbox --disable all = 4, build_insns 79 == target.
 * RE-MEASURED s5 (2026-08-20) on the POST-MIGRATION chassis (asm-until-matched, src carries
 * INCLUDE_ASM; this body applied over it reproduces score 4 / 79 insns exactly). The s1-s4 floor is
 * therefore chassis-current, not a stale pre-migration number. Symbol names updated to the current
 * naming wave: SetPolyG4 / SetSemiTrans / AddPrim (the pre-naming header said initPolyG4 /
 * gpu_SetSemiTransp / ot_Link — those identifiers no longer exist in src/text1b.c).
 *
 * Pure C: one local `int fc_const` (named 0xFC constant, mirrors COMPLETED-C sibling func_80072BC4).
 * NO asm/pins/volatile/barriers/do-while/dead-stores/unused-decls/duplication. Every store is a real
 * GameObj field write; the per-arm @0xE writes (0x32 vs 0x46) are genuinely different per branch.
 *
 * s2 progress (clean floor 18 -> 4):
 *   18 -> 9  : write @0xE per-arm (its real branch value) instead of via a cross-block temp in the merge.
 *              The value stops sharing a live range with the $v0 byte-store constants, so RA matches
 *              target (fc_const->$v1, @0xE-value->$v0); cross-jump merges the identical `sb v0,0xE` to
 *              the merge point (li stays in each arm delay slot). Closes the 1-insn gap (78->79).
 *    9 -> 4  : order the arg0>=4 branch stores as target (@4 first -> outer-beqz delay li v0,0x10;
 *              @5,@6 next -> multi-use v1/a0 consts hoisted; then @0xC).
 *
 * REMAINING 4 (objdump-confirmed again in s5, tmp/grind/func_80072CD4/s5/base.dis): the whole function
 * is byte-identical to target through both inner arms — including the THEN arm's `j` + `li v0,0x32`
 * delay slot and the ELSE arm's trailing `li v0,0x46`. The ONLY divergence is the merge block's store
 * ORDER: target emits `sb v1,4; sb v1,0xC; sb v0,0xE` at the merge head, ours emits `sb v0,0xE` first
 * (jump2 splices the cross-jumped common tail at the join label) and defers `sb v1,4; sb v1,0xC` to the
 * block tail (sched2 gives them no launch pairing — their $v1 datum is set in a predecessor block).
 *
 * WHY NO CLEAN 0 EXISTS (5 sessions of measurement, all banked):
 *   - cross-block var_v0 (target's own source shape) = 13/78: sched1 hoists the lone constant `li` to
 *     the arm top (no in-arm consumer -> no 7f000001 launch boost -> loses the potential_hazard tiebreak,
 *     sched.c:2683-2699), so var_v0 goes live across the $v0 byte constants and RA rotates it to $v1.
 *   - the merge-block source order is exhaustively searched (s4b directed PERM_LINESWAP, 15.8k iters,
 *     zero finds below base) and both random-permuter chassis (s4, ~11k iters) surface only cheats.
 *   - s5 killed the last two never-run levers: the 2026-06-14 WIP "Lever A" (route the arm byte
 *     constants 0xC3/0x1E/0xC8 through a shared local so they vacate $v0) is INERT on this chassis (4)
 *     and only reshuffles the diff on the cross-block chassis (11/78 — it cross-jumps `sb v0,0xD` to the
 *     merge head instead, losing an insn, and the RA rotation persists); hoisting the @4/@0xC stores
 *     above the inner `if` = 8/78; a base-pointer-local spelling (`u8 *p = (u8 *)arg1; p[N] = ...`) = 17/82.
 *   - the only sandbox-0 form ever found duplicates @4/@0xC into both arms for a store-SCHEDULE effect
 *     (jump2 deletes the second copy); Judge-FAILed 2026-07-24 16:38 and refused by owner ruling
 *     2026-07-27 (docs/grind/decisions.md). Do NOT re-propose it.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int fc_const;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        fc_const = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 4) = fc_const;
        *(u8 *)((s32)(arg1) + 0xC) = fc_const;
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
