/* func_80045878 (src/text1a_c.c) -- BEST FORM, s14 (2026-08-31, synthesis).
 *
 * SUBMITTED as candidate-ready by s14 under the owner directive on this
 * function's queue item (owner ruling 2026-08-31,
 * .claude/rules/ordinary-c-judge-decidable.md, commit 73bee8f8): the s13b form
 * returns to active for a FRESH layer-1 + default-FAIL Judge adjudication
 * against the amended rules (named-intermediate prong (1) relaxed to
 * "once-written"; dead-store deadness is STORE-level).  Nothing is
 * pre-decided; every prerequisite is verified per instance in
 * memory/grind/func_80045878/self_vet.md.
 *
 * MEASURED BY s14 ON TODAY'S CHASSIS, with this body in place in src/:
 *   sandbox func_80045878 --disable all  ->  score 0, build_insns 108 ==
 *     target_insns 108, scorable true, rules_dropped 0.  (Re-measured after
 *     the annotation edits; unchanged.)
 *   verify-oracle                        ->  "ok": true, "build_matches": true
 *     -- the FULL clean-driver build+link SHA1 still equals the oracle
 *     62efab4f73f992798c43e8c730aa43baa10bb4fa with this C replacing the
 *     INCLUDE_ASM line.  Whole-EXE proof, not an isolated score.
 *   dump.ps1 re-run against THIS form; tmp/grind/func_80045878/dumps/
 *     text1a_c.lreg (function at line 15452) shows
 *       Register 79 used 7 times across 9 insns in block 13 ... pointer.  (= p)
 *       Register 80 used 2 times across 2 insns in block 13
 *       Register 102 used 2 times across 4 insns in block 13
 *     i.e. `p` really is BLOCK-LOCAL (local_alloc, not global_alloc) and its
 *     7-ref quantity really does outrank the two 2-ref tail scratches in
 *     local-alloc.c:1641 qty_compare -- the mechanism, dump-proven rather
 *     than inferred.
 *
 * ==================================================================
 * THE THREE CONSTRUCTS AND THEIR FAMILIES
 * ==================================================================
 *  (1) `s0 = (s32) p;`  -- dead store to the EXISTING local s0; its stored
 *      value is never read on any path (s0 unread on the then-path,
 *      re-assigned in the else arm before every read).  Dead-store family,
 *      store-level deadness (owner Ruling 2, 2026-08-31).
 *  (2) `s16 *p; ... p = s1;` -- FRESH local, written EXACTLY ONCE, read 7
 *      times as the tail base.  Named-intermediate family as amended (owner
 *      Ruling 1, 2026-08-31: once-written, reads unlimited).
 *  (3) `s32 t; t = a0 + 3; p[11] = t;` -- fresh once-written once-read named
 *      intermediate; target materialises the value as `addiu $v1, $s2, 0x3`.
 *
 * Two disclosures are made explicitly in the self-vet rather than argued away:
 * prong (3) of the named-intermediate entry says "the compiler folds the copy"
 * and here the copy is NOT folded (it survives, byte-identical to the target's
 * own `addu $v0, $s1, $zero`), and construct (1) reads `p` before `p` is
 * assigned (the read feeds only a store GCC deletes, so no byte depends on it).
 * Both are the Judge's call.
 *
 * ==================================================================
 * WHY NO SIMPLER BYTE-EXACT FORM EXISTS (s12/s13/s13b, re-affirmed s14)
 * ==================================================================
 * EVERY byte-exact form of this function contains a semantically dead
 * statement, because SOME mention of the tail-base pseudo must exist before
 * the tail block AND be dead:
 *   - it must exist, or cse folds the base copy away  (107 insns / score 9);
 *   - it must be dead, or flow.c marks the pseudo multi-block, the base goes
 *     to global_alloc, a block-local tail scratch takes $v0 first and the
 *     seats invert (s13 H13.4, a real call argument: 108 / score 10);
 *   - with no mention at all there is neither copy nor base: 110 / score 14
 *     (s12's fully constraint-compliant plain tail).
 * That is a property of the residual, not of a spelling choice.
 * Among the three measured byte-exact spellings of the dead mention, this one
 * is the simplest (ruling 3 item 4, simplest-known-form): `s16 *p = 0;`
 * (alt_deadzero_s13.c) gives p a defined value but makes p a TWO-write local,
 * which prong (1) bans; `s16 *q; q = p;` (alt_singlewrite_freshq_s13b.c) has
 * the same indeterminate read PLUS an extra no-semantic-purpose local.
 * Carrying a REAL value in the dead mention is KILLED (109 / score 13,
 * rejected/s13b-deadinit-real-v0-value-109-score13.c).
 *
 * Apply verbatim over the INCLUDE_ASM line in src/text1a_c.c.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    s16 *p;
    s32 t;
    v0 = (s16 *) func_8004574C(a0);
    /* FAKE: dead store to the existing local `s0` (its stored value is never
       read -- s0 is not read on the then-path and is re-assigned in the else
       arm before every use), mechanism:
       regclass.c reg_scan fixes regno_first_uid[p] before cse.c:836-864
       make_regs_eqv runs, so `p` stays canonical and the tail base copy
       survives, while flow.c life_analysis deletes this statement and leaves
       reg_basic_block[p] block-local so local-alloc.c:1641 qty_compare seats p
       in $v0 ahead of the tail scratches, lever-exhaustion:
       memory/grind/func_80045878/hypotheses.md (s1-s14) + 47 banked rejected
       forms in memory/grind/func_80045878/rejected/ */
    s0 = (s32) p;
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
    /* FAKE: fresh once-written named intermediate for the tail base pointer,
       the value target materialises as `addu $v0, $s1, $zero`, mechanism:
       cse.c:836-864 make_regs_eqv keeps `p` canonical so the base copy
       survives and local-alloc.c:1641 qty_compare seats its 7-ref quantity in
       $v0, lever-exhaustion: memory/grind/func_80045878/hypotheses.md
       (s1-s14) */
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
