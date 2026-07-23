#include "_prelude.h"
/* vSECOND (forensic): anchor reg100's REG_DEAD note on a REAL insn by giving
 * val1 a second, semantically-real consumer before the a0 reload — pass val1
 * as the FIRST call arg too? No: keep semantics. Instead compute idx1's access
 * through a pointer q that is itself used after the switch, so the address
 * register survives as a referenced pseudo the note can attach to (line 10735
 * reg_referenced_p(i3) / line 10806 path). Tests whether a live address pointer
 * that is ALSO dereferenced a second time keeps combine from stranding it. */
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
            s32 *q;

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
                q = &(&D_801027BC)[idx1 * 5];
                {
                    s32 val1 = *q;
                    a0 = *(u8 **)a0;
                    {
                        s16 idx2 = *(s16 *)(a0 + 0x4A);
                        single_game_SetStatusUpData(i, val1, (&D_801027BC)[idx2 * 5]);
                    }
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
