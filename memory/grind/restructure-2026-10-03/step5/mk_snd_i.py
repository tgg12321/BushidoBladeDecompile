import re
s = open("include/sound.h", encoding="utf-8").read()
body = s.split("/* Named globals */\n", 1)[1].split("/* Functions */", 1)[0]
# drop the ProgAtr and VabHdr typedefs (with their comments): they are public (libsnd.h)
prog = re.search(r"/\* PsyQ ProgAtr.*?\} ProgAtr;\n\n", body, re.S)
body = body.replace(prog.group(0), "")
vab = re.search(r"/\* PsyQ VabHdr.*?\} VabHdr;\n", body, re.S)
body = body.replace(vab.group(0), "")
head = """#ifndef LIBSND_I_H
#define LIBSND_I_H

/* PsyQ LIBSND library-internal state and helpers shared by the modules in this directory
 * (SOTN src/main/psxsdk/libsnd/libsnd_i.h). */

#include <psxsdk/libsnd.h>
"""
tail = """
/* Voice-manager and sequencer state (one declaration per object; splat names kept). */
extern u8 _SsVmMaxVoice;
extern s16 kMaxPrograms;
extern s32 VBLANK_MINUS;
extern s32 _snd_ev_flag;
extern s32 _snd_openflag;
extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;
extern s16 _svm_damper;
extern s16 _svm_stereo_mono;
extern u8 _svm_auto_kof_mode;
extern s16 _svm_sreg_buf[];
extern u8 _svm_sreg_dirty[];
extern s32 _svm_envx_hist[];
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 D_800F1B14;              /* psyz _svm_orev1 */
extern u16 D_800F2B68;              /* psyz _svm_orev2 */
extern u16 _svm_vab_count;
extern u8 _svm_vab_used[];
extern s32 _svm_vab_start[];
extern s32 _svm_vab_total[];
extern s32 _svm_vab_vh[];
extern s32 _svm_vab_pg[];
extern s32 _svm_vab_tn[];
/* _svm_rattr: the voice manager's reverb attribute block, declared by its splat per-field
   names (a typed SpuReverbAttr declaration is Phase 2 work). */
extern s32 _svm_rattr;
extern s32 _svm_rattr_plus_0x4;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;

extern void _SsInit(void);
extern void _SsVmInit(s32);
extern void _SsVmFlush(void);
extern void _SsVmKeyOnNow(s32, u16);
extern s16 _SsVmGetSeqVol(s32, s16 *, s16 *);
extern s16 func_80087770(s16, u16, u16, s16);
extern void vmNoiseOn(u8);
extern s32 note2pitch2(u16, u16);
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsSeqPlay(s16, s16);
extern void _SsSndPlay(s16, s16);
extern void _SsSndCrescendo(s16, s16);
extern void _SsSndDecrescendo(s16, s16);
extern void _SsSndTempo(s16, s16);
extern void _SsSndPause(s16, s16);
extern void _SsSndReplay(s16, s16);
extern void _SsSndStop(s16, s16);

#endif /* LIBSND_I_H */
"""
out = head + "\n" + body.strip("\n") + "\n" + tail
open("src/main/psxsdk/libsnd/libsnd_i.h", "w", encoding="utf-8", newline="\n").write(out)
print(out[:3000])
