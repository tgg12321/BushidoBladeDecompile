/* REJECTED (s4 permuter). The "base-reuse-for-reload" chassis: reuse the `base`
 * variable to hold the *arg0 reload after the partial-add.
 *
 *   s32 partial = base + var_v0;
 *   base = *((s32 *) arg0);          // reuse base's register for the reload
 *   return (partial + *(s16*)(base + 0x40A)) + 0x12C;
 *
 * WHY IT MATTERS (new finding, contradicts s2/s3): in the REAL full-context
 * sandbox this reaches build_insns 43 (== target), breaking the 1-insn
 * shortfall — the load-delay nop appears. s2/s3 concluded only arm-duplication
 * reaches 43 (and that fails cross-jump). A SINGLE-basic-block variable-reuse
 * form reaches 43 in full context.
 *
 * WHY REJECTED: it does NOT lower the floor. It trades the missing insn for an
 * equivalent register-rename residual — sandbox stays 10 (this exact form 11;
 * the a2-hosted variant sandbox 10). The residual is a pure 6-register TAIL
 * swap {partial coalesces with var_v0->$v0 / reload->$a1} vs target
 * {partial coalesces with base->$a1 / reload->$v0}. The permuter (45k iters)
 * only flips that swap via a `base++; base--;` dead-op coercion (score 30,
 * still not 0) — a forbidden dead-computation cheat. No CLEAN sub-floor form
 * exists on this chassis. Also: `base = *arg0` is itself a borderline
 * variable-reuse coercion (defeat-licm/RA-reuse family), FAKE-annotation +
 * layer-2 territory even if it had helped. */
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    {
        s32 partial = base + var_v0;
        base = *((s32 *) arg0);
        return (partial + (*((s16 *) (base + 0x40A)))) + 0x12C;
    }
}
