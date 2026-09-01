/* candidate.c - func_800770B8 (src/text1b.c) - s14 synthesis, 2026-09-01
 * Measured THIS form on today's chassis (s14):
 *     sandbox func_800770B8 --disable all = 4   (175 build insns / 175 target insns)
 * This SUPERSEDES the s11 body (floor 5) that sessions s11-s13 carried.
 * The s11 body is preserved verbatim at tmp/grind/func_800770B8/s3/... and as
 * rejected/s14-prev-floor5-unflipped-ABCD.c; it still measures 5.
 *
 * ############################################################################
 * # CLASSIFICATION IS OPEN ON *ONE* TOKEN IN THIS BODY - READ BEFORE SUBMITTING
 * ############################################################################
 * The floor-4 form differs from the floor-5 form in exactly two source tokens:
 *   (1) p_6a / p_7e written flipped, `(s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A)`
 *       - ordinary C, the operand order the original source must have had (s13);
 *   (2) group A's index written `(t0 * 4) >> 1` instead of `t0 * 2`.
 * Token (2) is arithmetically an identity (it computes t0*2) and is NOT something
 * a human would write from the specification. Under the 6-test cheat checklist it
 * fails T1 (no semantic purpose) and T2 (human-programmer). It may or may not be
 * covered by the frozen SOTN family "opaque arithmetic variables" - note the
 * FORBIDDEN catalog separately lists "`s32 one = 1;` opaque variable to defeat
 * single-bit transform", so the two entries are in tension and this is a
 * first-reach question, not a self-approvable one.
 * THE HONEST, CHEAT-FREE FLOOR OF THIS FUNCTION IS THEREFORE STILL 5 UNTIL A
 * RULING SAYS OTHERWISE, and the correct next outcome for the token as spelled is
 * `ruling-request`, not `candidate-ready`. What is NOT in question is the
 * STRUCTURAL finding this form proves (below): it is the first build in fourteen
 * sessions in which residual class C is closed.
 *
 * WHY THIS FORM MATTERS - CLASS C IS CLOSED FOR THE FIRST TIME
 * -----------------------------------------------------------
 * s6-s13 typed class C (rows 62-64, `addu $2,$2,$3` vs target `addu $v1,$v1,$v0`)
 * as a two-part lock: the p_6a operand flip fixes the RTL plus-operand order but
 * swaps the two loop-head seats (+24 rows of collateral in rows 38-59, score 29),
 * while s12's CABD store-group order fixes the seats but breaks the store rows
 * (score 12). s14 broke the lock from a third direction, the one the s13 frontier
 * named: change what the LOOP HEAD COMPUTES, not how the inner block spells its
 * address. Deriving group A's t0*2 from the t0*4 pseudo makes t0*4 the loop head's
 * first demanded value; with the flip in place, the contested addition then emits
 * `addu $v1,$v1,$v0` with the target's own seats.
 *
 * RESIDUAL AT FLOOR 4 - three classes, positional diff s14
 * (tmp/grind/func_800770B8/s3/posdiff.py, 175 vs 175):
 *   class B, rows 35-36: ours `sw $0,0x30($17) / sh $0,0x34($17)`,
 *     target `sw $zero,0x30($v0) / sh $zero,0x34($v0)` - the two stores go through
 *     the p_old copy instead of the raw call-result pseudo. Foreclosed by s7/s8/s9
 *     dumps and RE-CONFIRMED on THIS chassis by s14 (4 fresh block spellings: the
 *     three that reach the raw pseudo all collapse to 170 insns, five FEWER than
 *     the target's 175; the fourth is byte-neutral at 6).
 *   class D (NEW, 2 rows, replaces class C): ours 40 `sll $3,$5,0x2` / 41 `lw $4`
 *     / 42 `sra $2,$3,0x1`, target 40 `sll $v0,$a1,1` / 41 `lw $a0` / 42
 *     `sll $v1,$a1,2`. Same three slots, same two registers by role ($3/$v1 = t0*4,
 *     $2/$v0 = t0*2), same lw position - only the ORDER of the two shifts and the
 *     opcode of the second differ, because our t0*2 is derived from t0*4 while the
 *     target computes both directly from t0. This is the price of token (2) and it
 *     is the whole remaining gap outside class B.
 *   (row 50 `addiu $2,$2,0` is the known LO16 scorer artifact,
 *    [[sandbox-lo16-text-addend-false-distance]], not a real byte diff.)
 *   CLASS A (prologue) and CLASS C (rows 62-64) are both GONE. Rows 43-64 now match
 *   the target row for row.
 *
 * s14 NEGATIVE RESULTS (do not re-run; full detail in evidence.md [s14]):
 *   - 33-build loop-head demand-order sweep on three bases (F=5, flipped-ABCD=29,
 *     flipped-CABD=12) x 11 perturbations: only the t0*4-derivation moves anything.
 *     Group C into the integer domain, group A into the pointer domain, group B in
 *     one step, `t0 << 2`, group D hoisted, and the 0x5C/0x60 pair hoisted to the
 *     top or to the inner-loop boundary are all neutral-or-worse on every base;
 *     hoisting the pair is catastrophic (50/71, and 173/174 insns).
 *   - 13-build dependence-direction sweep: the direction is asymmetric. t0*2
 *     derived from t0*4 gives 4; t0*4 derived from t0*2 (`(t0*2)*2`, `(t0*2)<<1`)
 *     gives 29, i.e. nothing at all. `(t0 * 4) / 2` folds straight back to t0*2 in
 *     fold() and measures 29 - the shift is load-bearing, the division is not.
 *     `>>`, `(u32)>>`, and a named `s32 i4 = t0*4;` + `i4 >> 1` all give 4.
 *   - 8-build reference-count sweep: bumping reg_n_refs on the t0*4 pseudo WITHOUT
 *     creating the dependence does not work. Spelling group C's two stores with the
 *     index inline (two references instead of one through `ptr`) gives 25; the same
 *     on group B gives 35/177; both together 12/177. So it is the DEPENDENCE, not
 *     the raw reference count, that reverses the shift birth order.
 *   - 79-position second-wrap sweep on THIS chassis: minimum 4, reached at 34 of the
 *     79 positions (all byte-identical to the no-second-wrap build); nothing below 4.
 *     A second `do { } while (0);` is therefore inert here - the form below carries
 *     exactly ONE wrap, the s11 prologue fence.
 *
 * INHERITED, STILL BINDING (do not re-derive):
 *   - s11: 63-position single-wrap sweep (min 5 on the old chassis) + 18 nested wraps.
 *   - s12: all 24 store-group orders under the flip; 8 group re-spellings; 12
 *     pointer/index hoists; two 79-position second-wrap sweeps.
 *   - s13: 315 loop-shape builds (inner/outer loop shape is byte-neutral under for /
 *     while / do-while; the third loop's `for` is uniquely correct; pointer-walking
 *     the inner loop gives 173 insns); 17 inner-address spellings; 240 five-element
 *     orders (the inner block must stay last); the inner block MUST re-read
 *     D_800A36A0 from $gp (routing it through `base` deletes the second lw).
 *   - The do-while(0) scoping conflict between .claude/rules/no-new-park-categories.md
 *     :256-271 (stale 2026-06-04 reorg.c-only summary) and
 *     .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06, abolishes
 *     that scoping) is STILL UNRESOLVED and still gates the prologue wrap below.
 *
 * Applying this body also requires the two caller-side edits (see
 * tmp/grind/func_800770B8/s3/try.py): the prototype becomes
 * `s32 func_800770B8(s32, s32, s32);` and the call site passes
 * `(s32)&D_8009BD24`. Byte-neutral for the caller.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap. Effect: it anchors a
       NOTE_INSN_LOOP_BEG/END pair at this statement position, which stops sched2
       interleaving the five reload-emitted frame-save stores with the first body
       insns; without it the prologue emits sw $s1 / addiu $s1,$s0,0x58 / lw
       D_800A374C / li 0x1008 / sw $ra where the target emits sw $ra / sw $s1 /
       li 0x1008 / lw D_800A374C / addiu $s1 (residual class A, 4 rows).
       mechanism: GCC 2.7.2 sched.c list scheduler, second pass (sched2, post-reload);
       the notes bound the scheduling region so the save stores cannot be hoisted
       across them. See evidence.md [s9] for the insn-level read-out of the
       unfenced order and [s11] for the measurement.
       lever-exhaustion: hypotheses.md classes A/B/C; s3 (12 statement orderings),
       s5 (honest-loop fence hunt, +11 insns), s9 (exhaustive 3234-atom sched_solver
       depth-1 sweep against the target emission order: 0 hits; the only reachable
       sub-goal needs atoms not expressible in C), s10 (struct-typed rederive 178
       insns), s11 (63-position single-wrap sweep + 18 nested-wrap variants). */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    p_old = (s32 *)(arg0 + 0x58);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    t0 = 0;
    do {
        u8 *base = D_800A36A0;
        u8 *ptr;
        a2 = 0;
        /* FAKE: group A's half-word index written as (t0 * 4) >> 1 rather than
           t0 * 2, so that the t0*4 pseudo (group C / group B / the t0*10 chain)
           acquires a FOURTH reference and is the loop head's FIRST demanded
           derived value. Effect: it is the ONLY measured spelling that makes the
           class-C plus-operand flip on p_6a affordable - with it the contested
           addition emits `addu $v1,$v1,$v0` and rows 43-64 match the target
           exactly (see the s14 read-out below); without it the same flip costs
           +24 rows (score 29).
           mechanism: GCC 2.7.2 cse.c / local-alloc reference counting - the extra
           use raises reg_n_refs on the t0*4 pseudo and reverses which of the two
           shift pseudos is born first, which in turn fixes the dest-coalesce of
           the contested plus in local-alloc (see evidence.md [s14]).
           lever-exhaustion: hypotheses.md classes A/B/C plus s12 (24/24 store-group
           orders, 8 group spellings, 12 pointer/index hoists, 79+79 second-wrap
           sweeps), s13 (315 loop-shape builds, 17 address spellings, 240 five-element
           orders), s14 (33 loop-head demand-order variants, 13 dependence-direction
           variants, 8 reference-count variants, 79-position second-wrap sweep on
           this very chassis - min 4, no position improves it).
           CLASSIFICATION IS OPEN - see the header note; this construct has NOT been
           cleared by a ruling and MUST NOT be submitted as candidate-ready until it
           is. */
        ptr = (u8 *)(((t0 * 4) >> 1) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)&D_800A35D0;
        ptr = (t0 * 4) + ptr;
        *(s16 *)(ptr + 2) = 0;
        *(s16 *)(ptr + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            s16 *p_6a = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A);
            s16 *p_7e = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}
