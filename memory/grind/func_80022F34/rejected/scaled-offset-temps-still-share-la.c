/* REJECTED (grind s8, escalation, 2026-08-26) — hoisting the SCALED offsets (idx*5) into temps instead of the loaded values keeps cse2 sharing the la
 * Measured: vars=0, sp -32, la=1, per-access=0 (vORIG class: frame right, body wrong)
 * Harness: tmp/grind/func_80022F34/s8/probe.sh (cc1 -O2 -G0 -mel -da).
 */
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;

extern s16 D_800A38DC;
extern u8 D_80101EC8;
extern s16 D_80102778;
extern u8 D_80102782;
extern s32 D_801027BC;

extern void single_game_SetStatusUpData(s32, s32, s32);

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
                s32 o1 = *(s16 *)(a0 + 0x4A) * 5;
                a0 = *(u8 **)a0;
                {
                    s32 o2 = *(s16 *)(a0 + 0x4A) * 5;
                    single_game_SetStatusUpData(i, (&D_801027BC)[o1],
                                                (&D_801027BC)[o2]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
