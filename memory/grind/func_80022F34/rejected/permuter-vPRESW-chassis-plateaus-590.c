/* func_80022F34 — REJECTED (grind s5, permuter, 2026-07-23)
 *
 * WHY DEAD: vPRESW chassis (val1 loaded BEFORE the switch) is the ONLY
 * structural form with BOTH target properties — per-access %hi/%lo AND vars=0
 * (no phantom frame). Its sole defect is scheduling: hoisting val1's def before
 * the switch merge forces val1 to live across the switch and pushes a0 off $a0
 * => the whole reload/arg chain reorders => sandbox 28 (banked s2:
 * val1-before-switch-per-access-vars0-but-reorder-28.c).
 *
 * s5 HYPOTHESIS: this reorder is PURE scheduling/register — exactly the
 * permuter's domain — so a directed permuter seeded from vPRESW (a chassis
 * s4 never used; s4 used base=174 and vH=1250, NEITHER of which had both
 * target properties) might recover the schedule while keeping per-access+vars=0.
 *
 * RESULT: KILLED. permuter_campaign vPRESW-schedule-recover chassis, -j8,
 * --stack-diffs, base_score 760. 13367 iterations. Best find = 590 (this class,
 * cosmetic new_var/do-while(0) mutations only — see tmp/grind/.../s5/best-590/).
 * The basin descends 760 -> 590 in the first ~30s then OSCILLATES 590-760
 * indefinitely; it NEVER approaches base's byte-perfect-body chassis (174),
 * let alone target (0). The permuter cannot undo vPRESW's val1-hoist reorder
 * without collapsing back to the late-val1 CSE/strand basin (base/vH), which
 * is precisely s2's coupling: moving val1's def back after the switch merge
 * restores the schedule but re-strands reg100 (vars=8) or shares the la base
 * (not per-access). There is no permuter-reachable middle.
 *
 * This EXTENDS s4's permuter kill (base 174 / vH 1250 chassis) to the third
 * and only remaining candidate chassis (vPRESW 760) — the one that already
 * had 2 of the 3 target properties. All three permuter chassis are now dead.
 * The per-access<->phantom<->schedule coupling is a fork-level cse2+combine
 * interaction with no C spelling any permuter mutation reaches.
 *
 * base (byte-perfect body + phantom, floor 11) remains the best form; see
 * ../candidate.c. src/code6cac.c unchanged this session.
 */
void func_80022F34(void) {
    s32 i = 0;
    u16 *tbl = (u16 *)&D_80102778;
    s32 offset = 0;
loop_22F34:
    {
        u8 *a0 = (u8 *)&D_80101EC8 + offset;
        if (*(s16 *)(a0 + 6) != 0) {
            s16 idx1 = *(s16 *)(a0 + 0x4A);
            s32 val1 = (&D_801027BC)[idx1 * 5];   /* HOISTED before switch */
            switch (D_800A38DC) {
                case 0:
                    *(s16 *)(a0 + 8) = (&D_80102782)[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(a0 + 8) = *tbl;
                    break;
                case 3:
                    break;
            }
            a0 = *(u8 **)a0;
            {
                s16 idx2 = *(s16 *)(a0 + 0x4A);
                single_game_SetStatusUpData(i, val1, (&D_801027BC)[idx2 * 5]);
            }
        }
        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
