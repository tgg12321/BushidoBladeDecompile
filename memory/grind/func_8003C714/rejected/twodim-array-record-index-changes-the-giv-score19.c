/*
 * REJECTED (s18b, 2026-09-05). Declaring the record block as
 * `extern u8 D_80106A58[3][8];` and indexing it directly (`src = D_80106A58[i];`,
 * no `base` local) is a cleaner data model than the flat `[24]` array, but it
 * measures `sandbox --disable all` = score 19 at build_insns 105 vs target 104:
 * the row index changes the general-induction-variable loop.c strength-reduces
 * the loop around, so the index-derived pointer pair the target uses is not
 * reproduced. The flat `extern u8 D_80106A58[24];` + `base = D_80106A58;` +
 * `src = base + i * 8;` spelling in candidate.c measures score 0 with the same
 * pun-free declaration property. Do not re-propose the 2-D spelling.
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
        src = D_80106A58[i];
        dst = (u8 *)s0 + i * 4;
                /* FAKE: dead store -- the record's elapsed time expressed in hundredths
           of a second (the record holds 30 Hz frames; the x100 scaling is done in
           64-bit so it cannot overflow), stored into v and unconditionally
           overwritten by `v = *src;` below.  mechanism: the DImode divide expands
           to a __divdi3 libcall block, whose CALL_INSN makes prescan_loop set
           loop_has_call (loop.c:2202) and so halves the LICM hoist threshold at
           loop.c:532 (122 -> 61), which is what leaves the 0x91A2B3C5 magic
           constant materialised in-loop as the target's lui/lw/ori/mult quartet;
           flow.c's libcall_dead_p (flow.c:1827) then deletes the whole block
           before combine and register allocation, so it costs zero emitted bytes.
           lever-exhaustion: memory/grind/func_8003C714/hypotheses.md K1-K54 and
           rejected/ (30 disproven forms) over 18 sessions. */
        v = ((long long)*(s32 *)(src + 4) * 100) / 30;
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
    *((u8 *)s0 + 0x30) = D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
