/* s15 KILL (instance) - a chain of DISTINCT loop-invariant locals
 * (w0 = (s32)s0 ^ C0; w1 = w0 ^ C1; ...) intended to create many hoisted
 * movables (each worth -3 threshold) collapses at cse1 to a SINGLE insn:
 * chain lengths 2, 4, 8, 12, 16, 20 and 24 all measure insn_count 57,
 * moved +1, asm_lines 110.  cse1 DOES fold constant-xor chains when every
 * link is loop-invariant (contrast s14's H19, where the same shape on a
 * loop-VARIANT local survived cse1 at +2 per pair).  Freeness of the
 * threshold dial cannot be bought this way.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 w0, w1, w2, w3, w4, w5, w6, w7;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        w0 = (s32)s0 ^ 0x1001;
        w1 = w0 ^ 0x1002;
        w2 = w1 ^ 0x1003;
        w3 = w2 ^ 0x1004;
        w4 = w3 ^ 0x1005;
        w5 = w4 ^ 0x1006;
        w6 = w5 ^ 0x1007;
        w7 = w6 ^ 0x1008;
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = w7;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
