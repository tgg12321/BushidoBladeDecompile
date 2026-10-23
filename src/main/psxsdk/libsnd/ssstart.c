/* PsyQ 4.0 LIBSND SSSTART: _SsStart, SsStart, SsStart2, _SsTrapIntrVSync and
 * _SsSeqCalledTbyT_1per2. .text 0x80083C34..0x80083F6C, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"
#include <psxsdk/libapi.h>
#include <psxsdk/libetc.h>

extern s32 InterruptCallback(s32, s32);
static void _SsTrapIntrVSync(void);
static void _SsSeqCalledTbyT_1per2(void);
extern void SsSeqCalledTbyT(void);

/* SSSTART .data (0x800A26CC..0x800A26E4, the module's 24-byte .data in
 * LIBSND.LIB, XDEF _snd_seq_tick_env): tick mode 60, tick-mode flag 1, the
 * tick handler SsSeqCalledTbyT, and the interrupt slot 0x7F (none). */
SndSeqTickEnv _snd_seq_tick_env = {
    60, 1, (s32)SsSeqCalledTbyT, 0, 0, 0, 0x7F, 0, 0};

/* BB2's 4.0 rev uses 0x7F for the case-0 sentinel where SOTN's uses 0xFF. */
void _SsStart(s32 arg0) {
    u16 rcnt_target;
    u32 rcnt_spec;

    s32 wait = 1000;
    while (--wait >= 0) {
    }

    _snd_seq_tick_env.unk16 = 0;
    _snd_seq_tick_env.unk18 = 6;
    _snd_seq_tick_env.unk17 = 0;
    _snd_seq_tick_env.unk12 = 0;
    rcnt_spec = 0xF2000002;
    rcnt_target = 0x44E8;
    switch (_snd_seq_tick_env.unk0) {
    case 0:
        _snd_seq_tick_env.unk18 = 0x7F;
        return;

    case 5:
        _snd_seq_tick_env.unk18 = 0;
        if (arg0 == 0) {
            _snd_seq_tick_env.unk16 = 1;
        } else {
            rcnt_spec = 0xF2000003;
            rcnt_target = 1;
        }
        break;

    case 3:
        rcnt_target = 0x89D0;
        break;

    case 2:
        break;

    default:
        if (_snd_seq_tick_env.unk4 == 0) {
            if (_snd_seq_tick_env.unk0 < 0x46) {
                rcnt_target = 0x204CC0 / _snd_seq_tick_env.unk0;
                _snd_seq_tick_env.unk17++;
            } else {
                rcnt_target = 0x409980 / _snd_seq_tick_env.unk0;
            }
        } else {
            return;
        }
        break;
    }

    if (_snd_seq_tick_env.unk16 != 0) {
        EnterCriticalSection();
        VSyncCallback(_snd_seq_tick_env.unk8);
    } else {
        s32 de;
        s32 a1_val;
        EnterCriticalSection();
        ResetRCnt(rcnt_spec);
        SetRCnt(rcnt_spec, rcnt_target, 0x1000);
        de = _snd_seq_tick_env.unk18;
        if (de == 0) {
            s32 ret = InterruptCallback(0, 0);
            de = _snd_seq_tick_env.unk18;
            a1_val = (s32)&_SsTrapIntrVSync;
            _snd_seq_tick_env.unk12 = ret;
        } else {
            a1_val = (s32)&_SsSeqCalledTbyT_1per2;
            if (_snd_seq_tick_env.unk17 == 0) {
                a1_val = _snd_seq_tick_env.unk8;
            }
        }
        InterruptCallback(de, a1_val);
    }
    ExitCriticalSection();
}

void SsStart(void) { _SsStart(1); }

static void SsStart2(void) { _SsStart(0); }

static void _SsTrapIntrVSync(void) {
    if (_snd_seq_tick_env.unk12 != 0) {
        ((void (*)(void))_snd_seq_tick_env.unk12)();
    }
    ((void (*)(void))_snd_seq_tick_env.unk8)();
}

/* Runs the tick handler (unk8) on every other call: reads the half-rate
 * toggle through `env`, flips it and stores it back (pointer read-modify-write,
 * pointer-alias-fake-exception). */
static void _SsSeqCalledTbyT_1per2(void) {
    SndSeqTickEnv *env = &_snd_seq_tick_env;

    if (env->unk20 == 0) {
        env->unk20 = 1;
    } else {
        env->unk20 = 0;
        ((void (*)(void))env->unk8)();
    }
}
