/* func_8005D554 - candidate at honest sandbox distance 0 (176/176 insns, frame 120 == target),
 * measured s3 2026-09-08 on HEAD main @ 1ddb0a8c with `sandbox func_8005D554 --disable all`.
 *
 * NOT YET SUBMITTED: the closing construct is a ruling-request (s3). The last-6 residual
 * (two identical 3-insn rotations, one per loop half) closes only when the a2-site base
 * `(s32)r4 - K` is computed one statement EARLIER (between `a0_offset += ...` and
 * `s.zero18 = a0_offset;`) into a carrier that is written a SECOND time in the same half
 * with another real, immediately-consumed value (`nv = ret; s.ret = nv;`).  The second
 * write is load-bearing: it makes the carrier's pseudo multi-set, so sched.c's
 * `birthing_insn_p` (reg_n_sets == 1) no longer gives the base insn LAUNCH priority.
 * Every measured variant that keeps the carrier single-set, or that borrows an EXISTING
 * local instead of a fresh one, compiles to 178 instructions instead of 176 (see the s3
 * probe table in evidence.md).  The carrier is therefore a FRESH (invented) local, which
 * `staged-value-reused-variable.md` bound 2 excludes, while its shape is exactly the
 * `s32 tmp;` multi-set reuse shipped at src/code6cac_c2.c:1360-1365 -- hence the ruling.
 */
s32 func_8005D554(s32 arg0, s32 arg1) {
    extern s32 rand(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
    S46C s;
    s32 v3;
    s32 v0;
    s32 i;
    u32 r5;
    u32 r4;
    s32 ret;
    s32 stride;
    s32 a0_offset;
    s32 a2_offset;
    s32 nv;
    s32 nw;
    s32 c100;
    s32 c1;
    s32 *p_b388;
    s32 *p_b390;
    u8 *p_b2e0;
    u8 *p_b2ec;
    s32 *base_offset;

    v3 = D_800A326C;
    v0 = v3;
    if (v3 < 0) v0 = v3 + 3;
    D_800A326C -= (v0 >> 2) * 4;

    ret = arg0;
    if (arg1 > 0) arg1 -= 1;
    i = 0;

    D_800A3418 ^= rand();
    r5 = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    r4 = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    if (i < ((D_800A326C + 1) * 2)) {
        c100 = 0x100;
        c1 = 1;
        stride = arg1 * 0x3C;
        p_b2e0 = (u8 *)&D_8009B2E0;
        p_b2ec = p_b2e0 + 0xC;
        base_offset = (s32 *)(p_b2ec + stride);
        p_b388 = &D_8009B388;
        p_b390 = p_b388 + 2;
        do {
            s.byte28 = 0;
            s.p0 = (void *)(stride + (s32)p_b2e0);
            s.p1 = p_b388;
            D_800A3418 ^= rand();
            i += 1;
            s.c24 = c100;
            s.c20 = c100;
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x19;
            a0_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            nv = (s32)r4 - 0xC;
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            a2_offset = nv;
            a2_offset += ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            nv = ret;
            s.ret = nv;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);

            s.byte28 = 0;
            s.c24 = c100;
            s.c20 = c100;
            s.p1 = p_b390;
            s.p0 = (void *)((u8 *)base_offset + (D_800A3418 & 1) * 0xC);
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x32;
            a0_offset += ((u32)(D_800A3418 * 0x64) >> 0xF);
            nw = (s32)r4 - 0x19;
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            a2_offset = nw;
            a2_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            nw = ret;
            s.ret = nw;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ret;
}
