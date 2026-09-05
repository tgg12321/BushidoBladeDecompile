/* s15 KILL (K42, instance) - the combine MERGE sub-channel, s14's #1 frontier
 * item, yields ZERO free insn_count in this loop.  Re-spelling `(x/30) % 60`
 * as `q = x/30; q - (q/60)*60`, `x % 30` as `x - (x/30)*30`, and `r * 100` as
 * `(r<<6)+(r<<5)+(r<<2)` each measure insn_count 56 and asm_lines 107 -
 * IDENTICAL to candidate.c on both axes.  The decompositions are exactly the
 * RTL the divmod/mult expanders already emit, so there is no pre-combine group
 * to count: nothing is added before loop_optimize and nothing is merged after.
 * Measured tmp/grind/func_8003C714/s15/dumps/r_mod60|r_mod30|r_mul100.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 q;
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
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        q = *(s32 *)(src + 4) / 30;
        dst[0x22] = q - (q / 60) * 60;
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
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
