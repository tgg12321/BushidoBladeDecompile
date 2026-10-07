/* PsyQ 4.0 LIBAPI PAD: the controller init/start/stop wrappers and their
 * interrupt patch (SetInitPadFlag .. _IsVSync). .text 0x80078BE0..0x80078F00, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libapi.h>

/* Declarations from the file this module was split from (src/main/64FD8.c, ex
 * text1b_b.c). */
s32 _Pad1(void);
/* SIO port 0, 0x1F801040 (asm/data/7D920.data.s:23915) */
extern volatile SioRegs *D_8009BD84;
/* I_STAT (I_MASK at [1]), 0x1F801070 (asm/data/7D920.data.s:23921) */
extern volatile u32 *D_8009BD88;

extern s32 D_8009BD80;

void SetInitPadFlag(s32 a0) { D_8009BD80 = a0; }

s32 ReadInitPadFlag(void) { return D_8009BD80; }

void _remove_ChgclrPAD(void);
void _patch_pad(void);
s32 SetPatchPad(void);
void PAD_init2(s32, s32, s32, s32);
void _send_pad(void);

void PAD_init(s32 a0, s32 a1, s32 a2, s32 a3) {
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    SetPatchPad();
    PAD_init2(a0, a1, a2, a3);
    _send_pad();
    D_8009BD80 = 1;
}

void InitPAD2(s32, s32, s32, s32);

void InitPAD(s32 a0, s32 a1, s32 a2, s32 a3) {
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    SetPatchPad();
    InitPAD2(a0, a1, a2, a3);
    _send_pad();
    D_8009BD80 = 1;
}

void StartPAD2(void);
void EnablePAD(void);

void StartPAD(void) {
    StartPAD2();
    ChangeClearPAD(0);
    EnablePAD();
}

void DisablePAD(void);
void StopPAD2(void);
s32 RemovePatchPad(void);

void StopPAD(void) {
    DisablePAD();
    StopPAD2();
    RemovePatchPad();
    D_8009BD80 = 0;
}

extern void SysDeqIntRP(s32, u32 *);
extern void SysEnqIntRP(s32, u32 *);

extern s32 _IsVSync(void);
extern u32 patch0_plus_0x4;
extern u32 patch0_plus_0x8;
extern u32 patch0;
extern u32 patch0_plus_0xC;

s32 SetPatchPad(void) {
    u32 *v1 = &patch0_plus_0x4;
    u32 *s0 = v1 - 1;
    EnterCriticalSection();
    *v1 = (u32)_Pad1;
    patch0_plus_0x8 = (u32)_IsVSync;
    patch0 = 0;
    patch0_plus_0xC = 0;
    SysDeqIntRP(1, s0);
    SysEnqIntRP(1, s0);
    ExitCriticalSection();
    return 1;
}

s32 RemovePatchPad(void) {
    EnterCriticalSection();
    SysDeqIntRP(1, &patch0);
    ExitCriticalSection();
    return 1;
}

/* PsyQ 4.0 LIBAPI PAD: _Pad1 (static) — verbatim-linked Sony object. */
s32 _Pad1(void) {
    /* FAKE: volatile delay counter, only i[0] used. volatile (owner rulings
       Q50 route A / Q48 route B, Q53): every access to the counter is a
       $sp-slot round-trip in the target; a plain local runs the loop in a
       register (score 15). [3]: frame layout -- the target frame is 16 bytes,
       a scalar counter gives 8 (score 2). SOTN's v_wait counts down element
       0 of a volatile array the same way. */
    volatile s32 i[3]; /* SOTN: src/main/psxsdk/libetc/vsync.c:52 @db41b28 */
    D_8009BD84->ctrl = 0;
    i[0] = 10;
    i[0] = i[0] - 1;
    if (i[0] != -1) {
        do {
            i[0] = i[0] - 1;
        } while (i[0] != -1);
    }
    return 0;
}

s32 _IsVSync(void) {
    volatile u32 *p = D_8009BD88;
    s32 ret;
    if ((p[1] & 1) == 0)
        return 0;
    if ((p[0] & 1) != 0) {
        ret = 1;
    } else {
        /* FAKE: two-set else arm defeats jump.c store-flag fold
         * (dead-store-fake-exception) */
        ret = 1;
        ret = 0;
    }
    return ret;
}
