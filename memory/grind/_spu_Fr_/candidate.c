/* PsyQ 4.0 LIBSPU spu.c: _spu_Fr_ — unreferenced in BB2 (dead code carried
   by the linked Sony object; SpuRGetAllKeysStatus/S_SCA precedent).
   C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_r_); this build's
   WASTE_TIME() is the out-of-line _spu_Fw1ts call. */
void _spu_Fr_(s32 addr, u16 mode, s32 size) {
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = mode;
    _spu_Fw1ts();
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = *(volatile u16 *)(D_800A2CDC + 0x1AA) | 0x30;
    _spu_Fw1ts();
    _spu_FsetDelayR();
    *D_800A2CE0 = addr;
    *D_800A2CE4 = (size << 16) | 0x10;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}
/* --- Required declaration change (src/main.c, replaces the plain-pointer externs) ---
 * extern volatile s32 *D_800A2CE0;   Sony _spu_madr -> .word 0x1F8010C0 (DMA4 MADR)
 * extern volatile s32 *D_800A2CE4;   Sony _spu_bcr  -> .word 0x1F8010C4 (DMA4 BCR)
 * extern volatile s32 *D_800A2CE8;   Sony _spu_chcr -> .word 0x1F8010C8 (DMA4 CHCR)
 * (asm/data/7D920.data.s:33123-33137). Session 1 (2026-09-06): sandbox 0 for
 * _spu_Fr_ AND 0 for sibling _spu_t (164 insns) under this declaration. */
