/* PsyQ 4.0 LIBSND SSCALL: SsSeqCalledTbyT. .text 0x80083F6C..0x800841E0, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "sound.h"
#include <psxsdk/libspu.h>

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _snd_ev_flag;  /* _snd_ev_flag */
extern s16 _snd_seq_s_max;  /* _snd_seq_s_max */
extern s16 _snd_seq_t_max;  /* _snd_seq_t_max */
extern s32 _snd_openflag;  /* _snd_openflag */
extern void _SsVmFlush(void);   /* SpuVmFlush */
void _SsSndPlay(s16 a0, s16 a1); /* _SsSndPlay */
void _SsSndCrescendo(s16 arg0, s16 arg1);     /* _SsSndCrescendo */
void _SsSndDecrescendo(s16 arg0, s16 arg1);     /* _SsSndDecrescendo */
void _SsSndTempo(s16 a0, s16 a1);         /* _SsSndTempo */
void _SsSndPause(s16 a0, s16 a1);    /* _SsSndPause */
void _SsSndReplay(s16 a0, s16 a1);   /* _SsSndReplay */
void _SsSndStop(s16 a0, s16 a1);         /* _SsSndStop */

static void SsSeqCalledTbyT(void) {
    int i;
    int j;
    if (_snd_ev_flag != 1) {
        _snd_ev_flag = 1;

        _SsVmFlush();

        for (i = 0; i < _snd_seq_s_max; i++) {
            s32 bit = 1 << i;
            if (_snd_openflag & bit) {
                for (j = 0; j < _snd_seq_t_max; j++) {
                    if (_ss_score[i][j].unk98 & 1) {
                        _SsSndPlay(i, j);

                        if (_ss_score[i][j].unk98 & 0x10) {
                            _SsSndCrescendo(i, j);
                        }
                        if (_ss_score[i][j].unk98 & 0x20) {
                            _SsSndDecrescendo(i, j);
                        }
                        if (_ss_score[i][j].unk98 & 0x40) {
                            _SsSndTempo(i, j);
                        }
                        if (_ss_score[i][j].unk98 & 0x80) {
                            _SsSndTempo(i, j);
                        }
                    }
                    if (_ss_score[i][j].unk98 & 2) {
                        _SsSndPause(i, j);
                    }
                    if (_ss_score[i][j].unk98 & 8) {
                        _SsSndReplay(i, j);
                    }
                    if (_ss_score[i][j].unk98 & 4) {
                        _SsSndStop(i, j);
                        _ss_score[i][j].unk98 = 0;
                    }
                }
            }
        }
        _snd_ev_flag = 0;
    }
}
