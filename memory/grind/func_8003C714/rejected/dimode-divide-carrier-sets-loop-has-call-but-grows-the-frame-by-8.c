/*
 * REJECTED (s18, 2026-09-05, forensics) -- the DImode-divide-by-a-variable
 * spelling of the loop_has_call carrier. It DOES set loop_has_call (threshold
 * 122 -> 61) and it IS deleted whole by flow.c (build_insns 104 == target 104),
 * but __divdi3 takes two DImode arguments and the outgoing-argument area grows
 * from 16 to 24 bytes, so the frame becomes 0x28 where the target is 0x20:
 * measured `subu $sp,$sp,40` on c_dvar / c_mvar / N_car1 / Q_llv / M_pre_C
 * versus `subu $sp,$sp,32` on the target and on every __fixsfdi spelling.
 * sandbox: M_pre_C score 16, N_car1 score 22, both at build_insns 104.
 * The frame cost is charged at RTL-expansion time (outgoing args size), long
 * before flow deletes the block, so deleting the call cannot recover it.
 * Use a float -> long long conversion (SF argument, 4 bytes) instead; see
 * rejected/loop-has-call-halves-threshold-one-dead-fixsfdi-carrier-d0-but-rhs-unnatural.c
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    u8 *base;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    base = (u8 *)&D_80106A58;
    do {
        src = base + i * 8;
        dst = (u8 *)s0 + i * 4;
                v = (long long)i / (long long)*(s32 *)(src + 4);
a = *(s32 *)(src + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(src + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(src + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = *src;
        dst[0x24] = v;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
