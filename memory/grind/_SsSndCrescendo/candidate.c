/* _SsSndCrescendo candidate — s3 (2026-09-16), structural modality.
 * NOT byte-matched. sandbox --disable all score = 130 (build_insns=213,
 * target_insns=200) — down from s2's 136 (inlined address expr, see the
 * function-body comment below) and the s1 recon floor of 200 (no C written).
 * To apply: replace the two lines
 *     INCLUDE_ASM("asm/funcs", _SsSndCrescendo);
 * with the body below, in src/main.c (right before
 * INCLUDE_ASM("asm/funcs", _SsSndDecrescendo);). Requires the existing
 * `extern s32 _ss_score;`, `extern void func_80087770(s32,s32,s32,s32);`,
 * `extern s16 _SsVmGetSeqVol(s32,s16*,s16*);` declarations already in the
 * TU (main.c:37/40/41) and the `SS_SCORE_FLAG(i,j)` macro (main.c:267-268).
 *
 * Structure is the SOTN reference `tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c`
 * (evidence.md s1) plus three confirmed BB2/4.1 divergences from it:
 *   H1 (CONFIRMED s2): an outer guard `if (--score->unk98 >= 0) { <all of
 *       cres.c's body> } else { score->unk90 &= ~0x10; }` wraps the ENTIRE
 *       function — SOTN's cres.c runs the unk42-sign logic unconditionally.
 *   H2 (CONFIRMED s2, asm read): every func_80087770 (SpuVmSetSeqVol analog)
 *       call's 4th argument is literal 1 in BB2's asm; cres.c's transcribed
 *       text uses literal 0 throughout. Systematic — spelled as `1` at every
 *       call site here.
 *   NEW (found s2, asm read, not in evidence.md yet): in the unk42<0 arm,
 *       the FIRST inner call (`func_80087770(key,0x7F,0x7F,1)`, guarded by
 *       `(voll-unk42>=0x7F) && (volr-unk42>=0x7F)`) is immediately followed
 *       by an extra `SS_SCORE_FLAG(a0,a1) &= ~0x10;` in BB2's asm
 *       (.L800843B8's lead-in) that does NOT appear in the transcribed
 *       cres.c text for that specific call. Spelled below; every OTHER
 *       call site's absence/presence of a trailing clear matches cres.c
 *       exactly (verified instruction-by-instruction against
 *       asm/funcs/_SsSndCrescendo.s).
 * H3 (CONFIRMED s2, matches cres.c as originally read — no divergence):
 *       the three `_ss_score[arg0][arg1].unk90 &= ~0x10;` sites re-index
 *       fresh from the global rather than reusing the `base` pointer — this
 *       is exactly what cres.c's own source does (score-> is used for
 *       READS, the fresh _ss_score[][] form only for these three specific
 *       WRITES). Spelled via the existing SS_SCORE_FLAG(i,j) macro, which
 *       is the proven idiom for this exact recompute-from-scratch pattern
 *       elsewhere in this TU (SsSeqCalledTbyT, main.c:281 etc).
 *
 * Field offset table (established this session by reading
 * asm/funcs/_SsSndCrescendo.s side-by-side against cres.c's control flow;
 * SOTN field NAMES below are cres.c's own arbitrary labels, not BB2-proven
 * numbers — only the OFFSETS and WIDTHS are asm-confirmed):
 *   +0xA0  s32  cres "unk98" — outer decrement counter (lw/sw, full word)
 *   +0x4C  s16  cres "unk42" — sign-branch field (lh)
 *   +0x4A  s16  cres "unk40" — inner decremented/added field (lhu load,
 *          sh store; sign tested via sll16;bltz trick, not a full sra)
 *   +0x98  u32  cres "unk90" — bit-flags word; bit 0x10 cleared (SS_SCORE_FLAG)
 *   +0x9C  s32  cres "unk94" — read via lw (full word) in the
 *          (unk94-unk98)*-unk42 < field48 comparison
 *   +0x48  s16  cres "unk3E" — read via lh; RHS of the same comparison
 *   +0x5C  s16  cres "unk78" — final _SsVmGetSeqVol output slot 1
 *   +0x5E  s16  cres "unk7A" — final _SsVmGetSeqVol output slot 2
 *
 * RESIDUAL (register-allocation class, NOT yet closed — see hypotheses.md
 * H4): the outer bank-index shift `(a0<<16)>>14` compiles here as THREE
 * insns (sll 16; sra 16 [full]; sll 2) instead of the target's/sibling
 * _SsSndPause's TWO (sll 16; sra 14, fused). Confirmed the identical
 * `s32 shifted=a0<<16; ...; shifted>>14` source shape DOES fuse in the
 * already-matched _SsSndPause (sandbox score 0) when key/base are used
 * only once each near the top; in this function `key` stays live across
 * ~6 call sites spanning the whole function body, and that extra liveness
 * appears to be what changes the RA/combine decision — declaring key as
 * s32 (worse, 141) vs s16 (this file, 136) vs inlining `(s16)(a0|(a1<<8))`
 * at every call site instead of a variable (worse, 143/220 insns) were all
 * measured this session; s16-cached-key is the best of the three.
 
 *
 * s4 (permuter modality, 2026-09-16): objdump-diffed a clean standalone
 * permuter workspace's target.o vs a bank_off-sharing chassis's base.o
 * (tmp/grind/_SsSndCrescendo/s4/{target,base_s4_bankoff}.dis) and confirmed
 * target caches BOTH the bank pointer (&_ss_score+bank_off) AND the a1*0xB0
 * product as separate hard registers, recombined once via addu to build
 * `base`. A directed permuter campaign on that insn-count-exact (200/200)
 * chassis plateaued 8332-9589 over 4687 iterations (see hypotheses.md) -
 * KILLED as a search route for this chassis. s5 measured caching BOTH
 * bank_off AND a1_off as named C locals (mirroring target's register
 * pattern literally) - KILLED, UNDER-counted insns (184 vs 200), proving
 * GCC's CSE over-applies the named a1_off local relative to what target's
 * asm actually does at the clear sites (rejected/shared-bank-off-and-a1off-s5.md).
 * The gap between "target visibly caches 2 components at the top" and "our
 * C can't reproduce that without over-caching downstream" remains OPEN -
 * next register-alloc session should dump-read (.greg/.lreg) the 139-score
 * bank_off-only chassis specifically at the clear sites to see whether GCC
 * is choosing to re-derive a1's x0xB0 fresh there (matching target) or not,
 * rather than guessing from the C-level symptom alone.
 *
 * s5 (enumerate modality, 2026-09-16): ran tools/spelling_enum.py over the
 * base/key preamble (bank_off/a1_off named locals, decl order, inline-or-
 * keep, commutative swaps) — 16/16 exhaustively-enumerated spellings
 * measured, holding the SS_SCORE_FLAG clear sites fixed at their existing
 * fresh-recompute form. Best = 130/213 (six-way tie, all equivalent to
 * THIS banked form); nothing beats it. CLASS KILL for the preamble's own
 * spelling space (see hypotheses.md s5) — the residual is NOT reachable by
 * respelling this block alone. New finding: naming BOTH bank_off and
 * a1_off in the preamble ties 130 as long as the clear sites are left
 * untouched — s4's worse 143/184 result came specifically from ALSO
 * rewriting the 6 clear sites to reuse those locals, not from naming them
 * in the preamble. This sharpens the frontier: the residual lives at the
 * clear sites (register-allocation modality), not in the preamble.
 */
void _SsSndCrescendo(s16 a0, s16 a1) {
    u8 *base;
    s16 key;
    s16 voll, volr;
    s16 unk42;
    s16 unk40;

    /* s3 (2026-09-16, structural): inlining the address computation into a
     * single expression (matching the already-matched _SsSeqPlay's style,
     * main.c:399) instead of the separate shifted/addr/base_ptr/offset
     * locals dropped score 136 -> 130 (build_insns unchanged at 213 — the
     * improvement is fewer register-diff penalties, not fewer instructions;
     * see hypotheses.md H4 for why build_insns itself doesn't move). */
    base = (u8 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0);
    key = a0 | (a1 << 8);

    if (--(*(s32 *)(base + 0xA0)) >= 0) {
        unk42 = *(s16 *)(base + 0x4C);
        if (unk42 > 0) {
            if ((*(s32 *)(base + 0xA0) % unk42) == 0) {
                unk40 = *(s16 *)(base + 0x4A) - 1;
                *(s16 *)(base + 0x4A) = unk40;
                if (unk40 >= 0) {
                    _SsVmGetSeqVol(key, &voll, &volr);
                    if ((voll + 1) <= (voll + unk40))
                        func_80087770(key, voll + 1, volr + 1, 1);
                } else {
                    func_80087770(key, 0x7F, 0x7F, 1);
                    SS_SCORE_FLAG(a0, a1) &= ~0x10;
                }
                if ((*(s32 *)(base + 0xA0) == 0) || (unk40 == 0))
                    SS_SCORE_FLAG(a0, a1) &= ~0x10;
            }
        } else if (unk42 < 0) {
            unk40 = *(s16 *)(base + 0x4A) + unk42;
            *(s16 *)(base + 0x4A) = unk40;
            if (unk40 >= 0) {
                _SsVmGetSeqVol(key, &voll, &volr);
                if (((voll - unk42) >= 0x7F) && ((volr - unk42) >= 0x7F)) {
                    func_80087770(key, 0x7F, 0x7F, 1);
                    SS_SCORE_FLAG(a0, a1) &= ~0x10;
                }
                if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -unk42) <
                    *(s16 *)(base + 0x48))
                    func_80087770(key, voll - unk42, volr - unk42, 1);
            } else {
                func_80087770(key, 0x7F, 0x7F, 1);
                SS_SCORE_FLAG(a0, a1) &= ~0x10;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (unk40 == 0))
                SS_SCORE_FLAG(a0, a1) &= ~0x10;
        }
    } else {
        SS_SCORE_FLAG(a0, a1) &= ~0x10;
    }
    _SsVmGetSeqVol(key, (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
