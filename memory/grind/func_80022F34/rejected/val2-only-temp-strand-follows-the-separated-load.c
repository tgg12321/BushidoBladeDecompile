/* REJECTED (grind s7, 2026-08-26) — name val2 ONLY (leave the first load inline).
 * WHY DEAD: vars=8, per-access x2, strand=1. Tests whether the strand is a
 * property of val1 specifically (s2/s6 framed it that way). It is not — the
 * strand follows whichever D_801027BC load is lifted into its own statement.
 * Together with vO2 (val1 named but defined AFTER the a0 reload: still strands)
 * this retires the "val1 placement / val1 lifetime" framing entirely: the
 * discriminator is statement separation, not which value or where it lives.
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

            switch (val) {
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

            {
                u8 *nxt = *(u8 **)a0;
                s16 idx1 = *(s16 *)(a0 + 0x4A);
                s16 idx2 = *(s16 *)(nxt + 0x4A);
                s32 val2 = (&D_801027BC)[idx2 * 5];
                single_game_SetStatusUpData(i, (&D_801027BC)[idx1 * 5], val2);
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
