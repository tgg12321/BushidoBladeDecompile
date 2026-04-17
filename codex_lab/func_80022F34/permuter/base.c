#include "common.h"

extern u8 D_80101EC8;
extern s16 D_800A38DC;
extern s16 D_80102778;
extern u8 D_80102782;
extern s32 D_801027BC;
extern void func_80055138(s32, s32, s32);

void func_80022F34(void) {
    register s32 i asm("s0");
    register s32 offset asm("s1");
    register s16 *vals asm("s2");

    i = 0;
    vals = &D_80102778;
    offset = 0;
loop:
    {
        u8 *entry = (u8 *)&D_80101EC8 + offset;
        s16 mode;

        if (*(s16 *)(entry + 6) == 0) {
            goto next;
        }

        mode = D_800A38DC;
        if (mode >= 3) {
            if (mode == 3) {
                goto do_call;
            }
            goto use_vals;
        }

        if (mode > 0) {
            goto use_vals;
        }

        if (mode == 0) {
            *(s16 *)(entry + 8) = (s16)(*(&D_80102782 + i) << 4);
            goto do_call;
        }

use_vals:
        *(s16 *)(entry + 8) = *vals;

do_call:
        func_80055138(
            i,
            *(&D_801027BC + (*(s16 *)(entry + 0x4A) * 5)),
            *(&D_801027BC + (*(s16 *)(*(s32 *)entry + 0x4A) * 5))
        );
    }

next:
    vals++;
    i++;
    offset += 0x44C;
    if (i < 2) {
        goto loop;
    }
}
