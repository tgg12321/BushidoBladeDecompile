/* _SsSndCrescendo candidate - s7 (2026-09-16), SYNTHESIS modality.
 * sandbox --disable all score = 0 (build_insns 200 == target_insns 200),
 * measured THIS session with this exact body in src/main.c.
 * Previous floor: 10 (s6). To apply: replace
 *   INCLUDE_ASM("asm/funcs", _SsSndCrescendo);
 * in src/main.c with the body below.
 *
 * STRUCTURE: this is the SOTN reference body verbatim in shape
 * (tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c, the matched PSX
 * _SsSndCrescendo of the same Sony LIBSND CRES module BB2 links) - the same
 * nested if / else-if chain, the same `if (unk40 >= 0) {...} else {...}`
 * handler written out in BOTH arms, the same tail check duplicated in both
 * arms, and no gotos at all. s6's goto-shared blocks turn out to have been
 * an artifact of the then-unfixed preamble: with the preamble spelled as
 * below, GCC 2.7.2's post-reload cross_jump merges the duplicated handlers
 * and tails by itself, exactly as it did for the original.
 *
 * The four documented BB2-4.1-build divergences from cres.c are unchanged
 * from s1-s6: (H1) the outer `if (--unk98 < 0) clear; else {...}` guard,
 * (H2) the literal 1 as func_80087770's 4th argument, (s6) the tail
 * disjunct `unk40 <= 0` rather than `== 0`, and (s6) the (u16) truncation
 * of the two computed volume argument pairs.
 *
 * WHAT S7 CHANGED (10 -> 0), two independent findings:
 *
 * 1. The handler and the tail check must be DUPLICATED into both arms
 *    (cres.c's own shape), not shared via goto. s6 measured the duplicated
 *    form at 212/233 insns and concluded GCC would not cross-jump it; that
 *    measurement was taken while the arms still carried an `a1_off` local,
 *    and CSE reached one copy's clear site with the temp pseudo and the
 *    other with the variable pseudo, so the two copies were NOT
 *    register-identical and jump.c's cross_jump correctly refused them.
 *    Dropping that local (writing `(s16)a1 * 0xB0` at the clear sites)
 *    makes the copies identical, and they merge: 212 -> 200 insns.
 *
 * 2. The bank-table address preamble must be THREE statements, in the
 *    order index-conversion, table-address, pointer-add (see the body).
 *    Target's first insns after the frame setup are
 *      sll $v0,$a3,16  /  la $v1,_ss_score  /  sra $v0,$v0,14
 *    i.e. the `_ss_score` address insn sits BETWEEN the two halves of the
 *    index shift. 13 single-expression spellings were measured this
 *    session and every one of them puts it after both halves (score 2).
 *    Mechanism, read from the cc1 -da dumps, not guessed: expand emits the
 *    s16->int conversion as `ashift:16` + `ashiftrt:16`; combine.c's
 *    try_combine folds that `ashiftrt:16` with the scale-by-4 `ashift:2`
 *    into one `ashiftrt:14` placed at the LATER insn's slot; sched.c's
 *    rank_for_schedule leaves the two independent insns in RTL/LUID order.
 *    So whatever is emitted between the conversion and the scale ends up
 *    between the surviving `ashift:16` and the folded `ashiftrt:14`, and
 *    only a separate statement for the table address, ordered after the
 *    index conversion, can land there: fold() moves the constant ADDR_EXPR
 *    to operand 1 of a pointer sum, so a single expression always
 *    evaluates the entire index (both shifts plus the scale) first.
 *
 * The two preamble locals carry their FAKE annotations in the body.
 * Nothing here is stripped by the sandbox (no asm, no volatile, no pin):
 * the 0 is the honest cheat-free distance.
 */
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
