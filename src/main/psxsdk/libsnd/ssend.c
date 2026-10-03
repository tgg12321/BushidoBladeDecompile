/* PsyQ 4.0 LIBSND SSEND: SsEnd (SOTN libsnd/ssend.c). .text 0x80083954..0x80083A18, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
void VSyncCallback(s32 a0);
void InterruptCallback(void);

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

void SsEnd(void) {
    if (_snd_seq_tick_env.unk4 != 0) {
        return;
    }
    _snd_seq_tick_env.unk17 = 0;
    if (_snd_seq_tick_env.unk18 == 0x7F) {
        return;
    }
    EnterCriticalSection();
    if (_snd_seq_tick_env.unk16 != 0) {
        VSyncCallback(0);
        _snd_seq_tick_env.unk16 = 0;
    } else if (_snd_seq_tick_env.unk18 == 0) {
        ((void (*)(s32, s32))InterruptCallback)(0, _snd_seq_tick_env.unk12);
        _snd_seq_tick_env.unk12 = 0;
    } else {
        ((void (*)(s32, s32))InterruptCallback)(6, 0);
    }
    ExitCriticalSection();
    _snd_seq_tick_env.unk18 = 0x7F;
}
