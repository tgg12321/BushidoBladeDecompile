/* REJECTED s1 (2026-09-06): sandbox 5 -- local bcr=(size<<16)|0x10 computed before _spu_FsetDelayR(); both ALU ops hoist above the call. Plain-pointer decls. */
void _spu_Fr_(s32 addr, u16 mode, s32 size) {
    s32 bcr;
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = mode;
    _spu_Fw1ts();
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = *(volatile u16 *)(D_800A2CDC + 0x1AA) | 0x30;
    _spu_Fw1ts();
    bcr = (size << 16) | 0x10;
    _spu_FsetDelayR();
    *D_800A2CE0 = addr;
    *D_800A2CE4 = bcr;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}
