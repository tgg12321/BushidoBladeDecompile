/* REJECTED — func_800645B0.  Banked by the session-6 (forensics) grind.
 *
 * WHY IT IS DEAD (measured, sweep18 — tmp/grind/func_800645B0/s5/sweep18.py):
 * Session 4's frontier item 1 proposed defeating optabs.c's commutative swap on
 * `idx = idx2 + idx;` by making GCC expand the sum with `target == 0` instead of
 * respelling the expression.  Reading expr.c's store_expr (expr.c:2692-2830)
 * enumerates every path for a scalar local, and exactly ONE of them passes
 * NULL_RTX down (expr.c:2793, the SUBREG_PROMOTED_VAR_P / narrower-than-word
 * destination).  Every C spelling of that — declaring the destination s16, u16,
 * s8 or u8, on either chassis, with either or both operands narrowed — pays for
 * the truncate/extend store_expr then forces, and lands at 79-81 instructions
 * against a 78-instruction target.  It can therefore never reach distance 0
 * regardless of register assignment.
 *
 *   variant (SB chassis unless noted)                 score / insns
 *   SB control  s32 idx                                   1 / 78
 *   CA control  s32 idx, sum in `wid`                     3 / 78
 *   NA  s16 idx                                           9 / 81
 *   NB  u16 idx                                           7 / 80
 *   NC  s8  idx                                           9 / 81
 *   ND  u8  idx                                           7 / 80
 *   PA  s16 idx on the CA chassis                         5 / 80
 *   PB  u8  idx on the CA chassis                         3 / 79
 *   QA  s16 idx2 only                                     3 / 79
 *   QB  u8  idx2 only                                     2 / 79
 *   RA/RB/RC/RD  BOTH idx and idx2 narrowed (s16/u16/u8/s8)
 *                                                16 / 81, 7 / 79, 7 / 79, 16 / 81
 *
 * The dump tmp/grind/func_800645B0/s5/dump_NA/f_rtl.txt confirms the mechanism
 * fired (the sum is expanded into a fresh pseudo, insn 72 -> reg:SI 93, then
 * copied into idx at insn 74) — the axis is dead on COST, not on mechanism.
 *
 * The body below is variant NB, the cheapest of the family (7 / 80).
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    u16 idx;   /* <-- the narrow destination; store_expr then forces a truncate */
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
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
