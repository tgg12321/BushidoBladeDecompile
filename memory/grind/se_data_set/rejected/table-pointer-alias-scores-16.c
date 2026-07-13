/* REJECTED (s2, structural) — sandbox --disable all = 16 (93/93 insns).
 *
 * MECHANISM PROBE, and it REFUTES the mechanism story banked in
 * rejected/direct-cast-no-address-cache-scores-6.c.
 *
 * That story said: "`la $sN, sym` is emitted only for a movsi of a SYMBOL_REF
 * into a pseudo, i.e. from a C-level `&` bound to a variable."  If that were the
 * whole rule, binding `&` to a variable for the lookup TABLE (D_8008E5A8) would
 * have to materialize the table's address into a callee-save register too (it is
 * live across func_8005BA8C).  It does NOT.
 *
 * Measured (tmp/grind/se_data_set/s2/v07_table_pointer.txt): every one of the
 * four table reads still emits `lui $at,%hi(D_8008E5A8); addu $at,$at,$idx;
 * lbu $rX,%lo(D_8008E5A8)($at)` — the pointer local was constant-folded away
 * entirely.  No `la`, no callee-save cache.  Score got WORSE (16), purely from
 * knock-on register-allocation churn.
 *
 * CORRECTED MECHANISM (this is the durable finding — it is about the USE shape,
 * not the `&`):
 *   - A pointer local dereferenced as a PLAIN SCALAR (`*p`, i.e. MEM(reg))
 *     survives as an address pseudo -> `la $sN, sym` + `lb 0($sN)`.  MEM(reg) is
 *     a 1-insn address; propagating the SYMBOL_REF back in would cost 2 (lui+lb),
 *     so GCC keeps the register.
 *   - A pointer local used as an INDEXED BASE (`t[i]`, i.e. MEM(plus(reg,reg)))
 *     gets folded back to MEM(plus(symbol,reg)), because MIPS addresses
 *     symbol+index natively (lui/addu/lbu %lo) and the pointer buys nothing.
 *
 * This is exactly why the target caches &D_8010277C (read as a plain scalar) in
 * $s0 but re-lui's D_8008E5A8 (used as an indexed table base) at all four of its
 * reads.  The asymmetry in the target bytes is the FINGERPRINT of this rule.
 */
void se_data_set(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        u8 *t = &D_8008E5A8;            /* <-- & bound to a variable, but INDEXED */

        EndADRSound();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        s1 = func_8005BA8C(s2, D_800A36A4, t[(s8)D_8010277C], t[D_8010277D]);

        D_800A390E = D_800A36A4;
        D_800A30FC = t[(s8)D_8010277C];
        D_800A30FD = t[D_8010277D];

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        bb2_memcpy(s0, s2, s1);
        func_8005BD30((s32)s0 - s2);
    }
}
