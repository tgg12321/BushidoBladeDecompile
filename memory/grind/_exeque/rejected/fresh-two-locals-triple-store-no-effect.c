/* s3 probe — NOT a cheat, NOT rejected for policy reasons; superseded because
 * it is byte-identical to the simpler direct-assignment form kept in
 * candidate.c (see hypotheses.md H7). Recorded so a future session does not
 * re-spend a measurement re-trying this exact variant.
 *
 * Hypothesis: loading BOTH _que[D_8009BF7C].arg and .count into fresh named
 * locals BEFORE either store (instead of storing immediately after each
 * load) would raise the D_8009BF6C store's INSN_LUID above the .count
 * field's address-recompute chain, breaking the rank_for_schedule
 * class+priority tie in its favor (see the sched.c:2417-2464 mechanism
 * documented in candidate.c). Measured: sandbox score unchanged at 12,
 * build_insns unchanged at 185; objdump of the triple-store region
 * byte-identical to both the mask-reuse form (s2 H5a) and the direct-
 * assignment form (this session, kept in candidate.c) — LUID-relative
 * ordering did not change relative to the .count recompute chain, because
 * both loads are still generated in the SAME relative source position as
 * the mask-reuse form (arg-then-count), just deferring their respective
 * stores by one statement.
 */
s32 _exeque(void) {
    s32 mask;
    s32 arg_val, count_val;

    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    mask = SetIntrMask(0);
    D_8009BF84 = mask;
    if (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        do {
            if (((D_8009BF7C + 1) & 0x3F) == D_8009BF78 && D_8009BE80 == 0) {
                DMACallback(2, 0);
            }
            while (!(*D_8009BF48 & 0x04000000)) {
            }
            _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
            D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
            arg_val = (s32)_que[D_8009BF7C].arg;
            count_val = _que[D_8009BF7C].count;
            D_8009BF6C = arg_val;
            D_8009BF70 = count_val;
            D_8009BF7C = (D_8009BF7C + 1) & 0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        s32 *p = &D_8009BE7C;
        if (*p != 0 && D_8009BE80 != 0) {
            *p = 0;
            ((s32 (*)(void))D_8009BE80)();
        }
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
