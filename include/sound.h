#ifndef SOUND_H
#define SOUND_H

/* Sound/SPU subsystem - BGM, SE, SPU control */

#include "common.h"

/* Named globals */
extern s32 g_snd_bgm_id;
extern s32 g_snd_se_id;
extern s32 g_snd_volume;

/* Functions */
extern void spu_SetVolume(s32, s32, s32);

#endif /* SOUND_H */
