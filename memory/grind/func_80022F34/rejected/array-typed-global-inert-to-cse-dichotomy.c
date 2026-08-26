/* REJECTED (grind s7, 2026-08-26) — `extern s32 D_801027BC[];` + D_801027BC[idx*5]
 * (header type correction from use sites) instead of `extern s32 D_801027BC;` + &.
 * WHY DEAD: symbol TYPE is inert to the separation dichotomy. Array-typed in the
 * separated class (this file) = vars=8 / per-access x2 / strand=1, exactly base;
 * array-typed in the single-expression class (vO14) = vars=0 / la=1 / strand=0,
 * exactly vORIG. Confirms s2's vARR result and extends it to the vORIG class.
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
                s16 idx1 = *(s16 *)(a0 + 0x4A);
                s32 val1 = (D_801027BC)[idx1 * 5];
                a0 = *(u8 **)a0;
                {
                    s16 idx2 = *(s16 *)(a0 + 0x4A);
                    single_game_SetStatusUpData(i, val1, (D_801027BC)[idx2 * 5]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
