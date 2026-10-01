s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    /* work holds two values: the entry list pointer (arg0 + 0x58, passed to func_8006E950 /
       func_80076FF8 and stored as the work area's f04), then the work area func_8006E49C returns
       (stored to D_800A36A0). Owner Ruling 11 (reused-local-necessity.md) with owner ruling Q78
       (rules: 6c8c276c3); package: memory/grind/func_800770B8/evidence.md (s41). */
    void *work;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap (do-while-zero-exception). Effect: its NOTE_INSN_LOOP_BEG/END
       pair bounds sched2's region at this point, so the five frame-save stores are not interleaved
       with the first body insns as in the target's prologue order. Measured without it (5) and
       the prior search: memory/grind/func_800770B8/evidence.md (s41). */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    work = (void *)(arg0 + 0x58);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, work);
    r = func_80076FF8(work);
    {
        s32 *list = work;
        work = (void *)func_8006E49C(r, (s32 *)D_800A35D8);
        D_800A36A0 = work;
        SELWORK->f04 = list;
        /* FAKE: dead store (dead-store-fake-exception; owner ruling Q78, rules: 6c8c276c3): work's
           restored value is never read. Effect: cse.c make_regs_eqv puts work and the call's $v0 in
           one quantity with work canonical, so the D_800A36A0 reloads for the 0x30/0x34 clears below
           would become work ($s1); this store takes work out of that class first (cse.c
           delete_reg_equiv), the reloads resolve to the f04 store's reload copy of the call result
           instead, and the clears use $v0 as in the target (0x80077144/48). Dumps (.cse with and
           without it, command lines) and lever exhaustion: memory/grind/func_800770B8/evidence.md (s40,
           s41). */
        work = list;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
    }
    t0 = 0;
    do {
        SelWork *base = SELWORK;
        /* FAKE: typed row pointer to D_800A35D0 (pointer-alias-fake-exception, the
           `s16 (*p)[] = &D_xxx` re-view), kept so the same-value re-set below has a pseudo to re-set
           (loop.c keeps its lui/addiu inside the outer loop). Measured without it:
           memory/grind/func_800770B8/evidence.md (s41). */
        s16 (*row)[2];
        a2 = 0;
        base->f10.half[t0] = 0;
        base->f08[t0] = 0;
        base->f0C[t0] = 0;
        base->f14.half[t0] = 0;
        base->f3C[t0] = 0;
        row = D_800A35D0;
        row[t0][1] = 0;
        row[t0][0] = 0;
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
