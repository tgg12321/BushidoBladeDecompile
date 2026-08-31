/* func_80045878 (src/text1a_c.c) -- BEST FORM, s13 (2026-08-31, structural).
 *
 * sandbox func_80045878 --disable all  ==  score 0, build_insns 108 ==
 * target_insns 108, rules_dropped 0.  MEASURED THIS SESSION on today's tree.
 *
 * This is the THIRD independent byte-exact pure-C form of this function, and
 * the first one that does NOT contain the multi-write fresh value carrier `c`
 * that the Judge FAILed on 2026-08-30 21:30.  The previous best form is
 * preserved verbatim as candidate_s10_multiwrite_carrier.c (and the second,
 * call-argument-anchored spelling as alt_anchor_e1.c); both are superseded by
 * this file, which is byte-exact with strictly fewer and cheaper constructs.
 *
 * ==================================================================
 * WHAT IS DIFFERENT, AND WHY IT WORKS (the s13 result)
 * ==================================================================
 *
 * Sessions s8-s12 believed a five-step proof that no compliant form could
 * exist.  Step (1) of that proof said: the tail base copy `addu $v0,$s1,$zero`
 * survives cse ONLY for a pseudo that is mentioned outside the tail block,
 * and such a mention makes flow.c mark the pseudo REG_BLOCK_GLOBAL, which
 * routes it to global_alloc -- which runs AFTER local_alloc, so any block-local
 * tail quantity takes $v0 first and the base is pushed to $a0.  s12 concluded
 * that the base can never be block-local, hence both tail scratch values need
 * a second call-free multi-block pseudo, hence a fresh multi-write carrier.
 *
 * THAT STEP IS FALSE, and this file is the disproof.  The two facts it missed:
 *
 *  (a) cse.c:836-864 `make_regs_eqv` decides canonicality from
 *      `regno_first_uid` / `regno_last_uid`, which are computed by
 *      `reg_scan` (regclass.c) over the RTL AS IT STANDS BEFORE cse.  A
 *      mention that is DELETED later still counts.
 *  (b) flow.c's `life_analysis` -- which is what actually writes
 *      `reg_basic_block[]` -- deletes dead stores as it walks, so a mention
 *      that is dead never makes the pseudo REG_BLOCK_GLOBAL.
 *
 * So a single DEAD store to a fresh pointer local, placed in the entry block,
 * separates the two properties that s12 believed were welded together:
 *   - reg_scan sees `p` first mentioned in the entry block, so at the tail
 *     `uid_cuid[regno_first_uid[p]] < cse_basic_block_start` holds, `p` (not
 *     `s1`) becomes the qty's canonical register, `p`'s uses are NOT rewritten
 *     to `s1`, and the copy insn survives to the emitted code;
 *   - flow deletes the dead store, so `reg_basic_block[p]` is block 13 and
 *     local_alloc -- not global_alloc -- allocates `p`.
 * local-alloc.c:1641 `qty_compare` then ranks the block-13 quantities by
 * floor_log2(n_refs)*n_refs*size/(death-birth): `p` has 7 refs over 10 insns
 * and outranks both tail scratches (2 refs each), so `p` takes $v0 (ascending
 * regno; mips.h defines no REG_ALLOC_ORDER) and the scratches take $v1.
 * That is exactly target's seating, with no carrier local anywhere.
 *
 * Measured, this session, all `sandbox --disable all`, rules_dropped 0:
 *   this file (dead init + named intermediate `t`)      108 insns  score 0
 *   same, with `p = 0;` as a statement not an initializer 108 insns score 0
 *   same, without `t` (a0+3 written inline)             109 insns  score 10
 *   same, without the dead init                         107 insns  score  9
 *      (copy folded and deleted -- the s12 prediction, confirmed)
 *   dead init replaced by a same-value `p = s1;` re-store
 *      in the join block / else arm / then arm      109/107/107  score 16/9/9
 *      (a same-value re-store creates the p==s1 equivalence early, so cse
 *       propagates s1 and the tail copy dies anyway -- the dead store MUST
 *       carry a different value)
 *   dead init replaced by a REAL once-read use of `p` in the else arm
 *      (the 0x1A88 call argument) that combine did not fold away
 *                                                       108 insns  score 10
 *
 * ==================================================================
 * WHY THIS FILE IS NOT SUBMITTED AS candidate-ready
 * ==================================================================
 * Two constructs are in the diff:
 *   1. `s16 *p = 0;` -- a dead store to a LOCAL whose stored value is never
 *      read.  That is verbatim the scope of the sanctioned family in
 *      .claude/rules/dead-store-fake-exception.md ("dead store to a local:
 *      dest = val1; where dest is never read"), FAKE-annotated below.
 *   2. `s32 t; t = a0 + 3; p[11] = t;` -- a fresh, once-written, once-read
 *      named intermediate carrying a real value that appears in target's
 *      bytes as `addiu $v1, $s2, 0x3`.
 * BUT the Judge's standing constraint for THIS function (2026-08-30) reads
 * "No fresh (newly invented) local may be written more than once to serve as
 * a register/allocation carrier, by any spelling or name; keep the s3
 * join-block placement and the v0 tail-base reuse".  `p` is a fresh local
 * written twice (the dead 0, then the real base), and this form necessarily
 * DROPS the v0 tail-base reuse, because the mechanism requires the base
 * pseudo to be block-local and `v0` is inherently multi-block.  s13 therefore
 * returned `ruling-request` rather than submitting against the letter of a
 * binding constraint.  See memory/grind/func_80045878/self_vet.md for the
 * full six-test vet and the exact question.
 *
 * Apply verbatim over the INCLUDE_ASM line in src/text1a_c.c.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    /* FAKE: dead store to the local `p`, mechanism: regclass.c reg_scan fixes
       regno_first_uid before cse.c:836-864 make_regs_eqv runs, so `p` stays
       canonical and the tail base copy survives, while flow.c life_analysis
       deletes this store and leaves reg_basic_block[p] block-local so
       local-alloc.c:1641 qty_compare seats it in $v0 ahead of the tail
       scratches, lever-exhaustion: memory/grind/func_80045878/hypotheses.md
       (s1-s13, 46 banked rejected forms) */
    s16 *p = 0;
    s32 t;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        func_80045600(a0, 0x1A88 + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && (s1[3] != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    p = s1;
    /* FAKE: named intermediate for a real value that target materialises as
       `addiu $v1, $s2, 0x3`, mechanism: keeps the a0+3 truncation out of the
       HImode cse temp that otherwise outranks `p` in local-alloc.c:1641
       qty_compare (measured: without it the base loses $v0, 109 insns /
       score 10), lever-exhaustion: memory/grind/func_80045878/hypotheses.md */
    t = a0 + 3;
    p[11] = t;
    p[2] = a0;
    p[4] = a1;
    p[10] = a0;
    p[8] = a0;
    *((s32 *) (((s32) p) + 0x18)) = 0x8000;
}
