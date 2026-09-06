/* REJECTED s1 (2026-09-06): sandbox 5 -- `size <<= 16;` statement before _spu_FsetDelayR(); shift hoists above the call, ori still lifted, extra reg/order diffs. Plain-pointer decls. */
void _spu_Fr_(s32 addr, u16 mode, s32 size) {
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = mode;
    _spu_Fw1ts();
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = *(volatile u16 *)(D_800A2CDC + 0x1AA) | 0x30;
    _spu_Fw1ts();
    size <<= 16;
    _spu_FsetDelayR();
    *D_800A2CE0 = addr;
    *D_800A2CE4 = size | 0x10;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}
