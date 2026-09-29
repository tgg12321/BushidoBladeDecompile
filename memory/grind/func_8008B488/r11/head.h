#include "common.h"
#include "psx.h"
#include "sound.h"

extern s32 _spu_RXX;
typedef struct {
    s16 left, right;
} SpuVolume;
extern s32 _spu_FsetRXXa(s32, s32);
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine);
