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
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        /* FAKE: same-value dead store restoring p_old's pre-call value (p_old is
           never read again). Effect: it denies local-alloc's combine_regs its
           reg_n_deaths == 1 precondition on the p_old pseudo, so the 0x30/0x34
           clears keep the target's base register instead of collapsing onto the
           freshly returned pointer.
           mechanism: GCC 2.7.2 local-alloc.c:472 (combine_regs / block-quantity
           grant gated on reg_n_deaths == 1).
           lever-exhaustion: hypotheses.md class B, s1-s31 (31 sessions of
           store-base spellings), re-measured negative on three differing chassis
           in s37 (k1-k6) and ablation-confirmed load-bearing here in s39
           (removing it costs 2 points: g1 0 -> g1a 2). */
        p_old = prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    t0 = 0;
    do {
        u8 *base = D_800A36A0;
        u8 *ptr;
        u8 *dp;
        u8 *sym;
        u8 *ap;
        a2 = 0;
        ap = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ap + 0x10) = 0;
        *(s16 *)(ap + 0x8) = 0;
        *(s16 *)(ap + 0xC) = 0;
        *(s16 *)(ap + 0x14) = 0;
        *(s16 *)(ap + 0x3C) = 0;
        sym = (u8 *)&D_800A35D0;
        dp = sym + (t0 * 4);
        *(s16 *)(dp + 2) = 0;
        *(s16 *)(dp + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            u8 *q = (u8 *)((t0 * 10) + (s32)D_800A36A0);
            s16 *p_6a = (s16 *)(q + 0x6A);
            s16 *p_7e = (s16 *)(q + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
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
                sym = (u8 *)&D_800A35D0;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}
