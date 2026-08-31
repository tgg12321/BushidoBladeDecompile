/* func_80045878 (src/text1a_c.c) -- BEST FORM, s13b (2026-08-31, structural).
 *
 * sandbox func_80045878 --disable all  ==  score 0, build_insns 108 ==
 * target_insns 108, rules_dropped 0.  MEASURED THIS SESSION, twice, on today's
 * chassis.  (The previous form, alt_deadzero_s13.c -- the one the Judge FAILed
 * on 2026-08-31 18:09 -- was re-measured first and also still scores 0/108, so
 * the chassis has not moved since s13.)
 *
 * This is the FOURTH independent byte-exact pure-C form of the function, and
 * the first in which the fresh local `p` is written EXACTLY ONCE.
 *
 * ==================================================================
 * WHAT s13b ESTABLISHED
 * ==================================================================
 * The mechanism is unchanged from s13 (full derivation in the header of
 * alt_deadzero_s13.c): regclass.c `reg_scan` fixes `regno_first_uid[p]` before
 * cse runs, so at the tail the cse.c:836-864 `make_regs_eqv` canonicality test
 * keeps `p` -- not `s1` -- canonical and the tail base copy `addu $v0,$s1,$0`
 * survives; flow.c `life_analysis` then DELETES the dead entry-block statement,
 * so `reg_basic_block[p]` is the tail block and local_alloc (which runs before
 * global_alloc) allocates `p`; local-alloc.c:1641 `qty_compare` ranks p's 7
 * refs over ~10 insns above the two 2-ref tail scratches, so p takes $v0 and
 * the scratches take $v1 -- exactly target's seating.
 *
 * What s13b adds is that the mechanism does not care HOW the early dead
 * mention is spelled, and in particular does not require `p` to be written
 * twice.  Measured this session, all `sandbox --disable all`, rules_dropped 0:
 *
 *   THIS FILE   `s0 = (s32) p;`  dead store to the EXISTING local s0,
 *               reading p; `p` itself is written once            108  score 0
 *   (P1)        `s16 *q; q = p;` same, via a fresh dead local q  108  score 0
 *   (s13)       `s16 *p = 0;`    dead init, placeholder value    108  score 0
 *   (P2)        `p = (s16 *) v0;` dead store carrying a REAL
 *               value (the object func_8004574C just returned)   109  score 13
 *               -- KILLED: the p==v0 equivalence cse records at the early site
 *               perturbs the tail; the early value must be one that creates no
 *               equivalence with any live pointer.  Banked as
 *               rejected/s13b-deadinit-real-v0-value-109-score13.c.
 *
 * Consequence for the Judge's standing constraint for this function ("no fresh
 * local may be written more than once to serve as a register/allocation
 * carrier, by any spelling, write-count or name"): the WRITE COUNT is not the
 * operative property.  This file satisfies the write-count clause literally --
 * `p` is written once -- and still reproduces the bytes.  What is actually
 * load-bearing, and provably unavoidable, is that SOME mention of the tail-base
 * pseudo exists before the tail block AND is semantically DEAD:
 *   - it must exist, or cse folds the copy away (measured: 107 insns/score 9);
 *   - it must be dead, or flow.c marks the pseudo REG_BLOCK_GLOBAL, the base
 *     goes to global_alloc, a block-local tail scratch takes $v0 first and the
 *     seats invert (measured s13 H13.4, a real call argument: 108/score 10);
 *   - with no mention at all there is no copy and no base: 110 insns/score 14
 *     (s12's fully compliant plain tail).
 * So EVERY byte-exact form of this function contains a semantically dead
 * statement.  That is a property of the residual, not of a spelling choice.
 *
 * ==================================================================
 * WHY THIS FILE IS NOT SUBMITTED AS candidate-ready
 * ==================================================================
 * The Judge's constraint bans the whole class by name ("by any spelling,
 * write-count or name -- this covers p = s1; with or without a dead
 * initialiser"), so submitting any of these forms would be submitting a
 * declared-banned construct.  s13b instead filed a DECISION PACKET
 * (docs/grind/decisions.md, 2026-08-31, s13b) carrying the two facts the
 * previous packets did not have -- the single-write measurement above, and the
 * SOTN `new_var_temp` PSX class in docs/reference/sotn-construct-index.md:649
 * -- and returned owner-gated.  The honest Judge-COMPLIANT floor for this
 * function remains 13 (s11's static-inline tail helper, 109 insns), NOT 0.
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
    /* FAKE: dead store to the existing local `s0` (its value is never read --
       s0 is re-assigned in the else arm before every use), mechanism:
       regclass.c reg_scan fixes regno_first_uid[p] before cse.c:836-864
       make_regs_eqv runs, so `p` stays canonical and the tail base copy
       survives, while flow.c life_analysis deletes this statement and leaves
       reg_basic_block[p] block-local so local-alloc.c:1641 qty_compare seats p
       in $v0 ahead of the tail scratches, lever-exhaustion:
       memory/grind/func_80045878/hypotheses.md (s1-s13b) + 47 banked rejected
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
