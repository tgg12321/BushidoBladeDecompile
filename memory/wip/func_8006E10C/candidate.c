/* func_8006E10C (text1b.c) — CLEAN pin-free body (NO register asm pins).
 * Honest full-build distance 13 vs HEAD's 5 register-asm pins
 * (ff0 asm("$17"), temp_s3 asm("$19"), a0v asm("$4"), a1v asm("$5")). The
 * instruction stream is otherwise correct; the residual is a coupled
 * s0 <-> s1 register-allocation swap (target: base in $s0, ff0=0xF0 in $s1;
 * GCC: opposite) PLUS the ff0=0xF0 `li` landing in the func_80036EA8 jal
 * delay slot (target) vs deferred (GCC). 5 structural orderings flatlined
 * or worsened (13/21). Register-allocation+scheduling wall — see notes.md. */
s32 func_8006E10C(void) {
    s32 ff0;
    s32 temp_s3 = D_800A3500;
    u8 rect[8];
    s32 v0;
    s32 a0v;
    s32 a1v;
    s32 base;
    s32 base2;

    __builtin_memcpy(rect, D_800A32D8, 8);
    if (((s32 *)D_800A3524)[8] & 1) {
        a0v = 2;
        a1v = 0x60;
    } else {
        a0v = 2;
        a1v = 7;
    }
    ff0 = 0xF0;
    v0 = func_80036EA8(a0v, a1v);
    replay_camera_Init(v0, D_800A3500);
    game_FrameLoop();
    func_80036F28(v0);
    gpu_SetDispMask(0);
    base = (s32)&D_800F7438;
    gpu_InitDrawEnv(base, 0, 0, 0x280, ff0);
    gpu_InitDrawEnv(base + 0x4090, 0, ff0, 0x280, ff0);
    gpu_InitDispEnv(base + 0x5C, 0, ff0, 0x280, ff0);
    base2 = base + 0x40EC;
    gpu_InitDispEnv(base2, 0, 0, 0x280, ff0);
    D_800F74A4 = 0;
    D_800FB534 = 0;
    D_800F74A5 = 0;
    D_800FB535 = 0;
    gpu_DrawSync(0);
    func_8007B4D0((s32)rect, 0, 0, 0);
    gpu_DrawSync(0);
    gpu_LoadImage(rect, temp_s3 + 0x14);
    gpu_DrawSync(0);
    func_8007B9B0(base);
    func_8007BC08(base2);
    gpu_SetDispMask(1);
    return 1;
}
