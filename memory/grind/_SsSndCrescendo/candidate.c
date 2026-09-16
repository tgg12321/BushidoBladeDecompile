/* _SsSndCrescendo candidate - s6 (2026-09-16), SYNTHESIS modality.
 * NOT byte-matched. sandbox --disable all score = 10 (build_insns=199,
 * target_insns=200) - down from the s2-s5 plateau of 130 (build_insns 213).
 * To apply: replace the line
 *     INCLUDE_ASM("asm/funcs", _SsSndCrescendo);
 * with the body below, in src/main.c (right before
 * INCLUDE_ASM("asm/funcs", _SsSndDecrescendo);). Requires the existing
 * `extern s32 _ss_score;`, `extern void func_80087770(s32,s32,s32,s32);`,
 * `extern s16 _SsVmGetSeqVol(s32,s16*,s16*);` declarations (main.c:37/40/41).
 * NOTE: this form no longer uses the SS_SCORE_FLAG macro at all - see the
 * s6 findings below; re-introducing the macro at the clear sites measures
 * 89/223 on this chassis (rejected/macro-clear-sites-s6.md).
 *
 * WHAT S6 CHANGED (all five changes measured individually; see evidence.md):
 *  1. NO cached field locals. s2-s5 cached `unk42`, `unk40` and `key` in s16
 *     locals; target re-reads every one of those fields from the struct at
 *     each use (lh 0x4C(s0) / lh 0x4A(s0) after each call, and a fresh
 *     `sll a1,8; or a0; sll 16; sra 16` key build per block). Direct field
 *     reads + an inline key expression at every call site: 130 -> 84.
 *  2. voll/volr are u16, not s16 (target uses `lhu` + `andi ,0xFFFF`), and
 *     the two computed func_80087770 calls pass (u16)-truncated arguments
 *     (target's `andi $a1,$a1,0xFFFF` / `andi $a2,$a2,0xFFFF` before/at the
 *     shared jal). SOTN's cres.c also declares these u16.
 *  3. The channel address is carried as TWO ordinary locals - `bank`
 *     (a pointer to the bank-table slot, = &_ss_score + ((a0<<16)>>14)) and
 *     `a1_off` (= (s16)a1 * 0xB0) - exactly target's $s3 / $s2. The clear
 *     sites reload `*bank` (memory is clobbered by the intervening calls,
 *     so the reload is real and matches H3's fresh-re-derive finding) and
 *     re-add a1_off. 84 -> 69.
 *  4. The tail check's second disjunct is `unk40 <= 0`, not `== 0`
 *     (target `bgtz $v0, .L800844C0` at 0x80084470; cres.c has `==0`).
 *     A 4.1-build divergence, same class as H1/H2. 69 -> 68.
 *  5. CONTROL FLOW: the two arms share their `unk40 < 0` handler and their
 *     `(unk98==0 || unk40<=0)` tail check via `goto neg40;` / `goto tail;`
 *     into single blocks placed before the common final call, and the outer
 *     guard is written in its ASM-ORDER polarity (`if (--unk98 < 0) clear;
 *     else { ... }`). Target emits exactly one copy of each of those blocks
 *     (.L8008441C and .L80084458/.L8008447C) reached by branches from both
 *     arms; GCC 2.7.2 would NOT cross-jump our duplicated copies (measured:
 *     duplicated tail = 233 insns, duplicated neg40 handler = 212 insns).
 *     68 -> 50 (shared tail) -> 20 (polarity + operand order + u16 casts)
 *     -> 10 (shared neg40 handler).
 *     This is the `cross-jump-store-tail-merge` shape (ordinary C - goto to
 *     a shared tail label, no FAKE annotation, nothing dead).
 *  6. One deliberate spelling asymmetry, forced by the bytes: the five
 *     clear sites inside the arms use the cached bank/a1_off pair, while
 *     the SHARED TAIL clear re-derives the whole address inline from
 *     _ss_score. Target does exactly this (`.L8008447C` rebuilds
 *     `sll a0,s5,16; sra a0,14; ...; lui at,%hi(_ss_score)` while
 *     .L8008441C/.L800844AC use `lw 0x0($s3)` + `addu $s2`). Using the
 *     cached pair at the tail too under-counts by 14 insns (186 vs 200).
 *
 * RESIDUAL at score 10 / 199 insns (the whole remaining diff, from
 * tmp/grind/_SsSndCrescendo/s6/diff.py output banked this session):
 *   - target opens with `addu $a3, $a0, $zero` - a SECOND live copy of
 *     param a0 (a third, $s5, is made later). Every pre-call key build in
 *     target reads $a3/$a1 (the incoming param regs); every post-call one
 *     reads $s5/$s4. Our build has only the $s5 copy and reads $a0
 *     directly, so it is exactly ONE instruction short (199 vs 200) and
 *     the register names at 6 sites differ. What C shape gives GCC 2.7.2
 *     two distinct pseudos for the same parameter here is the open
 *     question - see hypotheses.md H5.
 *   - two delay slots target fills with `sll $a0,$s4,8` (a reorg.c peel of
 *     the final call's key build, see [[reorg-peel-is-not-a-source-statement]])
 *     are `nop` in ours; a consequence of the same register split, not an
 *     independent problem.
 *   - `sra $v0,$v0,14` sits one slot later in target's preamble (after the
 *     lui/addiu of %hi/%lo(_ss_score)) - pure scheduling.
 *
 * Everything from s1-s5 that is still load-bearing (field offset table,
 * H1 outer guard, H2 literal-1 4th arg, H3 fresh re-derive at the clear
 * sites, the extra clear in the unk42<0 arm) is unchanged and still
 * correct; see evidence.md.
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
        *(s32 *)(a1_off + *bank + 0x98) &= ~0x10;
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
                *(s32 *)(a1_off + *bank + 0x98) &= ~0x10;
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
    *(s32 *)(a1_off + *bank + 0x98) &= ~0x10;
tail:
    if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
        *(s32 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
done:
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
