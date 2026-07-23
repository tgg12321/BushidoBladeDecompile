#include "_prelude.h"
/* vTAIL: wrap the switch in do{}while(0) so the switch-merge label lands
 * INSIDE that construct, separating it from the val1 sub-block. Goal: move
 * combine's stranded (use reg100) off the val1-block boundary. */
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

            do {
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
            } while (0);

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
