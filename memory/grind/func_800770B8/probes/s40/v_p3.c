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
    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    p_old = (s32 *)(arg0 + 0x58);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        ((SelWork *)p_old)->f04 = prev;
        p_old = (s32 *)SELWORK;
        ((SelWork *)p_old)->f30 = 0;
        ((SelWork *)p_old)->f34 = 0;
    }
    t0 = 0;
    do {
        SelWork *base = SELWORK;
        /* FAKE: typed row pointer to D_800A35D0 (pointer-alias-fake-exception,
           the `s16 (*p)[] = &D_xxx` re-view), kept so the same-value re-set
           below has a pseudo to re-set; mechanism: see that comment (loop.c
           may_not_move keeps the lui/addiu of the base inside the outer loop).
           lever-exhaustion (memory/grind/func_800770B8/evidence.md): direct
           subscripts with no row pointer 19/175; row pointer without the
           re-set 18/175. */
        s16 (*sym)[2];
        a2 = 0;
        base->f10.half[t0] = 0;
        base->f08[t0] = 0;
        base->f0C[t0] = 0;
        base->f14.half[t0] = 0;
        base->f3C[t0] = 0;
        sym = D_800A35D0;
        sym[t0][1] = 0;
        sym[t0][0] = 0;
        base->f40[t0][1] = 0;
        base->f40[t0][0] = 0;
        base->f68[t0] = t0;
        do {
            SELWORK->f6A[t0][a2] = -1;
            SELWORK->f7E[t0][a2] = 0;
            a2 = (s16)(a2 + 1);
        } while (a2 < 5);
        SELWORK->f5C[t0] = 0;
        SELWORK->f60[t0] = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] | 1);
                sp[t0] += 1;
                /* FAKE: same-value dead store re-establishing sym's own value
                   (sym is never read after the loop body's D stores). Effect: it
                   gives the sym pseudo a SECOND set, in a different basic block
                   from its first, which is what keeps the lui %hi/addiu %lo pair
                   for D_800A35D0 inside the outer loop where the target builds
                   it (rows 49/50/51) instead of hoisting it to the pre-header.
                   mechanism: GCC 2.7.2 loop.c:3040-3041 (count_loop_regs_set sets
                   may_not_move[regno] when a set is the first in the current basic
                   block but the reg was already set in the loop, i.e. it is set in
                   two basic blocks); scan_loop then skips the insn at loop.c:649,
                   so move_movables never sees it. The n_times_set > 1 route to the
                   same gate is unreachable in C here -- s38 proved cse folds a
                   two-statement refinement of the same local back into one set.
                   lever-exhaustion: 38 prior sessions, 8 modalities, 188 banked
                   rejected forms, 33,926 permuter iterations; ablation this session
                   shows removing it costs 18 points (g1 0 -> g1d 18) and that no
                   real-valued second write substitutes for it (h1 41/178, h2 54/170,
                   h3 51/178) nor does a literal self-assign (g6 18). */
                sym = D_800A35D0;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        SelWork *p = SELWORK;
        p->f20.word = 0;
        p->f1C.word = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            p->f64 = (s16)sp[0] - 3;
        } else {
            p->f64 = (s16)sp[1] - 3;
        }
    }
    if (SELWORK->f64 >= 3) {
        SELWORK->f64 = 2;
    }
    {
        SelWork *q = SELWORK;
        q->f00 = arg1;
        q->f65 = 0;
    }
    SELWORK->f67 = 1;
    SELWORK->f66 = D_8009BD20[SELWORK->f67][1];
    D_800A35DC = 1;
    return 1;
}
