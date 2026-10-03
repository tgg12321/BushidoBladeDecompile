/* PsyQ 4.0 LIBETC INTR_VB: the VSync interrupt hooks (startIntrVSync, trapIntrVSync, setIntrVSync
 * and the module's memclr, sys_MemClear; SOTN libetc/intr_vb.c). .text 0x800832A0..0x800833C8, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
void InterruptCallback(void);

extern s32 D_800A2614[8];
extern volatile s32 Vcount;
extern s32 *D_800A2638;

void trapIntrVSync(void);
void setIntrVSync(s32, s32);

s32 startIntrVSync(void) {
    *D_800A2638 = 0x107;
    Vcount = 0;
    sys_MemClear(&D_800A2614[0], 8);
    ((void (*)(s32, void *))InterruptCallback)(0, (void *)trapIntrVSync);
    return (s32)setIntrVSync;
}

void trapIntrVSync(void) {
    s32 i;
    s32 *p;

    ++Vcount;

    i = 0;
    p = &D_800A2614[0];
    for (; i < 8; i++) {
        s32 fp = *p;
        if (fp != 0) {
            ((void (*)(void))fp)();
        }
        p++;
    }
}

void setIntrVSync(s32 a0, s32 a1) {
    if (a1 != D_800A2614[a0]) {
        D_800A2614[a0] = a1;
    }
}

void sys_MemClear(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}
