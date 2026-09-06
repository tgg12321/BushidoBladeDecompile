/* REJECTED -- s20 (2026-09-05, structural). Six dead LOOP-INVARIANT constant
 * divisions e0..e5 = ((s32)s0) / {7,11,13,17,19,23} at the top of the loop body,
 * never read. Intent: each division expands to ~7 invariant single-set pseudos,
 * so each should be a movable moved AHEAD of the 0x91A2B3C5 movable, buying
 * -3 threshold each (loop.c:1719/1904) while also adding to insn_count -- the
 * combined dial predicted a flip at three divisions.
 * MEASURED: sandbox 15, build_insns 105 at n=1..6; the -dL dump still reads
 * "Loop from 28 to 222: 62 real insns" and the 0x91A2B3C5 movable is still
 * "moved". The chains are dead with distinct destination pseudos, so
 * delete_dead_from_cse cascades backwards through the whole chain (including
 * the `mult` of the division magic) and erases them before loop_optimize.
 * Companion forms measured the same session, same result:
 *   distinct-pseudo VARIANT chain t0 = *(s32*)(src+4); t1 = t0/7; ... t10
 *     -> "Loop from 28 to 263: 61 real insns", score 17 (also erased);
 *   single-variable INVARIANT chain t = (s32)s0; t = t/7; ... -> score 15;
 *   mixed chain e_k = ((s32)s0)/D_k accumulated into a dead reused t
 *     -> "Loop from 28 to 230: 62 real insns", score 15.
 * Contrast: s19's d_n10 (single reused variable rooted at the loop-VARIANT load
 * *(s32*)(src+4)) re-measured score 0 on this chassis -- that shape survives
 * because every set of the reused pseudo has a nonzero use count. */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    s32 e0, e1, e2, e3, e4, e5;
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
        e0 = ((s32)s0) / 7;
        e1 = ((s32)s0) / 11;
        e2 = ((s32)s0) / 13;
        e3 = ((s32)s0) / 17;
        e4 = ((s32)s0) / 19;
        e5 = ((s32)s0) / 23;
        src = base + i * 8;
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
    *((u8 *)s0 + 0x30) = D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
