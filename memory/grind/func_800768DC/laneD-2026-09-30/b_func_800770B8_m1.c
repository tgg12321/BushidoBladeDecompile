s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    p_old = (s32 *)(arg0 + 0x58);
    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        SelWork *work;
        work = (SelWork *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)work;
        work->f04 = p_old;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
    }
    t0 = 0;
    do {
        SelWork *base = SELWORK;
        s16 (*sym)[2];
        a2 = 0;
        base->f10.half[t0] = 0;
        base->f08[t0] = 0;
        base->f0C[t0] = 0;
        base->f14.half[t0] = 0;
        base->f3C.half[t0] = 0;
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
