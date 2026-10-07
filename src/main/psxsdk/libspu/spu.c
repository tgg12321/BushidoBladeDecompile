/* PsyQ 4.0 LIBSPU SPU: _spu_init, _spu_FwriteByIO, _spu_FiDMA, _spu_Fr_,
 * _spu_t, _spu_Fw, _spu_Fr, _spu_FsetRXX, _spu_FsetRXXa, _spu_FgetRXXa,
 * _spu_FsetPCR, _spu_FsetDelayW, _spu_FsetDelayR and _spu_Fw1ts. .text
 * 0x80088740..0x800892D4, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "psx.h"
#include "libspu_internal.h"
#include <psxsdk/libapi.h>

/* Declarations from the old main.c's head that this module uses. */
extern s32 _spu_mem_mode;
extern s32 _spu_mem_mode_unit;
extern volatile u32 *D_800A2CEC;
extern s32 _spu_addrMode;
extern s32 D_800A2D1C;
extern void printf(s32 *, s32 *);
extern void _spu_Fw1ts(void);
extern s32 D_800A2D2C;
extern s32 D_800A2D30;
extern s32 D_800A2D34;
/* Sony _spu_madr/_spu_bcr/_spu_chcr: pointers to the SPU DMA (ch4) MMIO
 * registers 0x1F8010C0/C4/C8 (asm/data/7D920.data.s); pointee volatile per
 * mmio-volatile-type-level. */
extern volatile s32 *D_800A2CE0;
extern volatile s32 *D_800A2CE4;
extern volatile s32 *D_800A2CE8;

/* SPU-module debug strings (rodata 0x800163D8..0x80016420), defined before
 * func_80088740 (_spu_init), their first user. bb2.ld links this object's
 * .rodata after sstick.o's jump table and ahead of s_sca.o's (the tables of
 * func_8008AF9C, SpuSetCommonAttr), in link order. */
const char D_800163D8[16] = "SPU:T/O [%s]\n";
const char D_800163E8[16] = "wait (reset)";
const char D_800163F8[20] = "wait (wrdy H -> L)";
const char D_8001640C[20] = "wait (dmaf clear/W)";

/* PsyQ 4.0 LIBSPU spu.c: _spu_init — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_init) */
s32 _spu_init(s32 a0) {
    u32 i;
    s32 channel;
    volatile u16 *spucnt;

    *D_800A2CEC |= 0xB0000;

    _spu_transMode = 0;
    _spu_addrMode = 0;
    _spu_tsa = 0;
    _spu_RXX->rxx.main_vol.left = 0;
    _spu_RXX->rxx.main_vol.right = 0;
    _spu_RXX->rxx.spucnt = 0;
    _spu_Fw1ts();

    _spu_RXX->rxx.main_vol.left = 0;
    _spu_RXX->rxx.main_vol.right = 0;

    if (_spu_RXX->rxx.spustat & 0x7FF) {
        i = 0;
        do {
            if (++i > 0xF00) {
                printf(&D_800163D8, &D_800163E8);
                break;
            }
        } while (_spu_RXX->rxx.spustat & 0x7FF);
    }

    channel = 0;
    _spu_mem_mode = 2;
    _spu_mem_mode_plus = 3;
    _spu_mem_mode_unit = 8;
    _spu_mem_mode_unitM = 7;
    _spu_RXX->rxx.data_trans = 4;
    _spu_RXX->rxx.rev_vol.left = 0;
    _spu_RXX->rxx.rev_vol.right = 0;
    _spu_RXX->rxx.key_off[0] = 0xFFFF;
    _spu_RXX->rxx.key_off[1] = 0xFFFF;
    _spu_RXX->rxx.rev_mode[0] = 0;
    _spu_RXX->rxx.rev_mode[1] = 0;
    for (channel = 0; channel < 10; channel++) {
        _spu_RQ[channel] = 0;
    }

    if (a0 == 0) {

        _spu_tsa = 0x200;
        _spu_RXX->rxx.chan_fm[0] = 0;
        _spu_RXX->rxx.chan_fm[1] = 0;
        _spu_RXX->rxx.noise_mode[0] = 0;
        _spu_RXX->rxx.noise_mode[1] = 0;
        _spu_RXX->rxx.cd_vol.left = 0;
        _spu_RXX->rxx.cd_vol.right = 0;
        _spu_RXX->rxx.ex_vol.left = 0;
        _spu_RXX->rxx.ex_vol.right = 0;
        _spu_FwriteByIO((s32)&D_800A2D1C, 0x10);

        for (channel = 0; channel < 0x18; channel++) {
            _spu_RXX->rxx.voice[channel].volume.left = 0;
            _spu_RXX->rxx.voice[channel].volume.right = 0;
            _spu_RXX->rxx.voice[channel].pitch = 0x3FFF;
            _spu_RXX->rxx.voice[channel].addr = 0x200;
            _spu_RXX->rxx.voice[channel].adsr[0] = 0;
            _spu_RXX->rxx.voice[channel].adsr[1] = 0;
        }

        _spu_RXX->rxx.key_on[0] = 0xFFFF;
        _spu_RXX->rxx.key_on[1] = 0xFF;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();

        _spu_RXX->rxx.key_off[0] = 0xFFFF;
        _spu_RXX->rxx.key_off[1] = 0xFF;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
    }

    _spu_inTransfer = 1;
    /* FAKE: SPUCNT stored through a pointer; the member store
       `_spu_RXX->rxx.spucnt = 0xC000` lets sched lift the _spu_IRQCallback zero
       store above the sh (score 4). */
    spucnt = &_spu_RXX->rxx.spucnt;
    *spucnt = 0xC000;
    _spu_transferCallback = 0;
    _spu_IRQCallback = 0;
    return 0;
}

/* PsyQ LIBSPU spu.c: `_spu_FwriteByIO` (static) — verbatim-linked Sony object.
   C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
   sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO). */

void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;

    cur = (u16 *)addr;
    spustat = _spu_RXX->rxx.spustat & 0x7FF;
    _spu_RXX->rxx.trans_addr = _spu_tsa;
    _spu_Fw1ts();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        for (i = 0; i < num; i += 2) {
            _spu_RXX->rxx.trans_fifo = *cur++;
        }
        cnt = _spu_RXX->rxx.spucnt;
        cnt &= ~0x30;
        cnt |= 0x10;
        _spu_RXX->rxx.spucnt = cnt;
        _spu_Fw1ts();
        if (_spu_RXX->rxx.spustat & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (_spu_RXX->rxx.spustat & 0x400);
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num;
    }
    cnt = _spu_RXX->rxx.spucnt;
    j = 0;
    cnt &= ~0x30;
    _spu_RXX->rxx.spucnt = cnt;
    if ((_spu_RXX->rxx.spustat & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((_spu_RXX->rxx.spustat & 0x7FF) != spustat);
    }
}

/* PsyQ LIBSPU spu.c: _spu_FiDMA + _spu_Fr_ — two further exported entry
   points that splat merged into func_800889D4
   (docs/naming/libscan/boundary_fixes.md); both must stay immediately after
   their former host, in address order, so the link order reproduces the
   original byte layout. _spu_FiDMA.s also keeps the address label that marks
   its entry point, since that address is referenced as data elsewhere.
   Do NOT spell that label's symbol name in this file: engine/queue.py's
   not_a_c_function_text() word-searches the raw .c text (comments included)
   and would misread it as a C function. */
/* PsyQ LIBSPU spu.c `_spu_FiDMA` (C ref: Xeeynamo/psyz
   decomp/src/libspu/spu.c:161). SPU DMA-completion interrupt handler: waits for
   the transfer-mode bits (0x30) in SPUCNT to clear with a bounded spin, then
   dispatches either the installed transfer callback or the SPU DMA event. */
void _spu_FiDMA(void) {
    u32 timeout;

    if (D_800A2D2C == 0) {
        _spu_Fw1ts();
    }
    _spu_RXX->rxx.spucnt = _spu_RXX->rxx.spucnt & ~0x30;
    timeout = 0;
    while (_spu_RXX->rxx.spucnt & 0x30) {
        timeout++;
        if (timeout > 0xF00) {
            break;
        }
    }
    if (_spu_transferCallback) {
        _spu_transferCallback();
        return;
    }
    DeliverEvent(0xF0000009, 0x20);
}

/* PsyQ 4.0 LIBSPU spu.c: _spu_Fr_ — unreferenced in BB2 (dead code carried
   by the linked Sony object; SpuRGetAllKeysStatus/S_SCA precedent).
   C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_r_); this build's
   WASTE_TIME() is the out-of-line _spu_Fw1ts call. */
void _spu_Fr_(s32 addr, u16 spu_addr, s32 size) {
    _spu_RXX->rxx.trans_addr = spu_addr;
    _spu_Fw1ts();
    _spu_RXX->rxx.spucnt = _spu_RXX->rxx.spucnt | 0x30;
    _spu_Fw1ts();
    _spu_FsetDelayR();
    *D_800A2CE0 = addr;
    *D_800A2CE4 = (size << 16) | 0x10;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}

/* PsyQ 4.0 LIBSPU spu.c: _spu_t — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_t) */
typedef char *va_list;
#define va_start(ap, parmN) ((ap) = (va_list)(&(parmN) + 1))
#define va_arg(ap, T) ((ap) += sizeof(T), *(T *)((ap) - sizeof(T)))

s32 _spu_t(s32 mode, ...) {
    s32 var_a2;
    u32 i;
    va_list args;
    u32 count;
    u16 ck2;
    u16 cnt;
    u16 t;

    va_start(args, mode);

    switch (mode) {
    case 2:
        count = va_arg(args, u32);
        _spu_tsa = count >> _spu_mem_mode_plus;
        _spu_RXX->rxx.trans_addr = _spu_tsa;
        break;

    case 1:
        t = _spu_RXX->rxx.trans_addr;
        i = 0;
        D_800A2D2C = 0;
        if ((t & 0xFFFF) != _spu_tsa) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (_spu_RXX->rxx.trans_addr != _spu_tsa);
        }
        cnt = _spu_RXX->rxx.spucnt;
        cnt &= ~0x30;
        cnt |= 0x20;
        _spu_RXX->rxx.spucnt = cnt;
        break;

    case 0:
        t = _spu_RXX->rxx.trans_addr;
        i = 0;
        D_800A2D2C = 1;
        if ((t & 0xFFFF) != _spu_tsa) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (_spu_RXX->rxx.trans_addr != _spu_tsa);
        }
        cnt = _spu_RXX->rxx.spucnt;
        cnt &= ~0x30;
        cnt |= 0x30;
        _spu_RXX->rxx.spucnt = cnt;
        break;

    case 3:
        if (D_800A2D2C == 1) {
            ck2 = 0x30;
        } else {
            ck2 = 0x20;
        }
        i = 0;
        while ((_spu_RXX->rxx.spucnt & 0x30) != ck2) {
            if (++i > 0xF00) {
                return -2;
            }
        }
        if (D_800A2D2C == 1) {
            _spu_FsetDelayR();
        } else {
            _spu_FsetDelayW();
        }
        count = va_arg(args, u32);
        D_800A2D30 = count;
        count = va_arg(args, u32);
        D_800A2D34 = (count / 64);
        D_800A2D34 += ((count % 64) ? 1 : 0);
        *D_800A2CE0 = D_800A2D30;
        *D_800A2CE4 = (D_800A2D34 << 16) | 0x10;
        if (D_800A2D2C == 1) {
            var_a2 = 0x1000200;
        } else {
            var_a2 = 0x1000201;
        }
        *D_800A2CE8 = var_a2;
        break;
    }
    return 0;
}

s32 _spu_Fw(s32 a0, s32 a1) {
    if (_spu_transMode == 0) {
        _spu_t(2, _spu_tsa << _spu_mem_mode_plus);
        _spu_t(1);
        _spu_t(3, a0, a1);
    } else {
        _spu_FwriteByIO(a0, a1);
    }
    return a1;
}

s32 _spu_Fr(s32 a0, s32 a1) {
    _spu_t(2, _spu_tsa << _spu_mem_mode_plus);
    _spu_t(0);
    _spu_t(3, a0, a1);
    return a1;
}

void _spu_FsetRXX(s32 arg0, u32 arg1, s32 arg2) {
    if (arg2 == 0) {
        _spu_RXX->raw[arg0] = arg1;
        return;
    }
    _spu_RXX->raw[arg0] = arg1 >> _spu_mem_mode_plus;
}

s32 _spu_FsetRXXa(s32 index, s32 val) {
    s32 aligned;
    if (_spu_mem_mode != 0) {
        u32 step = _spu_mem_mode_unit;
        if ((u32)val % step != 0) {
            val += step;
            val &= ~_spu_mem_mode_unitM;
        }
    }
    aligned = (s32)((u32)val >> _spu_mem_mode_plus);
    if (index == -2)
        goto ret_val_m2;
    if (index != -1)
        goto store;
    return aligned & 0xFFFF;
ret_val_m2:
    return val;
store:
    _spu_RXX->raw[index] = aligned;
    return val;
}

s32 _spu_FgetRXXa(s32 index, s32 mode) {
    u16 val = _spu_RXX->raw[index];
    if (mode == -1) {
        return val;
    }
    return val << _spu_mem_mode_plus;
}

void _spu_FsetPCR(s32 arg0) {
    *D_800A2CEC &= 0xFFF8FFFF;
    if (arg0 != 0) {
        *D_800A2CEC |= 0x30000;
    } else {
        *D_800A2CEC |= 0x50000;
    }
}

extern volatile u32 *g_spu_dma_ctrl;

void _spu_FsetDelayW(void) {
    *g_spu_dma_ctrl = (*g_spu_dma_ctrl & DMA_CHAN_MASK) | DMA_SPU_FROM_RAM;
}

void _spu_FsetDelayR(void) {
    *g_spu_dma_ctrl = (*g_spu_dma_ctrl & DMA_CHAN_MASK) | DMA_SPU_TO_RAM;
}

/* LIBSPU spu.c WASTE_TIME(): the 4.0 rev's out-of-line busy-wait. */
void _spu_Fw1ts(void) {
    /* FAKE: volatile locals admitted on SOTN precedent (owner rulings Q50
       route A, Q53) -- every access to i and v is a $sp-slot round-trip, as
       in the target; the plain-local spelling keeps both in registers
       (score 25). SOTN's WASTE_TIME() runs the same counter/accumulator
       loop on its volatile pair (src/main/psxsdk/libspu/spu.c:7-11). */
    volatile s32 i;       /* SOTN: src/main/psxsdk/libspu/spu.c:14 @db41b28 */
    volatile s32 v = 0xD; /* SOTN: src/main/psxsdk/libspu/spu.c:15 @db41b28 */
    for (i = 0; i < 0x3C; i++) {
        v = v * 13;
    }
}
