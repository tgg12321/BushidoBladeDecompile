/* [s7f] FORENSICS SESSION 7 (2026-08-25) - BODY BELOW IS UNCHANGED.
 * Re-measured on the current chassis: sandbox --disable all = 9 (245/248,
 * rules_dropped 10, cheat_asm_stripped 0). The ledger floor holds.
 *
 * What is new: the sched1 decision that produces the 9-diff residual is now
 * CLOCK-EXACT on disk (tmp/grind/func_800460E4/s7/base2.rank.txt, captured with
 * BB2_PRIO_DEBUG/BB2_RANK_DEBUG/BB2_SCHED_DEBUG on the instrumented cc1), and it
 * says something simpler than [s3] recorded:
 *   - adjust_priority raises EVERY register-setting insn in the block to
 *     max_priority 0x7F000001; the `sh` to D_8009947A is the only insn excluded,
 *     because birthing_insn_p requires a REG dest and a store's is a MEM. The
 *     bottom tier is picked last in sched1's BACKWARD walk = emitted FIRST, so
 *     the store is structurally pinned into the first load's delay slot.
 *   - schedule_select/potential_hazard is NOT involved: zero SELBEST decisions
 *     occur in this block ([s3]'s attribution corrected).
 * Consequence for future sessions: the LUID / RTL-emission-order axis is DEAD.
 * A once-written/once-read two-intermediate form really does make the two `lw`s
 * adjacent in the RTL (luids 4 and 5, ahead of all shift work) and still measures
 * 245/9 - the schedule is invariant under any RTL order C can express, because
 * the store's tier membership does not depend on its position. Banked at
 * rejected/s7f-luid-adjacent-loads-rtl-245-9-schedule-invariant.c.
 * Do not re-derive it. See evidence.md [s7f] and hypotheses.md H36-H38.
 */
/* [s7] FORENSICS-SESSION UPDATE 2026-08-25 (session 7) - READ THIS FIRST.
 * The BODY BELOW IS UNCHANGED (still the [s7]/[s8]/[s9] non-banned form,
 * re-measured 245 insns / 9 diffs this session). What changed is that the
 * residual is no longer a one-lvalue question, and a SECOND, fully independent
 * route to sandbox 0 now exists:
 *
 *  - [s6.5]/[s9.7]'s "there is exactly one closing construct and it is banned"
 *    enumerated only ONE term of sched.c's five-term anti_dependence exemption
 *    (tools/gcc-2.7.2/sched.c:843-864). The exemption also requires the STORE's
 *    MEM to have no /s AND a non-varying address. Both are reachable.
 *  - L4 (give the D_8009947A store MEM_IN_STRUCT_P by typing the global as
 *    `extern s16 D_8009947A[1]` and spelling its four accesses `D_8009947A[0]`)
 *    makes case 3 byte-exact against target with case 3 LEFT IN THIS FUNCTION'S
 *    OWN PLAIN IDIOM (s0[s3-2] / s0[s3-1], identical to case 13) - no pointer
 *    intermediate, no volatile, no carrier, no borrow. Measured 248/19 alone,
 *    and 248/0 once case 34 is written load-before-store (that reorder is
 *    byte-INERT on the plain-scalar chassis, so it carries no coercion content).
 *    `sandbox func_800460E4 --disable all` independently printed score 0.
 *    Form banked at rejected/s7-ruling-pending-array-typed-store-248-0.c.
 *    It is NOT promoted here because scalar-to-one-element-array re-typing of a
 *    global is a FIRST REACH of an unsanctioned family (fails
 *    header-type-correction prong (a); no SOTN-master precedent; no independent
 *    aggregate evidence). Session 7 returned `ruling-request` on exactly that.
 *  - L5 (the sanctioned pointer-alias family: `s16 *vp = &D_8009947A; *vp = 1;`)
 *    reaches the same edge honestly but measures 246/58 and 246/43 - dead on
 *    instruction count, same failure mode as the [s6.4] walking pointers.
 *
 * If the ruling on L4 is YES, this function closes immediately at 0 by applying
 * rejected/s7-ruling-pending-array-typed-store-248-0.c. If it is NO, the
 * conjunction is fully enumerated (L1 banned, L2/L3 impossible, L4 unsanctioned,
 * L5 dead) and the remaining question is routing, not grinding.
 * See evidence.md [s7] and hypotheses.md H31-H35.
 */
/* [s6] FORENSICS-SESSION UPDATE 2026-08-25 - read this with the [s9] note below.
 * Body UNCHANGED (still the [s7] non-banned form; re-measured 245 insns / 9 diffs
 * this session). Four things are now settled that were not before:
 *  1. The MEM_IN_STRUCT_P account is RTL-DUMP-PROVEN, not inferred: t.flow shows
 *     both case-3 header loads as (mem/s:SI (plus (reg 138) (const_int -8|-4)))
 *     and the D_8009947A store as a bare (mem:HI (symbol_ref)); t.sched shows
 *     sched1 emitting 308 -> li 1 -> sh -> 309 -> 310 -> 322 with insn 322's
 *     dependence list naming only its address insn. Born in expr.c expand, taken
 *     by sched1. Dumps: tmp/grind/func_800460E4/s6/dumps_cand/.
 *  2. Only the -4 (SECOND) header read needs /s = 0. Flipping it alone measures
 *     248/0; flipping only the -8 read measures 245/9 (inert). The residual is a
 *     fidelity question about ONE lvalue.
 *  3. The aggregate merge is CONTRADICTED by the binary, not just closed by
 *     ruling: all 12 accesses to D_80099478/D_8009947A across 7 functions use
 *     independent lui %hi/%lo pairs, and func_8004668C emits two separate lui $at
 *     for back-to-back stores to the two symbols.
 *  4. Walking pointers (*hp++ / *--hp) - honest, project-precedented, and they DO
 *     clear /s - are dead here on instruction count: holding the address in the
 *     pointer costs one extra addiu (249 vs target's 248). Measured 249/4, 249/5,
 *     246/25.
 * Closing corollary: matching needs the base register to stay at s0+(s3<<2) AND
 * the -4 read to be non-PLUS. Those intersect in exactly one construct, the banned
 * pm1 pointer intermediate. See evidence.md [s6].
 */
/* [s9] SOLVER-SESSION UPDATE 2026-08-25 — read this first.
 * The body below is UNCHANGED (still the [s7] non-banned form, sandbox 9,
 * 245/248, re-measured this session). What changed is the diagnosis:
 *  - The residual is NOT two independent requirements. local_alloc's qty_compare
 *    replay (s5b/block19_localalloc.md) reproduces TARGET's block-19 seats from
 *    TARGET's instruction ORDER alone: load adjacency shortens the address
 *    quantity's span 7 insns -> 3, priority 7142 -> 16666, so it ranks first and
 *    takes $v0, and $v1/$a0 follow. No multiply-assigned carrier is needed for
 *    anything. The whole carrier axis was aimed at a non-existent goal.
 *  - The rebase/atom-set axis is foreclosed by construction: extra atoms alive
 *    across the loads keep the ADDRESS alive too, so its span never shrinks
 *    (measured on the vC chassis: still 7142, still $v1).
 *  - The one remaining bit is MEM_IN_STRUCT_P on the two case-3 header loads
 *    (expr.c:4567-4577 sets it iff the address subtree is a PLUS_EXPR;
 *    sched.c:831-839 then lets sched1 hoist li/sh between them). Flip it and the
 *    function measures 248/0 — every /s=0 spelling is already banned for this
 *    function (evidence.md [s9.7]).
 * Do NOT spend another session on carriers, borrows, detours, statement order, or
 * scheduler perturbation. See evidence.md [s9] and hypotheses.md H26-H30.
 */
/* candidate for func_800460E4 - session s8 2026-08-25 (SYNTHESIS modality)
 *
 * STATE: `sandbox func_800460E4 --disable all` = **9** (245/248, rules_dropped=10),
 * re-measured THIS session with this exact body in src/text1a_c2.c. This is the
 * best form that carries NONE of the seven banned constructs.
 *
 * WHY THIS AND NOT THE PRIOR CANDIDATE: the [s5]/[s6] candidate measured 0 but was
 * FAILED by the 2026-08-25 07:54 layer-1 review (the off_a/off_b borrow plus the
 * declaration hoist that stages it is a fresh function-invented multi-set carrier);
 * it is banked at rejected/layer1-fail-0825-0754.c and banned_constructs #7. The
 * honest, submittable floor is therefore 9 again.
 *
 * SYNTHESIS OF THE WHOLE LEDGER (s1-s7 + this session), for the next session:
 *  - The entire 9-instruction residual is case-3-local and is ONE sched1 decision in
 *    block 19 at reverse-cycle T-6, dump-quoted (tmp/grind/func_800460E4/s4b2/
 *    base.sched, block 19): two insns are both lifted to 0x7f000001 by sched.c
 *    adjust_priority -> birthing_insn_p and the tie decides whether the two stage-
 *    header loads issue adjacently (target) or with li/lui/sh hoisted between them
 *    (ours).
 *  - TWO independent things must both go right to close it: (A) the loads must issue
 *    adjacently, and (B) each ALIGN4 shift chain must refine IN PLACE so the value
 *    keeps its own hard register ($v1 for the -8 word, $a0 for the -4 word) instead
 *    of running through $v0. (B) is [s5] H23. Every form that achieves (A) and (B)
 *    together so far needs a carrier written more than once, which is exactly what
 *    the 07:19 ruling (fresh carrier) and the 07:54 layer-1 FAIL (hoisted pre-existing
 *    carrier) close, and every genuinely pre-existing function-scope local in this
 *    function is live across the tail's calls, so global_alloc seats it callee-saved
 *    and it cannot land in $v1/$a0 ([s5.2], measured 8/22/40 diffs).
 *  - NEW THIS SESSION: (A) is reachable WITHOUT any multi-set carrier, by changing the
 *    atom set that sched1 sees. Two value-neutral rebase detours on the two header
 *    words (see rejected/s8-rebase-detour-both-words-246-10-adjacent-loads.c and
 *    rejected/s8-xor-rebase-detour-248-10-seats-right-address-v1.c) put target's two
 *    loads back to back and give BOTH header values target's exact hard registers
 *    ($v1 / $a0) - the first non-banned construct in eight sessions to do so. What is
 *    still wrong there is only (B) plus the address register ($v1 where target uses
 *    $v0). Those rebase spellings are research forms, NOT submittable (T1/T2 cheat-
 *    checklist failures: they have no semantic purpose and no human would write them);
 *    they are banked purely as the proof that the frontier-1 lever is real.
 *
 * Everything else about this body is inherited and unchanged from [s7]:
 *  - case 3 and case 13 read the two header words with the SAME indexed form
 *    s0[s3 - 2] / s0[s3 - 1] (the function's own first-switch idiom);
 *  - the p/p2 param-alias locals are dropped and the scaled index is spelled
 *    (s3 << 2) + (s32)s0 (both ruled legitimate by the 03:53 layer-1 review);
 *  - the FAKE s1 chain-extender is load-bearing (removing it: 9 -> 32).
 */
void func_800460E4(s32 stage_id, s32 arg1) {
    s32 *s0;
    s32 s7;
    s32 *s6, *s4, *s2;
    s32 s3;
    s32 *s1;
    s32 *fp_ptr;
    s32 *sp10, *sp18, *sp20;

    s0 = func_800457A0(7);
    if (s0 != NULL) {
        if (D_80099478 == stage_id) {
            s7 = 1;
            switch (stage_id) {
            case 3:
                break;
            case 4:
            case 7:
            case 18:
                s3 = s0[0];
                {
                    s32 off = ALIGN4(s0[s3 - 1]);
                    func_8003EDC0(PTR_OFF(s0, off), 7);
                }
                break;
            case 34:
                s7 = 0;
                break;
            }
            stage_ExecInitFunc();
            if (s7 != 0) {
                return;
            }
        }
    }

    D_80099478 = (s16)stage_id;
    s7 = 7;
    s0 = func_800455AC(7);

    if (arg1 != 0) {
        func_80044F30(stage_id, arg1);
    } else {
        func_80044F30(stage_id, (s32)s0);
    }

    if (arg1 != 0) {
        s3 = *(s32 *)arg1;
        func_80045824(arg1, (s32)s0, ((s32 *)arg1)[s3]);
    }

    {
        s32 off1_raw = s0[1];
        s3 = s0[0];
        s6 = (s32 *)((u8 *)s0 + ALIGN4(off1_raw));
        {
            s32 *a0_ptr = (s32 *)((s3 << 2) + (s32)s0);
            s4 = (s32 *)((u8 *)s0 + ALIGN4(a0_ptr[-1]));
            sp10 = (s32 *)((u8 *)s0 + ALIGN4(s0[2]));
            sp18 = (s32 *)((u8 *)s0 + ALIGN4(s0[3]));
            sp20 = (s32 *)((u8 *)s0 + ALIGN4(s0[4]));

            {
                s32 off = ALIGN4(a0_ptr[0]);
                s2 = (s32 *)((u8 *)s0 + off);
            }

            if (arg1 != 0) {
                fp_ptr = (s32 *)((u8 *)arg1 + ALIGN4(((s32 *)arg1)[s3]));
            } else {
                fp_ptr = s2;
                {
                    s32 off3 = ALIGN4(a0_ptr[1]);
                    func_80045230(PTR_OFF(s0, off3));
                }
            }
        }
    }

    D_8009947A = 0;
    /* FAKE: live default init of s1 routed through a delta-rebase detour that
       combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c
       reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare
       priority above the s2 pointer so allocation order matches target,
       lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */
    s1 = (s32 *)((s32)s4 - (s32)s0);
    s1 = (s32 *)((s32)s1 + (s32)s0);
    switch (stage_id) {
    case 3:
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1]));
        D_8009947A = 1;
        break;
    case 4:
    case 7:
    case 18:
        s1 = s2;
        func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);
        s1 = (s32 *)func_80044670(PTR_OFF(s0, ALIGN4(s0[6])), 8, (s32)s1);
        break;
    case 11:
        snd_SetVolume((s32)s1);
        s1 = (s32 *)((s32)s1 + snd_GetMaxFade());
        break;
    case 13:
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1]));
        func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);
        D_8009947A = 1;
        break;
    case 34:
        s1 = s2;
        D_8009947A = 1;
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5]));
        break;
    }

    func_80044010((s32)s6, 7);
    func_800481E8((s32)fp_ptr, 0);
    func_8003EDC0((s32)s4, 7);
    func_80054410((s32)sp10);
    D_800A33B0 = (s32)sp18;
    D_800A33B4 = (s32)sp20;
    DrawSync(0);
    func_80045600(s7, (s32)s1);
    func_80045694(s7, (s32)func_800466C0);
    stage_ExecInitFunc();
    if (D_800A38DC != 0) {
        if (stage_id != 0x22) {
            func_8004659C(-1);
        }
    }
}
