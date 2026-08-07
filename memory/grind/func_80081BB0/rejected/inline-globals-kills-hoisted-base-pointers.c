/* REJECTED — session 1.  Score 40 (worse than the 18 floor); 85 build insns
 * vs 91 target, i.e. SIX INSTRUCTIONS SHORT.
 *
 * Hypothesis under test: the three loop-hoisted table base pointers that
 * target keeps in callee-saves ($s0 = D_800A125C, $s1 = &D_800A1494,
 * $s3 = D_800A11DC) might arise from loop-invariant motion of inline global
 * references rather than from explicit source-level pointer locals — which
 * would give their pseudos higher register numbers and change the allocno
 * tiebreak order.
 *
 * KILLED.  With the locals removed, GCC 2.7.2 does NOT hoist the symbol
 * addresses out of the goto-loop at all: it emits the %hi/%lo pair at each
 * use site inside the loop body, the three `lui`+`addiu` prologue pairs
 * disappear, and the build is 85 insns against target's 91.  So the three
 * base pointers MUST be explicit source-level locals assigned before the
 * loop.  This is a structural constraint on every future candidate form —
 * do not remove those locals again.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    tslTm2LoadImage_2(&D_800161B8);
    {
        s32 arg4, arg5;
        arg4 = D_800A125C[D_800A1494];
        arg5 = D_800A125C[D_800A1495];
        debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, arg5);
    }
    cdrom_ClearIrq();
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
