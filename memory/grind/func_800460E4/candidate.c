/* [s10] SESSION 10 (2026-08-25), modality `escalation` -> OWNER-DIRECTIVE EXECUTION.
 * THIS IS THE APPLIED, MEASURED-ZERO BODY. It is in src/text1a_c2.c right now.
 *
 * MEASURED THIS SESSION, twice, `& tools/wteng.ps1 main sandbox func_800460E4
 * --disable all`:  score 0, target_insns 248, build_insns 248, scorable true,
 * rules_dropped 10, cheat_asm_stripped 0.
 *
 * WHAT CHANGED vs the [s9] body that sat at 9: exactly one arm. The second
 * switch's `case 3` now reads the stage header's LAST word through a single
 * fresh, block-scoped, once-written/once-read pointer local `hp`:
 *     s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
 *     s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
 * while the -8 word keeps this function's own plain array-index idiom
 * `s0[s3 - 2]`, byte-identically to the `case 13` arm. That is the form
 * banked at rejected/s9-ruling-pending-single-local-named-intermediate-248-0.c
 * and the form the owner ruled YES on (docs/grind/decisions.md, 2026-08-25
 * "OWNER RULING: YES" entry at :11006): it is an APPLICATION of the frozen
 * named-intermediate family (.claude/rules/no-new-park-categories.md:193-214;
 * in-repo SOTN exhibit docs/reference/sotn-construct-index.md:113 =
 * sotn-decomp src/weapon/w_037.c:300, PSX). The 2026-08-25 09:06 /s-axis
 * closure is superseded for THIS EXACT FORM ONLY; everything else it closed
 * (volatile casts, alias-rename second handles, the pm2/pm1 two-pointer form,
 * inlined cast-deref respellings, multi-word respellings, function-invented
 * multi-set carriers) remains banned and none of it is present here.
 *
 * ALSO IN THIS BODY, inherited unchanged and re-verified at 0:
 *  - the FAKE delta-rebase detour on `s1`'s default initialization. Session 10
 *    re-measured the control: collapsing it to the direct `s1 = s4;` keeps
 *    build_insns at 248 but the score goes 0 -> 32. It is load-bearing AND
 *    non-materializing, i.e. it satisfies the extra prerequisite of the
 *    2026-07-01 dead-store scope extension (.claude/rules/dead-store-fake-exception.md:32).
 *    Banked control: rejected/s10-control-chain-extender-removed-248-32.c.
 *  - the `(s3 << 2) + (s32)s0` mainline base for `a0_ptr` (the `&s0[s3]` and
 *    `(u8 *)s0 + (s3 << 2)` spellings each cost exactly one diff:
 *    `addu v0,s0,v0` vs target's `addu v0,v0,s0`).
 *  - canonical `g_stage_id` / `g_stage_variant`. WHEN APPLYING THIS FILE, the
 *    two file-top lines `extern s16 D_80099478;` / `extern s16 D_8009947A;`
 *    MUST be deleted (session 10 deleted them); leaving them gives one object
 *    two C handles, which reads as an alias-rename.
 *
 * REMAINING WORK TO COMPLETED-C: this function still carries 10 regfix/asmfix
 * rules on main from its asm-until-matched deferral. The honest body needs
 * none of them (rules_dropped 10 with score 0). Retiring them is `retire`,
 * an operator/driver step outside a grind session's allowed surface.
 * Self-vet for this body: memory/grind/func_800460E4/self_vet.md.
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
        if (g_stage_id == stage_id) {
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

    g_stage_id = (s16)stage_id;
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

    g_stage_variant = 0;
    /* FAKE: live default init of s1 routed through a delta-rebase detour that
       combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c
       reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare
       priority above the s2 pointer so allocation order matches target,
       lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */
    s1 = (s32 *)((s32)s4 - (s32)s0);
    s1 = (s32 *)((s32)s1 + (s32)s0);
    switch (stage_id) {
    case 3: {
        /* FAKE: fresh once-written/once-read pointer intermediate naming the
         * address of the stage header's last word, mechanism: expand-time
         * MEM_IN_STRUCT_P (expr.c:4567-4577) -> sched.c anti_dependence
         * exemption -> sched1 load/store order, lever-exhaustion:
         * memory/grind/func_800460E4/hypotheses.md + evidence.md [s1]-[s8r] */
        s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        g_stage_variant = 1;
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
        g_stage_variant = 1;
        break;
    case 34:
        s1 = s2;
        g_stage_variant = 1;
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
