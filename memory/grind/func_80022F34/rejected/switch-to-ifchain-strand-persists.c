/* REJECTED (grind s7, 2026-08-26) — switch -> if/else-if chain.
 * WHY DEAD: this was the s6 frontier's headline "novel whole-function reshape"
 * lever ("eliminate the switch-merge label"). Measured: vars=8, per-access x2,
 * strand=1 — IDENTICAL to base. The jump table is gone and the case-merge label
 * is gone, and combine STILL emits the orphaned (use regN); the loop's own
 * CODE_LABEL is enough for distribute_notes to land on. Corroborated by vIF2
 * (reordered chain), vTERN (single store via a conditional expression) and vMIN
 * (the whole switch DELETED — diagnostic, semantics broken): all vars=8,
 * strand=1. The strand is NOT switch-dependent. Body is also 5 insns shorter
 * than base's byte-perfect switch, so this is strictly worse besides.
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *a0 = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(a0 + 6) != 0) {
            s32 val = D_800A38DC;

            if (val == 0) {
                *(s16 *)(a0 + 8) = (&D_80102782)[i] << 4;
            } else if (val != 3) {
                *(s16 *)(a0 + 8) = *tbl;
            }

            {
                s16 idx1 = *(s16 *)(a0 + 0x4A);
                s32 val1 = (&D_801027BC)[idx1 * 5];
                a0 = *(u8 **)a0;
                {
                    s16 idx2 = *(s16 *)(a0 + 0x4A);
                    single_game_SetStatusUpData(i, val1, (&D_801027BC)[idx2 * 5]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
