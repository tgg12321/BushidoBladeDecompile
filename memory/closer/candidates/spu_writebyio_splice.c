/* BANKED CANDIDATE (Closer phase-3 session 11, 2026-07-10): the
 * DispUpdateStatusMessage splice region (0x800889D4..0x80088D0C, 206 words)
 * = Sony spu.c _spu_FwriteByIO + _spu_FiDMA + _spu_Fr_.
 * STATUS: _spu_FwriteByIO BIT-EXACT (132/132 words incl. the vestigial
 * volatile sp0/sp4 frame). Residual: _spu_FiDMA wait loop 2 words short
 * (entry li-fold vs move+addu, missing reorg compensation addiu -1) +
 * _spu_Fr_ 2 masked ori-position diffs. See phase3-progress.md session 11
 * for the fully-named mechanism + cc1psx exhibit (tmp/closer/spu3/t_psx.s).
 * Prover: tmp/closer/spu_splice_prove.sh (full-Makefile-flag pipeline).
 * DO NOT apply while asmfix:123 (DispUpdateStatusMessage splice) is active
 * (statics would double-emit).
 */
/* PsyQ 4.0 LIBSPU spu.c: _spu_FwriteByIO (static) - verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/
   spu.c (_spu_writeByIO). Splice extent also covers _spu_FiDMA (0x80088BA0,
   address-installed as the SPU DMA IRQ callback via g_snd_irq_data) and
   _spu_Fr_ (0x80088C60, unreferenced Sony export linked as dead code). */
void DispUpdateStatusMessage(u8 *addr, u32 size) {
    volatile s32 sp0; /* FAKE(ruling-3 class): Sony's WASTE_TIME() macro locals (sotn spu.c
                         _spu_writeByIO ships both decls); vestigial in the 4.0-interim build
                         where WASTE_TIME is the out-of-line _spu_Fw1ts call - the original
                         object's 0x30 frame (8 bytes of untouched locals) proves them */
    volatile s32 sp4; /* FAKE: see sp0 */
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;
    s32 b;

    cur = (u16 *)addr;
    spustat = *(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF;
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = D_800A2CF4;
    spu_WriteReg16();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        i = 0;
        if (num > 0) {
            b = D_800A2CDC;
            do {
                *(volatile u16 *)(b + 0x1A8) = *cur++;
                i += 2;
            } while (i < num);
        }
        cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x10;
        *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        spu_WriteReg16();
        if (*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    debug_printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x400);
        }
        spu_WriteReg16();
        spu_WriteReg16();
        size -= num;
    }
    cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
    j = 0;
    cnt &= ~0x30;
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
    if ((*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                debug_printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF) != spustat);
    }
}

extern void bios_DeliverEvent(s32, s32);

/* PsyQ 4.0 LIBSPU spu.c: _spu_FiDMA - SPU DMA-complete IRQ callback;
   installed by address only (g_snd_irq_data = 0x80088BA0). */
static void _spu_FiDMA(void) {
    u32 i;
    s32 b;

    if (D_800A2D2C == 0) {
        spu_WriteReg16();
    }
    b = D_800A2CDC;
    *(volatile u16 *)(b + 0x1AA) = *(volatile u16 *)(b + 0x1AA) & 0xFFCF;
    /* NOTE: Sony's source here is the plain `while (spucnt & 0x30) { if
       (++i > 0xF00) break; }` (cc1psx emits the target bytes from it,
       exhibit tmp/closer/spu3/t_psx.s) -- our fork SEGFAULTS on that input
       class (volatile-cond while + break). Closest accepted spelling: */
    i = 0;
    if (*(volatile u16 *)(b + 0x1AA) & 0x30) {
        i++;
poll:
        if (i < 0xF01) {
            if (*(volatile u16 *)(b + 0x1AA) & 0x30) {
                i++;
                goto poll;
            }
        }
    }
    if (D_800A2D14 != 0) {
        ((void (*)())D_800A2D14)();
    } else {
        bios_DeliverEvent(0xF0000009, 0x20);
    }
}

/* PsyQ 4.0 LIBSPU spu.c: _spu_Fr_ - unreferenced in BB2 (dead code carried
   by the linked Sony object; SpuRGetAllKeysStatus/S_SCA precedent). */
static void _spu_Fr_(s32 arg0, u16 arg1, s32 arg2) {
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = arg1;
    spu_WriteReg16();
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = *(volatile u16 *)(D_800A2CDC + 0x1AA) | 0x30;
    spu_WriteReg16();
    spu_ReadReg();
    *D_800A2CE0 = arg0;
    *D_800A2CE4 = (arg2 << 16) | 0x10;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}
