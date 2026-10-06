/* PsyQ 4.0 LIBGPU EXT: LoadTPage, LoadClut, LoadClut2, SetDefDrawEnv and SetDefDispEnv. .text
 * 0x8007A4D8..0x8007A788, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include <psxsdk/libgpu.h>

u16 LoadTPage(u32 *pix, s32 tp, s32 abr, s32 x, s32 y, s32 w, s32 h) {
    RECT buf;
    buf.x = x;
    buf.h = h;
    buf.y = y;
    switch (tp) {
    case 0:
        buf.w = w / 4;
        break;
    case 1:
        buf.w = w / 2;
        break;
    case 2:
        buf.w = w;
        break;
    }
    LoadImage(&buf, pix);
    return GetTPage(tp, abr, x, y);
}

u16 LoadClut(u32 *a0, s32 a1, s32 a2) {
    RECT buf;
    buf.x = a1;
    buf.y = a2;
    buf.w = 0x100;
    buf.h = 1;
    LoadImage(&buf, a0);
    return GetClut(a1, a2);
}
u16 LoadClut2(u32 *a0, s32 a1, s32 a2) {
    RECT buf;
    buf.x = a1;
    buf.y = a2;
    buf.w = 0x10;
    buf.h = 1;
    LoadImage(&buf, a0);
    return GetClut(a1, a2);
}
DRAWENV *SetDefDrawEnv(DRAWENV *env, s32 x, s32 y, s32 w, s32 h) {
    s32 ret;
    ret = GetVideoMode();
    env->clip.x = x;
    env->clip.y = y;
    env->clip.w = w;
    env->tw.x = 0;
    env->tw.y = 0;
    env->tw.w = 0;
    env->tw.h = 0;
    env->r0 = 0;
    env->g0 = 0;
    env->b0 = 0;
    env->dtd = 1;
    env->clip.h = h;
    if (ret) {
        env->dfe = (h < 0x121);
    } else {
        env->dfe = (h < 0x101);
    }
    env->ofs[0] = x;
    env->ofs[1] = y;
    env->tpage = 10;
    env->isbg = 0;
    return env;
}

DISPENV *SetDefDispEnv(DISPENV *env, s32 x, s32 y, s32 w, s32 h) {
    env->disp.x = x;
    env->disp.y = y;
    env->disp.w = w;
    env->screen.x = 0;
    env->screen.y = 0;
    env->screen.w = 0;
    env->screen.h = 0;
    env->isrgb24 = 0;
    env->isinter = 0;
    env->pad1 = 0;
    env->pad0 = 0;
    env->disp.h = h;
    return env;
}
