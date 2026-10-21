/* PsyQ 4.0 LIBETC VMODE: SetVideoMode, GetVideoMode (SOTN libetc/vmode.c).
 * .text 0x80083670..0x80083698, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libetc.h>

s32 video_mode = 0; /* 0 = NTSC, 1 = PAL */

s32 SetVideoMode(s32 a0) {
    s32 old = video_mode;
    video_mode = a0;
    return old;
}

s32 GetVideoMode(void) { return video_mode; }
