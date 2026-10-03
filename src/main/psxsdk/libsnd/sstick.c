/* PsyQ 4.0 LIBSND SSTICK: SsSetTickMode. .text 0x80085544..0x800856B0, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsSetTickMode(s32 arg) {
    s32 mode;
    s32 v;

    mode = GetVideoMode();

    if (arg & 0x1000) {
        _snd_seq_tick_env.unk4 = 1;
        _snd_seq_tick_env.unk0 = arg & 0xFFF;
    } else {
        _snd_seq_tick_env.unk4 = 0;
        _snd_seq_tick_env.unk0 = arg;
    }

    v = _snd_seq_tick_env.unk0;
    if (v >= 6) goto big_v;
    switch (v) {
    case 4: {
        s32 t = 50;
        VBLANK_MINUS = t;
        if (mode == 1) _snd_seq_tick_env.unk0 = 5;
        else _snd_seq_tick_env.unk0 = t;
        break;
    }
    case 1: {
        s32 t = 60;
        VBLANK_MINUS = t;
        if (mode == 0) _snd_seq_tick_env.unk0 = 5;
        else _snd_seq_tick_env.unk0 = t;
        break;
    }
    case 3:
        VBLANK_MINUS = 120;
        break;
    case 2:
        VBLANK_MINUS = 240;
        break;
    case 5:
        if (mode == 0) VBLANK_MINUS = 60;
        else if (mode == 1) VBLANK_MINUS = 50;
        else VBLANK_MINUS = 60;
        break;
    case 0:
        if (mode == 0) VBLANK_MINUS = 60;
        else if (mode == 1) VBLANK_MINUS = 50;
        else VBLANK_MINUS = 60;
        break;
    default:
        VBLANK_MINUS = 60;
        break;
    }
    return;
big_v:
    VBLANK_MINUS = v;
}
