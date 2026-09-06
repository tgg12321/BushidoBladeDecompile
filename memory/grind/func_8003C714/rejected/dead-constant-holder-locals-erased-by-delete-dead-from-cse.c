/* REJECTED -- s20 (2026-09-05, structural). 24 distinct dead scalar locals
 * p0..p23, each assigned a distinct small constant at the top of the loop body,
 * never read. Intent: manufacture 24 loop-invariant movables ahead of the
 * 0x91A2B3C5 movable, to drive the loop.c:1719/1904 `threshold -= 3` order term.
 * MEASURED: sandbox 15, build_insns 105 (byte-free, but inert) and the -dL loop
 * dump still reads "Loop from 28 to 242: 62 real insns" with the 0x91A2B3C5
 * movable "moved to 297". insn_count did not move at all and no extra movable
 * appeared: cse1's delete_dead_from_cse (tools/gcc-2.7.2/cse.c:8684) walks
 * backwards deleting every insn whose destination pseudo has a zero use count,
 * so all 24 sets are gone before loop_optimize ever runs. n=8 and n=16 measured
 * identically. */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    s32 p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23;
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
        p0 = 256;
        p1 = 263;
        p2 = 270;
        p3 = 277;
        p4 = 284;
        p5 = 291;
        p6 = 298;
        p7 = 305;
        p8 = 312;
        p9 = 319;
        p10 = 326;
        p11 = 333;
        p12 = 340;
        p13 = 347;
        p14 = 354;
        p15 = 361;
        p16 = 368;
        p17 = 375;
        p18 = 382;
        p19 = 389;
        p20 = 396;
        p21 = 403;
        p22 = 410;
        p23 = 417;
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
