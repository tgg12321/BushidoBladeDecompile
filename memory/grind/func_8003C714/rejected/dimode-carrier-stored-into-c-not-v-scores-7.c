/*
 * REJECTED (s18b, 2026-09-05). The s18b __divdi3 dead-store carrier is
 * frame-exact and drives the hoist decision correctly, but its DESTINATION
 * variable is load-bearing: written into `c` (the hundredths accumulator) with
 * the RHS `((long long)(t %% 30) * 100) / 30`, the loop dump shows insn_count 76
 * with BOTH life-1 movables declined and the emitted function still measures
 * `sandbox --disable all` = score 7 at build_insns 104 == target 104. The
 * residual is a v0/v1 swap plus one `sra` scheduled a slot early inside the
 * magic-multiply quartet. Written into `v` instead (candidate.c) the same
 * mechanism measures score 0. Do not re-propose the `c` destination.
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
                c = ((long long)(*(s32 *)(src + 4) % 30) * 100) / 30;
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
