void _SsSndCrescendo(s16 a0, s16 a1) {
    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);
    u16 voll, volr;

    if (--(*(s32 *)(base + 0xA0)) < 0) {
        SS_SCORE_FLAG(a0, a1) &= ~0x10;
    } else if (*(s16 *)(base + 0x4C) > 0) {
        if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
            if (*(s16 *)(base + 0x4A) >= 0) {
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((voll + 1) <= (voll + *(s16 *)(base + 0x4A)))
                    func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll + 1), (u16)(volr + 1), 1);
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                SS_SCORE_FLAG(a0, a1) &= ~0x10;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                SS_SCORE_FLAG(a0, a1) &= ~0x10;
        }
    } else if (*(s16 *)(base + 0x4C) < 0) {
        *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
        if (*(s16 *)(base + 0x4A) >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if (((voll - *(s16 *)(base + 0x4C)) >= 0x7F) &&
                ((volr - *(s16 *)(base + 0x4C)) >= 0x7F)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                SS_SCORE_FLAG(a0, a1) &= ~0x10;
            }
            if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                *(s16 *)(base + 0x48))
                func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - *(s16 *)(base + 0x4C)),
                              (u16)(volr - *(s16 *)(base + 0x4C)), 1);
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
            SS_SCORE_FLAG(a0, a1) &= ~0x10;
        }
        if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
            SS_SCORE_FLAG(a0, a1) &= ~0x10;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
