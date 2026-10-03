/* PsyQ 4.0 LIBETC VSYNC: VSync and v_wait (SOTN libetc/vsync.c). .text 0x800828CC..0x80082AB0, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
extern volatile s32 Vcount;

extern volatile s32 *g_vsync_gpu_stat_reg;
extern volatile s32 *g_vsync_rcnt1_count_reg;
extern s32 Hcount;
extern s32 D_800A151C;
void v_wait(s32 a0, s32 a1);

s32 VSync(s32 a0) {
    s32 s0_val;
    s32 s1_val;

    s0_val = *g_vsync_gpu_stat_reg;
    s1_val = (*g_vsync_rcnt1_count_reg - Hcount) & 0xFFFF;

    if (a0 < 0) {
        return Vcount;
    }
    if (a0 == 1) {
        return s1_val;
    }

    {
        s32 frame;
        s32 count;

        if (a0 > 0) {
            s32 base = D_800A151C - 1;
            frame = base + a0;
        } else {
            frame = D_800A151C;
        }
        count = 0;
        if (a0 > 0) {
            count = a0 - 1;
        }
        v_wait(frame, count);
    }

    s0_val = *g_vsync_gpu_stat_reg;
    v_wait(Vcount + 1, 1);

    if (s0_val & 0x400000) {
        volatile s32 *ptr = g_vsync_gpu_stat_reg;
        if ((s32)(s0_val ^ *ptr) >= 0) {
            do {
            } while (!((s0_val ^ *ptr) & 0x80000000));
        }
    }

    D_800A151C = Vcount;
    Hcount = *g_vsync_rcnt1_count_reg;

    return s1_val;
}

extern s32 D_80016318;
extern void puts(void *);
extern void ChangeClearPAD(s32);
extern void ChangeClearRCnt(s32, s32);
/* PsyQ 4.0 LIBETC VSYNC: v_wait (static) — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libetc/vsync.c.
   FAKE(partial-use volatile array, Ruling 3): only [0] is
   referenced — SOTN ships the identical `volatile s32 timeout[2]` shape;
   original author idiom. */
void v_wait(s32 a0, s32 a1) {
    volatile s32 timeout[2];

    timeout[0] = a1 << 0xF;
    while (Vcount < a0) {
        if (timeout[0]-- == 0) {
            puts(&D_80016318);
            ChangeClearPAD(0);
            ChangeClearRCnt(3, 0);
            return;
        }
    }
}
