/* candidate for func_800460E4 - session s4b 2026-08-25 (permuter modality)
 *
 * STATE: `sandbox func_800460E4 --disable all` = **0** (248/248 insns,
 * rules_dropped=10, cheat_asm_stripped=0), measured THIS session with this
 * exact body in src/text1a_c2.c. It supersedes the previous 0-scoring body,
 * whose closing construct (the `*(s32 *)((s32)&s0[s3] - 8)` byte-offset cast)
 * was REFUSED by the 2026-08-25 06:39 ruling as banned_constructs #5 respelled.
 * NOTHING in this body is a banned construct: case 3 and case 13 both read the
 * header words as `s0[s3 - 2]` / `s0[s3 - 1]`, the file's own indexed idiom.
 *
 * !!! NOT CLEARED FOR SUBMISSION YET. This session returned `ruling-request`,
 * not `candidate-ready`, because the ONE new construct that closes the last 9
 * instructions - two FRESH locals each written TWICE (raw byte offset, then
 * refined in place to a word index) - falls in the gap between two sanctioned
 * families and I cannot quote a scope sentence for either:
 *   - named-intermediate (narrow-byte-args-packed-call.md + the 2026-08-17
 *     clarification) requires ONCE-written/ONCE-read; these are twice-written.
 *   - staged-value-reused-variable.md is exactly the right MECHANISM
 *     (adjust_priority -> birthing_insn_p, `reg_n_sets[regno] == 1`) but its
 *     bound 2 requires borrowing a local the function ALREADY has for another
 *     job; these are fresh, and its own parenthetical assumes a fresh
 *     intermediate is single-assigned - which is precisely the case this
 *     construct is not.
 * The ruling question is in tmp/grind/outcome_func_800460E4.json.
 *
 * WHAT CHANGED vs the floor-9 chassis (the ENTIRE diff is case 3):
 *   -   s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
 *   -   s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1]));
 *   +   u32 hidx = s0[s3 - 2];
 *   +   u32 lidx = s0[s3 - 1];
 *   +   hidx >>= 2;  s6 = &s0[hidx];
 *   +   lidx >>= 2;  s4 = &s0[lidx];
 * Case 13 keeps the plain ALIGN4 form and still matches byte-for-byte; the
 * inherited FAKE s1 chain-extender (ruled legitimate by the 2026-08-25 03:53
 * layer-1 review) is unchanged and re-measured LOAD-BEARING (removing it takes
 * the diff from 0 to 32).
 *
 * MECHANISM (measured): the whole 9-instruction residual was one sched1 tie in
 * case 3's block. Two insns - the second header load and the first shift of the
 * other ALIGN4 chain - were both lifted to 0x7f000001 by sched.c
 * adjust_priority's birthing boost and were both ready at reverse-cycle T-6.
 * `birthing_insn_p` only boosts an insn whose destination pseudo has
 * `reg_n_sets[regno] == 1`. Writing each carrier TWICE (load, then in-place
 * shift) clears the boost on both, exactly one boosted insn remains ready at
 * T-6, both header loads issue adjacently, and the address seat ($v0), both
 * load seats ($v1/$a0), the li/lui/sh placement and the jump2 cross-jump
 * decision all follow into target's shape.
 *
 * PROVENANCE: decomp-permuter campaign perm_d (label s4b-v6-twin-248, base
 * score 260, 28327 iterations, 2 finds) produced output-95-1, which reused a
 * value local as the shift carrier and was the first form in this function's
 * history to issue both header loads adjacently WITHOUT any address respelling.
 * Directed probes probe6/probe7/probe9 generalised it: splitting BOTH ALIGN4
 * chains through their own twice-written carrier reaches 0 (probe7 y3), and
 * probe9 k3/k6 are the same thing in its most idiomatic spelling.
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
    case 3: {
        /* FAKE: the two stage-header words are staged in hidx/lidx as raw byte
           offsets and then refined in place to word indices, mechanism: GCC
           2.7.2 sched.c adjust_priority -> birthing_insn_p (reg_n_sets[regno]
           == 1); the second write clears the birthing boost on both carriers so
           only ONE boosted insn is ready at block 19's reverse-cycle T-6 and
           both header loads issue adjacently, as in target,
           lever-exhaustion: memory/grind/func_800460E4/evidence.md [s1]-[s4b] */
        u32 hidx = s0[s3 - 2];
        u32 lidx = s0[s3 - 1];
        s1 = s2;
        hidx >>= 2;
        s6 = &s0[hidx];
        lidx >>= 2;
        s4 = &s0[lidx];
        D_8009947A = 1;
        break;
    }
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
