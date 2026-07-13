/* REJECTED (s2, structural) — sandbox --disable all = 6 (93/93 insns).
 *
 * ASYMMETRY PROBE. The shape a human would most naturally write if aliasing were
 * a style choice: alias BOTH index bytes that the block reads twice across the
 * call (`u8 *p = &D_8010277C; u8 *q = &D_8010277D;`).
 *
 * Result: 6 — i.e. aliasing D_8010277D COSTS exactly as much as not aliasing
 * D_8010277C.  Target caches ONLY D_8010277C in $s0 and re-lui's D_8010277D at
 * both of its reads (asm/funcs/se_data_set.s lines ~50, ~65).
 *
 * CONCLUSION: the condition/body asymmetry that the 2026-06-22 audit called
 * "the tell" of a coercion construct is a FACT ABOUT THE SHIPPED 1998 BYTES, not
 * an artifact of our spelling.  The original source aliased exactly one of the
 * two adjacent bytes.  Any form that is "consistent" (alias both, or alias
 * neither) is measurably NOT the original.  A uniformly-spelled body cannot
 * match, so consistency is not available as a completion bar here.
 *
 * (Also kills the array-decay idea for the pair: if the original had spelled
 * these as one array `u8 arr[2]` reached through a single pointer, the second
 * read would be `lb 1($s0)`.  Target emits `lui %hi(D_8010277D)` instead, so the
 * two bytes were NOT reached through a common base in the original C.)
 */
void se_data_set(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        u8 *p = &D_8010277C;
        u8 *q = &D_8010277D;

        EndADRSound();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        s1 = func_8005BA8C(s2, D_800A36A4, *(&D_8008E5A8 + (s8)*p), *(&D_8008E5A8 + *q));

        D_800A390E = D_800A36A4;
        D_800A30FC = *(&D_8008E5A8 + (s8)*p);
        D_800A30FD = *(&D_8008E5A8 + *q);

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        bb2_memcpy(s0, s2, s1);
        func_8005BD30((s32)s0 - s2);
    }
}
