/* REJECTED s6 (2026-09-16): 233/186-family results.
 * The s6 chassis with the tail check (and separately the unk40<0 handler)
 * written out in BOTH arms instead of shared via goto. GCC 2.7.2 does not
 * cross-jump the two byte-identical 21-insn tail blocks (verified by reading
 * the two copies side by side in tmp/grind/_SsSndCrescendo/s6/build.txt):
 * duplicated tail = 233 insns, duplicated neg40 handler = 212 insns, vs 199
 * for the goto-shared form. Target emits one copy of each.
 */
void _SsSndCrescendo(s16 a0, s16 a1) {
    u8 *base;
    s32 *bank;
    s32 a1_off;
    u16 voll, volr;

    bank = (s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14));
    a1_off = (s16)a1 * 0xB0;
    base = (u8 *)(*bank + a1_off);

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
                    *(s32 *)(*bank + a1_off + 0x98) &= ~0x10;
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
                    *(s32 *)(*bank + a1_off + 0x98) &= ~0x10;
                }
                if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                    *(s16 *)(base + 0x48))
                    func_80087770((s16)(a0 | (a1 << 8)), voll - *(s16 *)(base + 0x4C),
                                  volr - *(s16 *)(base + 0x4C), 1);
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                *(s32 *)(*bank + a1_off + 0x98) &= ~0x10;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
        }
    } else {
        *(s32 *)(*bank + a1_off + 0x98) &= ~0x10;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
