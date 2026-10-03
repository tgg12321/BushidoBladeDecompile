/* PsyQ 4.0 LIBSPU S_INI: _SpuInit and SpuStart. .text 0x800885CC..0x80088740, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "system.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_keystat;
extern s32 _spu_trans_mode;
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern s32 _spu_init(s32);
extern void _spu_FsetRXX(s32, u32, s32);
extern s32 _spu_rev_flag;
extern s32 _spu_rev_reserve_wa;
extern s32 _spu_rev_offsetaddr;
extern volatile s32 _spu_RQvoice; /* _spu_RQvoice — Ruling-4 grant (volatile_extern_allowlist.txt:44) */
extern volatile s32 _spu_RQmask;
extern volatile s32 _spu_env;
extern s32 _spu_AllocBlockNum;
extern s32 _spu_AllocLastNum;
extern s32 _spu_memList;
extern s32 _spu_rev_startaddr[]; /* _spu_rev_startaddr */
extern s32 _spu_transMode;
typedef struct {
    s16 left, right;
} SpuVolume;

/* Sony _spu_rev_attr — ONE struct (sotn libspu_internal.h:87 struct
   SpuRevAttr), base 0x800A2888: mode / depth L,R / delay / feedback. */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuRevAttr;
extern SpuRevAttr _spu_rev_attr; /* _spu_rev_attr */
extern u16 _spu_voice_centerNote[]; /* one entry per SPU voice (24) */

void _SpuInit(s32 arg0) {
    u16 *var_v0;
    s32 var_v1;
    s32 val;

    ResetCallback();
    _spu_init(arg0);
    val = 0xC000;
    if (arg0 == 0) {
        var_v1 = 0x17;
        var_v0 = &_spu_voice_centerNote[23];
        do {
            *var_v0 = val;
            var_v1 -= 1;
            var_v0 -= 1;
        } while (var_v1 >= 0);
    }
    SpuStart();
    _spu_rev_flag = 0;
    _spu_rev_reserve_wa = 0;
    _spu_rev_attr.mode = 0;
    _spu_rev_attr.depth.left = 0;
    _spu_rev_attr.depth.right = 0;
    _spu_rev_attr.delay = 0;
    _spu_rev_attr.feedback = 0;
    _spu_rev_offsetaddr = _spu_rev_startaddr[0];
    _spu_FsetRXX(0xD1, _spu_rev_startaddr[0], 0);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = 0;
    _spu_trans_mode = 0;
    _spu_transMode = 0;
    _spu_keystat = 0;
    _spu_RQmask = 0;
    _spu_RQvoice = 0;
    _spu_env = 0;
}
extern s32 _spu_isCalled;
extern s32 _spu_EVdma;
void _spu_FiDMA(void);

void SpuStart(void) {
    s32 v0;
    if (_spu_isCalled == 0) {
        _spu_isCalled = 1;
        EnterCriticalSection();
        _SpuDataCallback((s32)_spu_FiDMA);
        v0 = OpenEvent((s32)0xF0000009, 0x20, 0x2000, 0);
        _spu_EVdma = v0;
        EnableEvent(v0);
        ExitCriticalSection();
    }
}
