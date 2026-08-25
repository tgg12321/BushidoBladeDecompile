/* candidate for func_800460E4 - session s7 2026-08-25 (recon re-baseline)
 * STATE: sandbox --disable all = 9 (245/248, 10 regfix rules dropped in
 * scoring), measured THIS session (three times: at HEAD-body chassis 35, at
 * the rebuilt floor-9 form with case-13-style ptr spelling, and at this
 * final uniform indexed spelling - the last two produce BYTE-IDENTICAL
 * objects, diff-verified tmp/grind/func_800460E4/s7/).
 *
 * This is the BEST NON-BANNED FORM. The prior [s6] candidate (StageState
 * aggregate merge, sandbox 4 = scorer artifact) was REFUSED by the
 * 2026-08-25 04:46 ruling (prong (a) fails all three evidence classes;
 * prong (c) unsatisfiable while asm consumers need both symbols) and is
 * banked at rejected/aggregate-merge-refused-0446.c. Honest floor per that
 * ruling and per this session's measurement: 9.
 *
 * The 9 (characterized s7, tmp/grind/func_800460E4/s7/ours_s7.dis vs
 * asm/funcs/func_800460E4.s): all case-3-local. Ours seats the case-3
 * address in $a0 / raw_m1 in $v0 with li/sh(D_8009947A=1) filling the FIRST
 * lw's delay slot; target seats address $v0 / raw_m1 $a0 with li/sh after
 * BOTH loads (target's li v0,1 REUSES the then-dead address register, which
 * is only possible in that order - seats and order are one rigid solution).
 * Our m1-in-$v0 makes case 3's s4 tail (srl/sll/addu $s4,$s0,$v0)
 * byte-identical to case 34's, so jump2 find_cross_jump merges the 3-insn
 * suffix (j into case 34). 9 = 3 merged insns + the seat-swapped registers.
 *
 * EXHAUSTION STATE for the case-3 residual (do not re-probe):
 *  - every load-side non-/s spelling BANNED (volatile cast, pm2/pm1,
 *    inlined cast-derefs, address-form divergence - layer-1 FAILs 03:53 /
 *    04:17 / 04:28);
 *  - store-side /s via aggregate merge REFUSED (ruling 04:46);
 *  - volatile-extern two-prong gate unmet (H14, no IRQ consumer);
 *  - statement-order space swept flat ([s4] P1-P4) and sched_solver-proven
 *    UNREACHABLE by perturbation of our RTL atoms (depth 2 all 584 atoms,
 *    depth 3 the 273 spellable);
 *  - indexed-read respelling s0[s3-2]/s0[s3-1] (this file's spelling)
 *    measured RTL-EQUIVALENT to the block-local-ptr spelling (H15 kill,
 *    byte-identical objects) - adopted here purely because it is the
 *    function's own first-switch idiom (see ALIGN4(s0[s3 - 1]) in the
 *    first switch) - most natural spelling, no block-local ptr, no named
 *    intermediates, zero divergence between case 3 and case 13.
 * Target's case-3 atom multiset is IDENTICAL to ours (15 insns) - the
 * divergence is order+seats only. Frontier: sched_solver classify at this
 * chassis incl. pass 2 / atom-set (not just perturbation) analysis, then
 * whole-function rederivation. See hypotheses.md [s7] frontier.
 *
 * ONE FAKE construct: the [s3] s1 chain-extender (dead-store-fake-exception
 * combine-foldable chain-extender family; ruled legitimate by the 03:53
 * layer-1 review; independently load-bearing - removing it returns the
 * 3-seat rotation, floor 32).
 * Exact copy of the src/text1a_c2.c body at measurement time.
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
