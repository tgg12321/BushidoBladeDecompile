/* [s10] CONTROL, REJECTED - the session-10 zero-scoring body with the FAKE
 * delta-rebase detour on s1's default initialization collapsed to the direct
 * `s1 = s4;`. Everything else, including the case-3 `hp` named intermediate,
 * is identical to the applied candidate.
 *
 * MEASURED 2026-08-25 (`sandbox func_800460E4 --disable all`):
 *   score 32, target_insns 248, build_insns 248, rules_dropped 10.
 *
 * WHY IT IS BANKED: it proves two things at once about the detour.
 *  (a) It is LOAD-BEARING even on the new 248-insn chassis - 0 -> 32. The
 *      s9-era claim "removing it: 9 -> 32" was measured on the floor-9 body;
 *      this is the re-measurement on the closing body, and the conclusion
 *      survives the chassis change.
 *  (b) It is NON-MATERIALIZING - build_insns is 248 with the detour and 248
 *      without it, so the fold emits zero bytes. That is the extra
 *      prerequisite the 2026-07-01 dead-store scope extension attaches to
 *      this spelling (.claude/rules/dead-store-fake-exception.md:32-46): a
 *      chain that MATERIALIZES is a real code change and not that lever.
 *      It does not materialize here.
 * Do not re-measure this control; the numbers above are definitive.
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
    s1 = s4;
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
