/* REJECTED s1 (grind recon, 2026-07-21): const-first single-statement store.
 *
 * This is the literal spelling of the known PsyQ libgpu source for
 * ClearOTagR (sys.c): `ot[0] = 0x00ffffff & (u_long)&Gpu_ot_tag;`.
 * Measured sandbox --disable all = 7 (floor is 6 with Lever B).
 *
 * Kill reason: statement fusion loses the named-intermediate pseudo that
 * aligns the addr register to target's $v1 (same failure mode as the
 * addr-first single-statement form, score 7). Operand order (const-first
 * vs addr-first) does NOT rescue the fused form — cc1 canonicalizes the
 * commutative AND before RTL scheduling. The original compiler evidently
 * reached target bytes from this shape via a different preceding-body
 * pseudo landscape (struct-typed Gpu_dev dispatch), not via the tail's
 * operand order.
 */
u32 *func_8007B844(u32 *ot, s32 n) {
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    *ot = 0xFFFFFF & (u32)&g_gpu_ot_end;
    return ot;
}
