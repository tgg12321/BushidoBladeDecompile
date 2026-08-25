/* candidate for func_800460E4 - session s6 2026-08-25
 * STATE: sandbox --disable all = 4 (248/248, 10 regfix rules dropped in
 * scoring) - and the 4 is a PROVEN scorer artifact, not a byte difference:
 * every instruction matches target (full diff: tmp/grind/func_800460E4/
 * s6_ours2.dis vs asm/funcs/func_800460E4.s); the 4 counted diffs are the
 * four variant stores whose reloc is D_80099478+2 where target's .s spells
 * D_8009947A+0 - the SAME address (0x8009947A), identical %hi/%lo halfwords
 * (0x800A/0x947A, matching target words 0A80013C/7A9420A4). engine/score.py
 * does not mask named-symbol addends, so this spelling cannot read 0 even
 * though the linked bytes are identical.
 *
 * GATING QUESTION (ruling-request, this session's outcome): the body depends
 * on declaring D_80099478/D_8009947A (g_stage_id/g_stage_variant,
 * include/game.h:15-16) as ONE struct:
 *     typedef struct { s16 id; s16 variant; } StageState;
 *     extern StageState D_80099478;
 * The /s COMPONENT_REF store restores the load->store anti-dependence edges
 * in sched.c (anti_dependence exemption needs the other ref NON-struct,
 * sched.c:855-863), giving target's case-3 lw/lw/li/sh order, target seats,
 * and the cross-jump defeat - with case 3 spelled EXACTLY like case 13
 * (no banned construct; all four banned case-3 spellings absent).
 * Aggregate-merge family (no-new-park-categories.md 2026-08-17): prong (a)
 * base-register/stride evidence is ABSENT (all accesses are per-symbol
 * lui/%lo); available evidence = semantic-pair census (symbol_addrs.txt:
 * 151-152), game.h adjacency, paired write in func_8004668C, and the [s4]
 * sched_solver proof that the original MUST have had this dependence edge.
 * Do not resubmit candidate-ready until that ruling lands.
 *
 * Case 34 must stay store-LAST (rejected/case34-store-first-under-struct.c).
 * ONE FAKE construct: the [s3] s1 chain-extender (ruled legitimate 03:53).
 * Exact copy of the src/text1a_c2.c body at measurement time; the typedef
 * above replaces the two s16 externs at the top of the TU.
 */

typedef struct {
    s16 id;      /* current stage index (g_stage_id) */
    s16 variant; /* stage variant flag (g_stage_variant) */
} StageState;
/* extern StageState D_80099478;  (replaces extern s16 D_80099478/D_8009947A) */

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
        if (D_80099478.id == stage_id) {
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

    D_80099478.id = (s16)stage_id;
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

    D_80099478.variant = 0;
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
        {
            s32 *ptr = (s32 *)((s3 << 2) + (s32)s0);
            s6 = (s32 *)((u8 *)s0 + ALIGN4(ptr[-2]));
            s4 = (s32 *)((u8 *)s0 + ALIGN4(ptr[-1]));
            D_80099478.variant = 1;
        }
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
        {
            s32 *ptr = (s32 *)((s3 << 2) + (s32)s0);
            s32 off1 = ALIGN4(ptr[-2]);
            s32 off2 = ALIGN4(ptr[-1]);
            s6 = (s32 *)((u8 *)s0 + off1);
            s4 = (s32 *)((u8 *)s0 + off2);
            func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);
            D_80099478.variant = 1;
        }
        break;
    case 34:
        s1 = s2;
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5]));
        D_80099478.variant = 1;
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
