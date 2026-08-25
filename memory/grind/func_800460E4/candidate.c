/* candidate for func_800460E4 - session s4 2026-08-25 (permuter modality)
 *
 * STATE: `sandbox func_800460E4 --disable all` = **0** (248/248 insns,
 * rules_dropped=10, cheat_asm_stripped=0), measured THIS session with this
 * exact body in src/text1a_c2.c. The previous floor was 9 and had been flat
 * for three sessions; the floor-9 body is preserved next to this file as
 * candidate_floor9_prev.c.
 *
 * !!! THIS FORM IS *NOT* CLEARED FOR SUBMISSION. The session that produced it
 * returned `ruling-request`, NOT `candidate-ready`, because the construct that
 * closes the last 9 instructions - reading the two stage-header words as
 *     *(s32 *)((s32)&s0[s3] - 8)   /  *(s32 *)((s32)&s0[s3] - 4)
 * instead of s0[s3 - 2] / s0[s3 - 1] - is a RESPELLING of the address form in
 * state.json banned_constructs #5 (and is adjacent to #4 and #6). Per the
 * grind contract a candidate-ready that re-declares a banned construct is
 * mechanically discarded, so the next session must carry the ruling before it
 * submits. See the ruling_question in tmp/grind/outcome_func_800460E4.json and
 * evidence.md [s4].
 *
 * WHAT CHANGED vs the floor-9 body (the ENTIRE diff, two lines per site, and
 * the SAME two lines at BOTH sites):
 *   case 3  and  case 13:
 *     - s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
 *     - s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1]));
 *     + s6 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s32)&s0[s3] - 8)));
 *     + s4 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s32)&s0[s3] - 4)));
 * Nothing else in the function changed. The inherited FAKE-annotated s1
 * chain-extender (ruled legitimate by the 2026-08-25 03:53 layer-1 review) is
 * unchanged and still load-bearing.
 *
 * WHY THE SPELLING IS UNIFORM: applying the byte-offset read at case 3 ONLY
 * also measures 0 (tmp/grind/func_800460E4/s4/src_r5_zero.c), but that is the
 * exact "address form choice vs. case 13's indexed form" divergence banned as
 * #6. Applying it identically at both sites that read these two header words
 * removes the divergence entirely and still measures 0 - so the form kept here
 * is the maximally self-consistent one, not the minimal-diff one.
 *
 * MECHANISM (measured, not hypothesised - cc1 -dS .sched dumps in
 * tmp/grind/func_800460E4/s4/dumps_qa/ and dumps_bs/): the whole 9-instruction
 * residual was one scheduler tie in case 3's block. With the indexed spelling
 * the ready list at reverse-cycle T-6 carries BOTH the second header load and
 * the first shift of the other ALIGN4 chain, each lifted to 0x7f000001 by
 * sched.c adjust_priority's birthing boost; the tie resolves toward the load
 * and everything else in the block (address seat, both load seats, li/lui/sh
 * placement, the jump2 cross-jump merge) follows. Under this spelling the two
 * chains desynchronise and T-6 carries only ONE boosted insn, which is exactly
 * the schedule target has.
 *
 * PROVENANCE: found by decomp-permuter. Campaign perm_a (label
 * s4-floor9-baseline, 41334 iterations) produced output-110-1, which reached
 * target's exact case-3 order and seats with one extra `addu` (sandbox 2);
 * directed probe r5_addr_cast_m1 (tmp/grind/func_800460E4/s4/probe3.py) then
 * removed the extra insn and reached 0.
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
        s6 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s32)&s0[s3] - 8)));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s32)&s0[s3] - 4)));
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
        s6 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s32)&s0[s3] - 8)));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s32)&s0[s3] - 4)));
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
