void _SsSndDecrescendo(s16 a0, s16 a1) {
    s32 *bank;
    u8 *base;
    u16 voll, volr;

    bank = &((s32 *)&_ss_score)[a0];
    base = (u8 *)(*bank + (s16)a1 * 0xB0);

    if (--(*(s32 *)(base + 0xA0)) < 0) {
        *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
    } else if (*(s16 *)(base + 0x4C) > 0) {
        if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
            if (*(s16 *)(base + 0x4A) >= 0) {
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((((u16)voll - 1) >= ((u16)voll - *(s16 *)(base + 0x4A))) ||
                    (((u16)volr - 1) >= ((u16)volr - *(s16 *)(base + 0x4A)))) {
                    if ((voll == 0) || (volr == 0)) {
                        *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
                    } else {
                        func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - 1),
                                      (u16)(volr - 1), 1);
                    }
                }
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x20;
        }
    } else if (*(s16 *)(base + 0x4C) < 0) {
        *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
        if (*(s16 *)(base + 0x4A) >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if ((((u16)voll + *(s16 *)(base + 0x4C)) <= 0) &&
                (((u16)volr + *(s16 *)(base + 0x4C)) <= 0)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
            }
            if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                *(s16 *)(base + 0x48)) {
                if ((voll == 0) || (volr == 0)) {
                    *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x20;
                } else {
                    func_80087770((s16)(a0 | (a1 << 8)),
                                  (u16)(voll + *(s16 *)(base + 0x4C)),
                                  (u16)(volr + *(s16 *)(base + 0x4C)), 1);
                }
            }
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
            *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
        }
        if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
            *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x20;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
