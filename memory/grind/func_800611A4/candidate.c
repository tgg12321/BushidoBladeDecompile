/* s9 (2026-08-20, rederive) -- SANDBOX 0. `sandbox func_800611A4 --disable all`
 * = 0 (target_insns 43 == build_insns 43, rules_dropped 0, zero regfix/asmfix
 * rules, zero pins, zero inline asm, zero FAKE constructs).
 *
 * THE KEY: the nine-session "v0<->v1 RA wall" was an artifact of the m2c
 * reused-load-temp decomposition that every prior session inherited. The
 * correct idiom is the one the two ALREADY-MATCHED siblings in this same
 * file use verbatim -- func_8006133C (src/text1b.c:3184) and func_8006156C
 * (src/text1b.c:3248), both COMPLETED-C with an identical post-call tail:
 *
 *     p = arg0;
 *     D_800F1140 = *p++;
 *     D_800F1144 = *p++;
 *     D_800F1148 = *p;
 *     D_800A3464 = <mask>;
 *
 * i.e. (1) a SEPARATE walking-pointer local seeded from the param -- NOT
 * `*arg0++` on the param itself (s2's rejected walking-pointer form mutated
 * the param, which costs +1 addiu because arg0/$s0 must stay live for the
 * pre-call D_800F1178 store), and (2) the mask store written LAST in source
 * order, not interleaved. GCC folds p's bumps back into `lw $v0,0/4/8($s0)`
 * (43 insns, no pointer bumps) while the walking-pointer dependence chain
 * gives the load web the $v0 quantity locally; the mask, written after the
 * last load in source, is scheduled INTO the load gaps by sched and lands in
 * $v1 -- exactly target's lui $v1 / ori $v1 / sw $v1 interleave. No session
 * ever tried "separate walking local + mask last" together: s2 tried
 * param-mutating walk + mask mid, s3-s8 tried reused-temp forms with the mask
 * hoisted or interleaved.
 *
 * Pre-call region is unchanged from the s3/s8b form (already proven
 * byte-identical to target). Removed vs s8b: the `new_var` staged temp and
 * the `sp` name (both cosmetic; measured score-neutral -- see form_B/form_C
 * in tmp/grind/func_800611A4/s9/).
 *
 * KILLED this session: rewriting the three u16 param reads as
 * `((u16 *) arg1)[i]` plus `(s32) svec` regresses 0 -> 15 at the same 43
 * insns -- the byte-offset cast spelling of the source reads is load-bearing
 * for the pre-call schedule. Banked as
 * rejected/s9_indexed_u16_reads_regress_to_15.c. */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 svec[3];
    s32 *p;
    s32 *v1 = (s32 *) (&D_800F116C);
    svec[0] = *((u16 *) (((s32) arg1) + 0));
    svec[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    svec[2] = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&svec[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFFFFEF;
}
