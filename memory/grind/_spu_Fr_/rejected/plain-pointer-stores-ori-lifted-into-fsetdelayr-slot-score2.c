/* REJECTED s1 (2026-09-06): sandbox 2 with the plain `extern s32 *D_800A2CE0/E4/E8` decls.
   Both our cc1 AND the original cc1psx (tmp/grind/_spu_Fr_/s1/v1_psx.s) lift
   `ori $s0,$s0,0x10` into the jal _spu_FsetDelayR delay slot (reorg.c
   fill_simple_delay_slots forward walk); target keeps a nop there and the ori
   in the lw D_800A2CE4 load delay. Non-volatile stores let the walk pass. */
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
