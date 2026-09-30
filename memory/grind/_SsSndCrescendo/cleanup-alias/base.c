void _SsSndCrescendo(s16 a0, s16 a1) {
    /* FAKE: named intermediate for the bank index, mechanism: combine.c
       try_combine folds the s16->int `ashiftrt:16` together with the scale
       `ashift:2` into one `ashiftrt:14` and emits it at the LATER insn's slot,
       and sched.c rank_for_schedule leaves the independent insns in RTL/LUID
       order - so the index conversion must be its own statement ahead of the
       table-address statement; lever-exhaustion: hypotheses.md H8 - 13
       single-expression spellings of this preamble measured, all score 2
       (v5,v6,v7,wa-we,x1,x2,y1-y4 in tmp/grind/_SsSndCrescendo/s7/). */
    s32 bank_no = a0;
    /* FAKE: C-level pointer alias to the _ss_score global, mechanism: the
       `movsi` of the symbol_ref has to be emitted BETWEEN the surviving
       `ashift:16` and combine.c's folded `ashiftrt:14`; fold() moves the
       constant ADDR_EXPR to operand 1 of any single pointer-sum expression,
       which therefore evaluates the whole index first; lever-exhaustion:
       hypotheses.md H8 - the same 13 measured spellings, all score 2. */
    s32 *score_tbl = (s32 *)&_ss_score;
    s32 *bank = score_tbl + bank_no;
    u8 *base = (u8 *)(*bank + (s16)a1 * 0xB0);
    u16 voll, volr;

    if (--(*(s32 *)(base + 0xA0)) < 0) {
        *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
    } else if (*(s16 *)(base + 0x4C) > 0) {
        if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
            if (*(s16 *)(base + 0x4A) >= 0) {
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((voll + 1) <= (voll + *(s16 *)(base + 0x4A)))
                    func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll + 1), (u16)(volr + 1), 1);
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
        }
    } else if (*(s16 *)(base + 0x4C) < 0) {
        *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
        if (*(s16 *)(base + 0x4A) >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if (((voll - *(s16 *)(base + 0x4C)) >= 0x7F) &&
                ((volr - *(s16 *)(base + 0x4C)) >= 0x7F)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
            }
            if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                *(s16 *)(base + 0x48))
                func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - *(s16 *)(base + 0x4C)),
                              (u16)(volr - *(s16 *)(base + 0x4C)), 1);
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
            *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
        }
        if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
            *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
