void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    register s32 i asm("t0");
    register u8 *src asm("a2");
    register u8 *dst asm("a1");

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    src = (u8 *)&D_80106A58;
    dst = (u8 *)s0;
    do {
        *(u8 *)(dst + 0x21) = (u8)(*(s32 *)(src + 4) / 1800);
        *(u8 *)(dst + 0x22) = (u8)((*(s32 *)(src + 4) / 30) % 60);
        i += 1;
        *(u8 *)(dst + 0x23) = (u8)(((*(s32 *)(src + 4) % 30) * 100) / 30);
        *(u8 *)(dst + 0x24) = *src;
        src += 8;
        dst += 4;
    } while (i < 3);
    func_8001CD68(buf);
    *(u8 *)((u8 *)s0 + 0x2D) = (u8)*(u16 *)buf;
    *(u8 *)((u8 *)s0 + 0x2E) = buf[2];
    *(u8 *)((u8 *)s0 + 0x2F) = buf[3];
    *(u8 *)((u8 *)s0 + 0x30) = (u8)D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
