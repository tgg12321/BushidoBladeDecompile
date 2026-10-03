/* PsyQ 4.0 LIBSPU S_SR: SpuSetReverb. .text 0x80089D60..0x80089E30, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_rev_flag;
extern s32 _spu_rev_reserve_wa;
extern s32 _spu_rev_offsetaddr;
extern s32 _spu_RXX;

/* PsyQ 4.0 LIBSPU s_sr: SpuSetReverb — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_sr.c */
s32 SpuSetReverb(s32 on_off) {
    u16 cnt;
    switch (on_off) {
    case 0:
        cnt = *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA);
        _spu_rev_flag = 0;
        cnt &= ~0x80;
        *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA) = cnt;
        break;

    case 1:
        if ((_spu_rev_reserve_wa != on_off) && _SpuIsInAllocateArea_(_spu_rev_offsetaddr)) {
            cnt = *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA);
            _spu_rev_flag = 0;
            cnt &= ~0x80;
            *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA) = cnt;
        } else {
            cnt = *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA);
            _spu_rev_flag = on_off;
            cnt |= 0x80;
            *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA) = cnt;
        }
        break;
    }

    return _spu_rev_flag;
}
