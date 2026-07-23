#include "_prelude.h"
/* vNH (forensic): explicit byte-pointer address arithmetic for val1, to test
 * whether the address pseudo (reg100) still strands when the address is spelled
 * as a char* offset rather than an array index. If combine still folds it into
 * the a1 mem, the strand is spelling-invariant. */
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
                s32 val1 = *(s32 *)((u8 *)&D_801027BC + idx1 * 20);
                a0 = *(u8 **)a0;
                {
                    s16 idx2 = *(s16 *)(a0 + 0x4A);
                    single_game_SetStatusUpData(i, val1, *(s32 *)((u8 *)&D_801027BC + idx2 * 20));
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
