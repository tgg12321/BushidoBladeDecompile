/* s15 - NOT a disproven form: this is the ORDINARY-C MAXIMUM of the free
 * insn_count channel, banked here so the next session can start from it.
 * Pure split-init / compound-assignment accumulation ([[split-init-accumulation
 * -sanctioned]], owner feedback: ORDINARY C) on the three division statements
 * plus the `v = *src;` named intermediate takes loop insn_count 56 -> 63 at
 * asm_lines 107 unchanged, and `sandbox func_8003C714 --disable all` measures
 * score 15 / target 104 / build 105 - byte-identical to candidate.c.
 * It is banked as rejected/ only because it does NOT reach distance 0: the
 * requirement is insn_count >= 117 (best movable order) and 63 is the measured
 * saturation point - four further split families (offset intermediates,
 * v = v + 0, c*4 then c*25, modulo-60 re-decomposition) ALL stay at 63.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
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
