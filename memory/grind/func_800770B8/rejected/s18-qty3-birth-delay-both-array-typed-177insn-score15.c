/* candidate.c - func_800770B8 (src/text1b.c) - s15 synthesis, 2026-09-01
 * Measured THIS form on today's chassis (s15, first action of the session):
 *     sandbox func_800770B8 --disable all = 5   (175 build insns / 175 target insns)
 *
 * THIS BODY IS THE HONEST FLOOR AND IT IS CHEAT-FREE.  It carries exactly one
 * non-semantic construct - the single-level annotated `do { } while (0);`
 * prologue fence - and s15 resolved that construct's four-session-old
 * classification doubt by READING the rule chain rather than by asking again:
 *   - .claude/rules/no-new-park-categories.md:256-271 still prints the stale
 *     2026-06-04 scoping ("applies only to the LABEL_OUTSIDE_LOOP_P / reorg.c
 *     interaction"), BUT that same paragraph designates the dedicated rule as
 *     the authority on the prerequisites ("The dedicated rule
 *     [[do-while-zero-exception]] enumerates the strict prerequisites");
 *   - .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06, the
 *     LATER document) states in its scope sentence that the wrap is "an allowed
 *     pure-C match device for ANY codegen effect incl. register allocation, with
 *     mandatory inline FAKE annotation", and in its body that "The former
 *     scoping to the reorg.c label-note mechanism is abolished";
 *   - its prerequisite 2 explicitly removes exhaustion as a hard gate for
 *     SINGLE-LEVEL wraps, and this body uses one level, annotated inline.
 * So the sched2 mechanism of this wrap is NOT a scoping problem, and the honest
 * floor of func_800770B8 is 5 - not 9.  (s11/s12/s13/s14 all carried this as an
 * open ruling question; it is closed by citation, and no Judge cycle is owed.)
 *
 * RESIDUAL AT FLOOR 5 - two classes, both measured dead
 * ----------------------------------------------------
 *   class B, rows 35-36: ours `sw $0,0x30($17) / sh $0,0x34($17)`, target
 *     `sw $zero,0x30($v0) / sh $zero,0x34($v0)` - the two stores go through the
 *     p_old copy instead of the raw func_8006E49C result pseudo.  FORECLOSED
 *     three times over (s7/s8/s9 dumps on the floor-9 chassis, s14 on the
 *     floor-4 chassis): every spelling that actually reaches the raw pseudo
 *     collapses the function to 170 instructions, FIVE FEWER than the target's
 *     175, because flow.c then deletes the copy and its four dependents.
 *   class C, rows 62-64: ours `addu $2,$2,$3`, target `addu $v1,$v1,$v0`.
 *     Reachable ONLY by naming the addend first in p_6a's address (the "flip"),
 *     and the flip costs +24 rows of loop-head collateral (score 29).  The ONLY
 *     construct ever measured to repair that collateral is the arithmetic
 *     identity `(t0 * 4) >> 1` for group A's index, which the Judge FAILED on
 *     2026-09-01 (docs/grind/decisions.md, 07:23 entry) as a byte-materializing
 *     chain-extender.  Banked at rejected/s14-classD-identity-detour-JUDGE-
 *     FAILED-2026-09-01.c; do not respell it in any form.
 *
 * s15 NEGATIVE RESULTS (38 fresh builds; full detail in evidence.md [s15]):
 *   - The Judge's own suggested replacement - "group A addressed through a cursor
 *     genuinely derived from group C's t0*4 pointer" - is MEASURED DEAD (8
 *     builds).  Every pointer-cursor spelling either materialises the pointer
 *     subtraction (`>>1` form 36/176, `- (s16 *)base` form 36/176: +1 insn the
 *     target does not have) or is byte-worse (`/2` 54/178, `rowC - t0*2` 51/175).
 *     GCC 2.7.2 does not fold the difference back to a shared shift, so a
 *     truthfully-derived cursor cannot supply the dependence edge for free.
 *   - Type-forced derivation (frontier item 1c) is DEAD (20 builds): array-typing
 *     group A (`*((s16 *)(base + 0x10) + t0)`) is byte-neutral under the flip
 *     (29) and worse without it (31); array-typing group C as
 *     `((s16 (*)[2])(base + 0x40))[t0]` costs an insn (15-18 at 176); both
 *     together 33-37 at 176.  Neither creates the t0*4 -> t0*2 edge.
 *   - The operand-order flip is MECHANISM-INDEPENDENT: casting the ADDEND to a
 *     pointer (`(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A)`), which reaches
 *     pointer_int_sum's ptrop slot by a completely different route than the
 *     `(s32)` cast, is BYTE-IDENTICAL to the (s32) flip (29/175 on both the
 *     unflipped and flipped bases).  Array-typing the 5-element rows
 *     (`((s16 (*)[5])(D_800A36A0 + 0x6A))[t0]`) is the flip plus one insn
 *     (9/176).  Eleven flip spellings across s6/s13/s15 now collapse onto exactly
 *     one build.
 *
 * s16 NEGATIVE RESULTS (128 fresh builds + a whole-corpus census; detail in
 * evidence.md [s16]):
 *   - The class-B "2+2 split" is UNIQUE in the executable.  An exhaustive census
 *     of all 1,435 asm/funcs/*.s (tmp/grind/func_800770B8/s16/census.py and
 *     census_broad.py) finds exactly ONE call-result copy whose copy register AND
 *     raw $v0 are both used as store bases afterwards: this function.  All four
 *     sibling call sites of func_8006E49C store through the raw $v0 with no copy
 *     retained.  The s15 frontier's only reserved re-opening path for class B -
 *     "find a sibling that names the source shape" - is measured non-existent.
 *   - Procedural factoring is byte-transparent (8 builds).  `static inline`
 *     helpers for the post-call header init that take the ALREADY-ASSIGNED pointer
 *     are byte-identical to this body (5/175); ones that take the CALL RESULT as a
 *     parameter collapse to 170 insns exactly as s7/s8's two-local forms did, since
 *     an inline parameter is the same pseudo-to-pseudo copy make_regs_eqv kills.
 *   - Local declaration ORDER is inert: all 120 permutations of the five top-level
 *     locals measure 5/175, one single distinct build.  GCC 2.7.2 numbers pseudos
 *     at first RTL emission, not at declaration, so the pseudo-number tie-break in
 *     cse.c make_regs_eqv / local-alloc allocno ordering is not C-controllable here.
 *
 * s17 NEGATIVE RESULTS (12 fresh builds + a compiler-source enumeration; detail in
 *   evidence.md [s17]):
 *   - CLASS B'S MECHANISM IS NOW DEMONSTRATED AND PRICED, not merely unfound.  Under
 *     cse pass 2 (`after_loop = 1`) an extended basic block is terminated by exactly
 *     two things - a CODE_LABEL or a NOTE_INSN_SETJMP (tools/gcc-2.7.2/cse.c:8038-
 *     8063), so no note/scope/wrap/inline fence can ever break it.  With a surviving
 *     CODE_LABEL placed between the copy-based stores and the raw-result stores, plus
 *     the s8 make_regs_eqv canonical promotion, the target's exact 2+2 split IS
 *     emitted (rejected/s17-classB-split-DEMONSTRATED-join-label-177insn-score30.c:
 *     `move $17,$2 / sw $17,0($28) / sw $18,4($17) ... sw $0,48($2) / sh $0,52($2)`).
 *     It costs 177 insns.  A label with no live reference is demoted to
 *     NOTE_INSN_DELETED_LABEL by jump.c pass 1 (dump-proven, A3 byte-identical to A2),
 *     so a free label does not exist; a label with a live reference costs its branch
 *     (176 s9, 177 B1, 177 C1).  This function has ZERO insn slack (175 = 175), so
 *     class B is foreclosed BY PRICE.
 *
 * INHERITED, STILL BINDING (do not re-derive): s6 (19 address spellings),
 *   s9 (3234-atom sched_solver sweep, 0 hits), s10 (full struct rewrite 178 insns),
 *   s11 (63-position single-wrap sweep + 18 nested), s12 (24/24 store-group orders,
 *   8 group spellings, 12 hoists, 79+79 second-wrap), s13 (315 loop-shape builds,
 *   17 inner-address spellings, 240 five-element orders), s14 (33 demand-order,
 *   13 dependence-direction, 8 reference-count, 79-position second-wrap).
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
        ptr = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ((s16 (*)[2])((u8 *)&D_800A35D0))[t0][1] = 0;
        ((s16 (*)[2])((u8 *)&D_800A35D0))[t0][0] = 0;
        ((s16 (*)[2])(base + 0x40))[t0][1] = 0;
        ((s16 (*)[2])(base + 0x40))[t0][0] = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            s16 *p_6a = (s16 *)(D_800A36A0 + (t0 * 10) + 0x6A);
            s16 *p_7e = (s16 *)(D_800A36A0 + (t0 * 10) + 0x7E);
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
