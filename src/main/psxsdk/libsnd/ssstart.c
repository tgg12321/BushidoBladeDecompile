/* PsyQ 4.0 LIBSND SSSTART: _SsStart, SsStart, SsStart2, _SsTrapIntrVSync and
 * _SsSeqCalledTbyT_1per2. .text 0x80083C34..0x80083F6C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"
#include <psxsdk/libapi.h>
#include <psxsdk/libetc.h>

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 EnterCriticalSection(void);

extern s32 InterruptCallback(s32, s32);
extern void ResetRCnt(s32);
extern void SetRCnt(s32, s32, s32);
static void _SsTrapIntrVSync(void); /* _SsTrapIntrVSync (ssstart.c static) */
static void _SsSeqCalledTbyT_1per2(void); /* _SsSeqCalledTbyT_1per2 (ssstart.c static) */

/* PsyQ 4.0 LIBSND ssstart: _SsStart (SndSeqTickEnv in libsnd_i.h) —
   verbatim-linked Sony object; C ref: sotn-decomp
   src/main/psxsdk/libsnd/ssstart.c (BB2's 4.0 rev uses 0x7F for the case-0
   sentinel where SOTN's rev uses 0xFF) */

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
/* PsyQ 4.0 LIBSND ssstart: SsStart / SsStart2 / _SsTrapIntrVSync /
   _SsSeqCalledTbyT_1per2 + sscall: SsSeqCalledTbyT — verbatim-linked Sony
   objects; C ref: sotn-decomp
   src/main/psxsdk/libsnd/{ssstart.c,sscall.c}. Only SsStart (=DispStuff)
   has a glabel: SsStart2 + the tick trampolines are statics inside the
   splat extent; SsSeqCalledTbyT is address-referenced only by the
   SndSeqTickEnv .data initializer (raw .word @0x800A26D4). */

void SsStart(void) {
    _SsStart(1);
}
static void SsStart2(void) {
    _SsStart(0);
}
static void _SsTrapIntrVSync(void) {
    if (_snd_seq_tick_env.unk12 != 0) {
        ((void (*)(void))_snd_seq_tick_env.unk12)();
    }
    ((void (*)(void))_snd_seq_tick_env.unk8)();
}
/* FAKE: the toggle's read goes through an inline accessor; the direct
   `_snd_seq_tick_env.unk20 == 0` read lets CSE share one base register
   across the three unk20 accesses (score 12, one insn short).
   SOTN: src/main/psxsdk/libsnd/ssstart.c:24 @db41b28 (same accessor, same
   function, ssstart.c:27). */
static inline s32 get20(void) { return _snd_seq_tick_env.unk20; }

static void _SsSeqCalledTbyT_1per2(void) {
    if (get20() == 0) {
        _snd_seq_tick_env.unk20 = 1;
    } else {
        _snd_seq_tick_env.unk20 = 0;
        ((void (*)(void))_snd_seq_tick_env.unk8)();
    }
}
