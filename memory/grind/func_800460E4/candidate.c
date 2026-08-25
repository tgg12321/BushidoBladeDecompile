/* candidate for func_800460E4 - session s4 2026-08-25
 * sandbox --disable all == 0 (248/248 insns, 10 regfix rules dropped in
 * scoring), measured with annotations in place. BOTH prior layer-1 blockers
 * are resolved:
 *   - the BANNED arg1/s1 whole-function merge stays absent (s32 *s1; is the
 *     real carrier, arg1 never written past the fp_ptr block);
 *   - the BANNED case-3 volatile cast is REMOVED. Its 3 target insns are
 *     kept honestly by fresh pointer intermediates pm2/pm1 (plain-var derefs
 *     emit non-struct MEMs, so sched.c raises REG_DEP_ANTI edges from both
 *     loads to the D_8009947A store, forcing target's load/store order and
 *     with it the seat assignment that defeats the case-34 cross-jump tail
 *     merge). Dump-proven: dumps/text1a_c2.sched insns 312/315/320.
 * Two FAKE constructs remain, both annotated: the s1 chain-extender ([s3])
 * and the pm2/pm1 named intermediates ([s4]). See evidence.md [s4] +
 * self_vet.md. Exact copy of the src/text1a_c2.c body at measurement time.
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
        {
            s32 *ptr = (s32 *)((s3 << 2) + (s32)s0);
            /* FAKE: fresh once-written/once-read pointer intermediates for the
               two header reads; the plain-var derefs emit non-struct MEMs so
               sched.c's alias check keeps the D_8009947A store after both
               loads (target order), mechanism: sched.c true/anti-dependence on
               a fixed-symbol store vs non-MEM_IN_STRUCT_P varying load,
               lever-exhaustion: evidence.md [s4] (statement-order sweep P1-P4
               measured flat; perturb.py goal unreachable depth<=3) */
            s32 *pm2 = ptr - 2;
            s32 *pm1 = ptr - 1;
            s32 raw_m2 = *pm2;
            s32 raw_m1 = *pm1;
            D_8009947A = 1;
            s6 = (s32 *)((u8 *)s0 + ALIGN4(raw_m2));
            s4 = (s32 *)((u8 *)s0 + ALIGN4(raw_m1));
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
            D_8009947A = 1;
        }
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
