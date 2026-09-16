/* REJECTED s6 (2026-09-16): 89/223 on the s6 chassis.
 * Identical to the banked s6 candidate except the five in-arm clear sites use
 * the SS_SCORE_FLAG(a0,a1) macro (the array-subscript re-derive idiom) instead
 * of the cached bank/a1_off pair. GCC does NOT CSE the macro expansion against
 * the preamble spelling, so all five sites re-derive the full address: +24
 * instructions over the banked form. The macro is right ONLY at the shared
 * tail clear (where target re-derives too).
 */
void _SsSndCrescendo(s16 a0, s16 a1) {
    u8 *base;
    s32 *bank;
    s32 a1_off;
    u16 voll, volr;

    bank = (s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14));
    a1_off = (s16)a1 * 0xB0;
    base = (u8 *)(*bank + a1_off);

    if (--(*(s32 *)(base + 0xA0)) < 0) {
        SS_SCORE_FLAG(a0, a1) &= ~0x10;
    } else {
        if (*(s16 *)(base + 0x4C) > 0) {
            if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
                *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
                if (*(s16 *)(base + 0x4A) < 0)
                    goto neg40;
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((voll + 1) <= (voll + *(s16 *)(base + 0x4A)))
                    func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll + 1), (u16)(volr + 1), 1);
                goto tail;
            }
        } else if (*(s16 *)(base + 0x4C) < 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
            if (*(s16 *)(base + 0x4A) < 0)
                goto neg40;
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
            goto tail;
        }
    }
    goto done;
neg40:
    func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
    SS_SCORE_FLAG(a0, a1) &= ~0x10;
tail:
    if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
        *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
done:
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
