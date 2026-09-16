/* REJECTED s6 (2026-09-16): 96/241 on the s6 chassis.
 * All six clear sites AND base written with the same inline byte-arithmetic
 * address expression, no bank/a1_off locals, on the hope that cse.c would share
 * within extended basic blocks and re-derive only at the join. It shares
 * nothing: every site re-derives (241 insns). The shared pseudos target holds
 * in $s3/$s2 have to come from source-level locals, not from CSE.
 */
void _SsSndCrescendo(s16 a0, s16 a1) {
    u8 *base;
    u16 voll, volr;

    base = (u8 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0);

    if (--(*(s32 *)(base + 0xA0)) >= 0) {
        if (*(s16 *)(base + 0x4C) > 0) {
            if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
                *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
                if (*(s16 *)(base + 0x4A) >= 0) {
                    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                    if ((voll + 1) <= (voll + *(s16 *)(base + 0x4A)))
                        func_80087770((s16)(a0 | (a1 << 8)), voll + 1, volr + 1, 1);
                } else {
                    func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                    *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
                }
                if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                    *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
            }
        } else if (*(s16 *)(base + 0x4C) < 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
            if (*(s16 *)(base + 0x4A) >= 0) {
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if (((voll - *(s16 *)(base + 0x4C)) >= 0x7F) &&
                    ((volr - *(s16 *)(base + 0x4C)) >= 0x7F)) {
                    func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                    *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
                }
                if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                    *(s16 *)(base + 0x48))
                    func_80087770((s16)(a0 | (a1 << 8)), voll - *(s16 *)(base + 0x4C),
                                  volr - *(s16 *)(base + 0x4C), 1);
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
        }
    } else {
        *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
