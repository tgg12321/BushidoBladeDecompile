/* candidate.c - func_800770B8 (src/text1b.c) - s11 escalation, 2026-09-01
 * Honest floor THIS form, measured s11 on today's chassis:
 *     sandbox func_800770B8 --disable all = 5   (175 build insns / 175 target insns)
 * This SUPERSEDES the s3 body (floor 9) that sessions s3-s10 carried.
 *
 * WHAT CHANGED IN s11 - and why the s4 rejection was stale
 * -------------------------------------------------------
 * s4 measured this exact body at 5 and banked it as
 *   rejected/s4-dw0-fence-plus-pold-move-FLOOR5-CHEAT.c
 * on the reading that an empty `do { } while (0);` acting on the SCHEDULER is a
 * scheduling barrier and therefore a cheat. That reading was correct under the
 * 2026-06-04 mechanism-scoping, and it is NO LONGER the project's rule.
 * `.claude/rules/do-while-zero-exception.md` (owner ruling 2026-07-06) states, in
 * its own frontmatter scope sentence:
 *     "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04
 *      mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an
 *      allowed pure-C match device for ANY codegen effect incl. register
 *      allocation, with mandatory inline FAKE annotation; nested wraps need a
 *      single-level-insufficient justification."
 * and in its body: "The former scoping to the reorg.c label-note mechanism is
 * abolished". Its "Confirmed applications" section records marionation_Exec
 * (2026-07-06) as EXACTLY this situation: a wrap-based candidate reviewer-FAILed
 * under the old scoping, then reinstated by the owner ruling.
 *
 * NOTE A DOCUMENTATION CONFLICT THE NEXT SESSION MUST RESOLVE BEFORE SUBMITTING:
 * `.claude/rules/no-new-park-categories.md:256-271` still carries the OLD
 * 2026-06-04 summary ("applies only to the LABEL_OUTSIDE_LOOP_P / reorg.c
 * interaction"), and the grind role-prompt's frozen-family table quotes that
 * stale summary. The dedicated rule is the designated authority (the stale entry
 * itself says so: "The dedicated rule [[do-while-zero-exception]] enumerates the
 * strict prerequisites"), but the two texts disagree, so a `ruling-request` on
 * this point should precede any candidate-ready submission of this form.
 *
 * THE FORM
 * --------
 * Two edits over the s3 body, both measured:
 *   1. an empty, FAKE-annotated `do { } while (0);` as the first statement
 *      (kills residual class A - the 4 prologue rows);
 *   2. `p_old = (s32 *)(arg0 + 0x58);` moved to AFTER the ClearOTagR call
 *      (unfenced this is WORSE, 10; fenced it is required for 5 - the coupling
 *      is measured, see evidence.md [s4] V9/V10).
 * Everything else is the s3 body verbatim (see the s3 notes preserved below in
 * the ledger, not repeated here).
 *
 * RESIDUAL AT FLOOR 5 - exactly two classes, 5 rows, positional diff re-read s11
 * (tmp/grind/func_800770B8/s3/posdiff.py, 175 vs 175):
 *   class B, rows 35-36 (2 rows): ours `sw $0,0x30($s1) / sh $0,0x34($s1)`,
 *     target `sw $zero,0x30($v0) / sh $zero,0x34($v0)` - the stores go through
 *     the copy (p_old) instead of the raw call-result pseudo. Foreclosed by
 *     measurement + dumps in s7/s8/s9.
 *   class C, rows 62-64 (3 rows): ours `addu $2,$2,$3 / addiu $7,$2,106 /
 *     addiu $5,$2,126`, target `addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A /
 *     addiu $a1,$v1,0x7E` - a local-alloc seat question, see s10's QTYDBG
 *     ground truth and the closed-form priority target.
 *   (row 50 `addiu $2,$2,%lo(D_800A35D0)` is the known LO16 scorer artifact,
 *    [[sandbox-lo16-text-addend-false-distance]], not a real byte diff.)
 *   CLASS A IS GONE at floor 5 - the prologue now matches row for row.
 *
 * s11 NEGATIVE RESULTS (do not re-run; all banked in evidence.md [s11]):
 *   - EXHAUSTIVE single-wrap sweep: an empty `do { } while (0);` inserted at each
 *     of the 63 legal statement positions of this body. Minimum score 5; no
 *     position beats the one already in the form. Log:
 *     tmp/grind/func_800770B8/s11/sweep.log, manifest sw/manifest.tsv.
 *   - Nested wraps (depth 2 and depth 3) at 9 positions in and around the outer
 *     loop body and the p_6a/p_7e inner loop: best 5, and depth 3 is byte-identical
 *     to depth 2 everywhere - loop-note ref weighting does NOT move the contested
 *     class-C quantities. Log: tmp/grind/func_800770B8/s11/nsweep.log.
 *   - The class-C operand flip on THIS chassis: 29 (was 33 on the floor-9 chassis),
 *     with or without an adjacent wrap; flip + a class-B wrap 34.
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
        s32 i2 = t0 * 2;
        s32 i4 = t0 * 4;
        a2 = 0;
        ptr = (u8 *)(i2 + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)&D_800A35D0;
        ptr = i4 + ptr;
        *(s16 *)(ptr + 2) = 0;
        *(s16 *)(ptr + 0) = 0;
        ptr = base + i4;
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
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
