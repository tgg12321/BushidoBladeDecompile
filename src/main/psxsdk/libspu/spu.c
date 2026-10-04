/* PsyQ 4.0 LIBSPU SPU: _spu_init, _spu_FwriteByIO, _spu_FiDMA, _spu_Fr_, _spu_t, _spu_Fw, _spu_Fr,
 * _spu_FsetRXX, _spu_FsetRXXa, _spu_FgetRXXa, _spu_FsetPCR, _spu_FsetDelayW, _spu_FsetDelayR and
 * _spu_Fw1ts. .text 0x80088740..0x800892D4, a verbatim LIBSCAN module span
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

    *D_800A2CEC |= 0xB0000;

    _spu_transMode = 0;
    _spu_addrMode = 0;
    _spu_tsa = 0;
    *(volatile u16 *)(_spu_RXX + 0x180) = 0;
    *(volatile u16 *)(_spu_RXX + 0x182) = 0;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = 0;
    _spu_Fw1ts();

    *(volatile u16 *)(_spu_RXX + 0x180) = 0;
    *(volatile u16 *)(_spu_RXX + 0x182) = 0;

    if (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF) {
        i = 0;
        do {
            if (++i > 0xF00) {
                printf(&D_800163D8, &D_800163E8);
                break;
            }
        } while (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF);
    }

    channel = 0;
    _spu_mem_mode = 2;
    _spu_mem_mode_plus = 3;
    _spu_mem_mode_unit = 8;
    _spu_mem_mode_unitM = 7;
    *(volatile u16 *)(_spu_RXX + 0x1AC) = 4;
    *(volatile u16 *)(_spu_RXX + 0x184) = 0;
    *(volatile u16 *)(_spu_RXX + 0x186) = 0;
    *(volatile u16 *)(_spu_RXX + 0x18C) = 0xFFFF;
    *(volatile u16 *)(_spu_RXX + 0x18E) = 0xFFFF;
    *(volatile u16 *)(_spu_RXX + 0x198) = 0;
    *(volatile u16 *)(_spu_RXX + 0x19A) = 0;
    for (channel = 0; channel < 10; channel++) {
        _spu_RQ[channel] = 0;
    }

    if (a0 == 0) {
        s32 kon;
        s32 koff;
        volatile u16 *vp;

        _spu_tsa = 0x200;
        *(volatile u16 *)(_spu_RXX + 0x190) = 0;
        *(volatile u16 *)(_spu_RXX + 0x192) = 0;
        *(volatile u16 *)(_spu_RXX + 0x194) = 0;
        *(volatile u16 *)(_spu_RXX + 0x196) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B0) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B2) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B4) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B6) = 0;
        _spu_FwriteByIO((s32)&D_800A2D1C, 0x10);

        vp = (volatile u16 *)_spu_RXX;
        for (channel = 0; channel < 0x18; channel++) {
            vp[channel * 8 + 0] = 0;
            vp[channel * 8 + 1] = 0;
            vp[channel * 8 + 2] = 0x3FFF;
            vp[channel * 8 + 3] = 0x200;
            vp[channel * 8 + 4] = 0;
            vp[channel * 8 + 5] = 0;
        }

        kon = 0xFFFF;
        koff = 0xFF;
        *(volatile u16 *)(_spu_RXX + 0x188) = kon;
        *(volatile u16 *)(_spu_RXX + 0x18A) = koff;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();

        *(volatile u16 *)(_spu_RXX + 0x18C) = kon;
        *(volatile u16 *)(_spu_RXX + 0x18E) = koff;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
    }

    _spu_inTransfer = 1;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = 0xC000;
    _spu_transferCallback = 0;
    _spu_IRQCallback = 0;
    return 0;
}
/* PsyQ LIBSPU spu.c: `_spu_FwriteByIO` (static) — verbatim-linked Sony object.
   C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
   sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO).

   `_spu_RXX` (0x800A2CDC) holds the SPU register-file base (0x1F801C00), so
   `_spu_RXX + 0x1A6` is the SPU transfer/control register block at 0x1F801DA6:
   transfer address, data FIFO, SPUCNT, transfer control, SPUSTAT — five
   consecutive 16-bit hardware registers.  Sony's own libspu reaches them
   through `union SpuUnion *_spu_RXX` with the SPUR()/SPUW() field macros; the
   struct below is that same register block, and every access in this function
   goes through it, exactly as the original source does. */
typedef struct {
    u16 trans_addr;  /* 0x1DA6 */
    u16 trans_fifo;  /* 0x1DA8 */
    u16 spucnt;      /* 0x1DAA */
    u16 trans_ctrl;  /* 0x1DAC */
    u16 spustat;     /* 0x1DAE */
} SpuCtrlRegs;

#define SPU_CTRL ((volatile SpuCtrlRegs *)(_spu_RXX + 0x1A6))

void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;

    cur = (u16 *)addr;
    spustat = SPU_CTRL->spustat & 0x7FF;
    SPU_CTRL->trans_addr = _spu_tsa;
    _spu_Fw1ts();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        for (i = 0; i < num; i += 2) {
            SPU_CTRL->trans_fifo = *cur++;
        }
        cnt = SPU_CTRL->spucnt;
        cnt &= ~0x30;
        cnt |= 0x10;
        SPU_CTRL->spucnt = cnt;
        _spu_Fw1ts();
        if (SPU_CTRL->spustat & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (SPU_CTRL->spustat & 0x400);
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num;
    }
    cnt = SPU_CTRL->spucnt;
    j = 0;
    cnt &= ~0x30;
    SPU_CTRL->spucnt = cnt;
    if ((SPU_CTRL->spustat & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((SPU_CTRL->spustat & 0x7FF) != spustat);
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
/* PsyQ LIBSPU spu.c `_spu_FiDMA` (C ref: Xeeynamo/psyz decomp/src/libspu/spu.c:161).
   SPU DMA-completion interrupt handler: waits for the transfer-mode bits
   (0x30) in SPUCNT (_spu_RXX + 0x1AA) to clear with a bounded spin, then
   dispatches either the installed transfer callback or the SPU DMA event. */
void _spu_FiDMA(void) {
    u32 timeout;

    if (D_800A2D2C == 0) {
        _spu_Fw1ts();
    }
    *(volatile u16 *)(_spu_RXX + 0x1AA) =
        *(volatile u16 *)(_spu_RXX + 0x1AA) & ~0x30;
    timeout = 0;
    while (*(volatile u16 *)(_spu_RXX + 0x1AA) & 0x30) {
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
void _spu_Fr_(s32 addr, u16 mode, s32 size) {
    *(volatile u16 *)(_spu_RXX + 0x1A6) = mode;
    _spu_Fw1ts();
    *(volatile u16 *)(_spu_RXX + 0x1AA) = *(volatile u16 *)(_spu_RXX + 0x1AA) | 0x30;
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
        *(volatile u16 *)(_spu_RXX + 0x1A6) = _spu_tsa;
        break;

    case 1:
        t = *(volatile u16 *)(_spu_RXX + 0x1A6);
        i = 0;
        D_800A2D2C = 0;
        if ((t & 0xFFFF) != _spu_tsa) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (*(volatile u16 *)(_spu_RXX + 0x1A6) != _spu_tsa);
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x20;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        break;

    case 0:
        t = *(volatile u16 *)(_spu_RXX + 0x1A6);
        i = 0;
        D_800A2D2C = 1;
        if ((t & 0xFFFF) != _spu_tsa) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (*(volatile u16 *)(_spu_RXX + 0x1A6) != _spu_tsa);
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x30;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        break;

    case 3:
        if (D_800A2D2C == 1) {
            ck2 = 0x30;
        } else {
            ck2 = 0x20;
        }
        i = 0;
        while ((*(volatile u16 *)(_spu_RXX + 0x1AA) & 0x30) != ck2) {
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
        *(volatile u16 *)(arg0 * 2 + _spu_RXX) = arg1;
        return;
    }
    *(volatile u16 *)(arg0 * 2 + _spu_RXX) = arg1 >> _spu_mem_mode_plus;
}
s32 _spu_FsetRXXa(s32 mode, s32 val) {
    s32 aligned;
    if (_spu_mem_mode != 0) {
        u32 step = _spu_mem_mode_unit;
        if ((u32)val % step != 0) {
            val += step;
            val &= ~_spu_mem_mode_unitM;
        }
    }
    aligned = (s32)((u32)val >> _spu_mem_mode_plus);
    if (mode == -2) goto ret_val_m2;
    if (mode != -1) goto store;
    return aligned & 0xFFFF;
ret_val_m2:
    return val;
store:
    ((s16 *)_spu_RXX)[mode] = (s16)aligned;
    return val;
}
s32 _spu_FgetRXXa(s32 index, s32 mode) {
    u16 val = ((u16 *)_spu_RXX)[index];
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
    volatile s32 i;     /* SOTN: src/main/psxsdk/libspu/spu.c:14 @db41b28 */
    volatile s32 v = 0xD; /* SOTN: src/main/psxsdk/libspu/spu.c:15 @db41b28 */
    for (i = 0; i < 0x3C; i++) {
        v = v * 13;
    }
}
