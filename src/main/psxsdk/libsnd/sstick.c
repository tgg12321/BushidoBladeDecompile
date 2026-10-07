/* PsyQ 4.0 LIBSND SSTICK: SsSetTickMode. .text 0x80085544..0x800856B0, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. C
 * ref: sotn-decomp src/main/psxsdk/libsnd/sstick.c. */
#include "common.h"
#include "libsnd_i.h"

void SsSetTickMode(s32 arg) {
    s32 mode;

    mode = GetVideoMode();
    if (arg & 0x1000) {
        _snd_seq_tick_env.unk4 = 1;
        _snd_seq_tick_env.unk0 = arg & 0xFFF;
    } else {
        _snd_seq_tick_env.unk4 = 0;
        _snd_seq_tick_env.unk0 = arg;
    }
    if (_snd_seq_tick_env.unk0 < 6) {
        switch (_snd_seq_tick_env.unk0) {
        case 4:
            VBLANK_MINUS = 50;
            if (mode != 1) {
                _snd_seq_tick_env.unk0 = 50;
            } else {
                _snd_seq_tick_env.unk0 = 5;
            }
            return;
        case 1:
            VBLANK_MINUS = 60;
            if (mode == 0) {
                _snd_seq_tick_env.unk0 = 5;
            } else {
                _snd_seq_tick_env.unk0 = 60;
            }
            return;
        case 3:
            VBLANK_MINUS = 120;
            return;
        case 2:
            VBLANK_MINUS = 240;
            return;
        case 5:
            if (mode == 0) {
                VBLANK_MINUS = 60;
            } else if (mode == 1) {
                VBLANK_MINUS = 50;
            } else {
                VBLANK_MINUS = 60;
            }
            break;
        case 0:
            if (mode == 0) {
                VBLANK_MINUS = 60;
            } else if (mode == 1) {
                VBLANK_MINUS = 50;
            } else {
                VBLANK_MINUS = 60;
            }
            return;
        default:
            VBLANK_MINUS = 60;
            return;
        }
    } else {
        VBLANK_MINUS = _snd_seq_tick_env.unk0;
    }
}
