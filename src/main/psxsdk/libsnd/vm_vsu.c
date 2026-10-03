/* PsyQ 4.0 LIBSND VM_VSU: _SsVmVSetUp. .text 0x80087E3C..0x80087F00, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "sound.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
typedef struct {
    u8 prior, mode, vol, pan, center, shift, min, max;
    u8 vibW, vibT, porW, porT, pbmin, pbmax, reserved1, reserved2;
    u16 adsr1, adsr2;
    s16 prog, vag;
    s16 reserved[4];
} VagAtr;
extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;

extern u8 _svm_vab_used[];
extern s16 kMaxPrograms;
extern s32 _svm_vab_vh[];
extern s32 _svm_vab_pg[];
extern s32 _svm_vab_tn[];
s32 _SsVmVSetUp(s32 a0, s32 a1) {
    u16 a0h;
    s16 a1h;
    s32 idx;
    s32 sa1;
    s32 v0;
    int v1;
    s32 v2;
    s32 entry;
    s32 ret;
    a0h = a0;
    a1h = a1;
    if ((a0 & 0xFFFFu) >= 0x10) goto fail;
    idx = (s16)a0h;
    if (_svm_vab_used[idx] != 1) return -1;
    sa1 = a1h;
    if (sa1 < kMaxPrograms) goto ok;
fail:
    return -1;
ok:
    ret = idx << 2;
    v0 = *(s32 *)((u8 *)_svm_vab_vh + ret);
    v1 = *(s32 *)((u8 *)_svm_vab_pg + ret);
    v2 = *(s32 *)((u8 *)_svm_vab_tn + ret);
    ret = sa1 << 4;
    _svm_cur.vabId = (u8) a0h;
    _svm_cur.prog = (u8) a1h;
    ret += v1;
    entry = *((s32 *) (ret + 8));
    _svm_vh = (VabHdr *)v0;
    _svm_pg = v1;
    _svm_tn = (VagAtr *)v2;
    _svm_cur.field_7_fake_program = (u8)entry;
    return 0;
}
