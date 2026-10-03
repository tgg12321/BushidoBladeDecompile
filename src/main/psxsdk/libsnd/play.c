/* PsyQ 4.0 LIBSND PLAY: _SsSndPlay. .text 0x80084948..0x80084974, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern void _SsSeqPlay(s16, s16);

void _SsSndPlay(s16 a0, s16 a1) {
    _SsSeqPlay(a0, a1);
}
