/* PsyQ 4.0 LIBGPU EXT: LoadTPage, LoadClut, LoadClut2, SetDefDrawEnv and SetDefDispEnv. .text
 * 0x8007A4D8..0x8007A788, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include <psxsdk/libgpu.h>

u16 LoadTPage(s32 a0, s32 mode, s32 a2, s32 a3, s32 texpage, s32 width, s32 clut) {
    s16 buf[4];
    buf[0] = a3;
    buf[3] = clut;
    buf[1] = texpage;
    switch (mode) {
    case 0:
        buf[2] = width / 4;
        break;
    case 1:
        buf[2] = width / 2;
        break;
    case 2:
        buf[2] = width;
        break;
    }
    LoadImage((s32)buf, a0);
    return GetTPage(mode, a2, a3, texpage) & 0xFFFF;
}

u16 LoadClut(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    buf[0] = a1;
    buf[1] = a2;
    buf[2] = 0x100;
    buf[3] = 1;
    LoadImage((s32)buf, a0);
    return GetClut(a1, a2);
}
u16 LoadClut2(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    buf[0] = a1;
    buf[1] = a2;
    buf[2] = 0x10;
    buf[3] = 1;
    LoadImage((s32)buf, a0);
    return GetClut(a1, a2);
}
s16 *SetDefDrawEnv(s16 *a0, s16 a1, s16 a2, s16 a3, s32 a4) {
    s32 ret;
    ret = GetVideoMode();
    a0[0] = a1;
    a0[1] = a2;
    a0[2] = a3;
    a0[6] = 0;
    a0[7] = 0;
    a0[8] = 0;
    a0[9] = 0;
    ((s8 *)a0)[0x19] = 0;
    ((s8 *)a0)[0x1A] = 0;
    ((s8 *)a0)[0x1B] = 0;
    ((s8 *)a0)[0x16] = 1;
    a0[3] = a4;
    if (ret) {
        ((s8 *)a0)[0x17] = (a4 < 0x121);
    } else {
        ((s8 *)a0)[0x17] = (a4 < 0x101);
    }
    a0[4] = a1;
    a0[5] = a2;
    a0[0xA] = 10;
    ((s8 *)a0)[0x18] = 0;
    return a0;
}

s16 *SetDefDispEnv(s16 *a0, s16 a1, s16 a2, s16 a3, s32 a4) {
    a0[0] = a1;
    a0[1] = a2;
    a0[2] = a3;
    a0[4] = 0;
    a0[5] = 0;
    a0[6] = 0;
    a0[7] = 0;
    ((s8 *)a0)[0x11] = 0;
    ((s8 *)a0)[0x10] = 0;
    ((s8 *)a0)[0x13] = 0;
    ((s8 *)a0)[0x12] = 0;
    a0[3] = a4;
    return a0;
}
