/* PsyQ 4.0 LIBETC VMODE: SetVideoMode, GetVideoMode (SOTN libetc/vmode.c).
 * .text 0x80083670..0x80083698, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libetc.h>

/* Declarations from the file this module was split from
 * (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
extern s32 video_mode;

s32 SetVideoMode(s32 a0) {
    s32 old = video_mode;
    video_mode = a0;
    return old;
}

s32 GetVideoMode(void) { return video_mode; }
