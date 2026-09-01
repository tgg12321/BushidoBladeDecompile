/*
 * CANDIDATE — func_8003C714 (src/code6cac_c2.c) — s1 (2026-09-01), honest floor 15
 * (sandbox func_8003C714 --disable all == 15 with this body in place of the
 * INCLUDE_ASM line; chassis = HEAD at session s1, target 104 insns, build 105).
 *
 * 100% ordinary C, zero cheats, zero FAKE constructs. The pre-migration d33 body
 * used register-asm pins (t0/a2/a1) — this supersedes it entirely.
 *
 * Structure discovered this session (see evidence.md):
 *  - dst/src MUST be recomputed from i each iteration ("index-derived pointers").
 *    loop.c then recognizes them as general givs of biv i and strength-reduces
 *    them to reduced-giv registers (init `addu a1,s0,zero` / `lui+addiu a2,%D`,
 *    step 4/8) with the address offsets 0x21..0x24 / 4,0 preserved in-place.
 *    Plain pointer bivs (src+=8/dst+=4 spelling) instead get biv-75 ELIMINATED
 *    into a biased combined giv (a1=s0+36, offsets -3..0) and an extra IV for
 *    src+4 — that spelling is dead (rejected/plain-pointer-bivs.c).
 *  - i += 1 MUST be the last statement of the body. Mid-body increment splits
 *    the giv value and forces a `move a0,a3` copy. sched1 hoists the addiu into
 *    the 4th mult's mfhi shadow exactly as target.
 *
 * [s2 2026-09-01 UPDATE] This body is BYTE-EXACT (distance 0, 104==104) � but
 * only when cc1 is told the PS1 has no FPU. Add `-msoft-float` (or the 32
 * `-ffixed-$f0..$f31`) to CC_FLAGS and this exact C scores 0; with the shipped
 * CC_FLAGS it scores 15. Mechanism: loop.c:532
 * `threshold = 2*(1+n_non_fixed_regs)` is 122 with the FP regs free and 58 with
 * them fixed; after the `threshold -= 3` that follows the first hoist
 * (loop.c:1719/1904) the /1800 magic faces 55 against the loop's insn_count 56
 * and stays in-loop in the split lui/ori form the target has. NO C SPELLING can
 * reach that on the shipped chassis (it would need a >119-insn loop inside a
 * 104-instruction function). Do not grind this body further; see
 * hypotheses.md H4/K5 and the 2026-09-01 docs/grind/decisions.md entry.
 *
 * Remaining d15 residual = ONE loop.c decision + its seat fallout:
 *  build hoists the 0x91A2B3C5 (/1800 magic) const load to the preheader
 *  (movable, savings 1, life 1, threshold 122 vs insn_count ~56); target has it
 *  IN-LOOP as a TWO-insn lui/ori pair interleaved by sched1 with the first lw
 *  (lui v0 / lw v1 / ori v0 / mult). With the hoist gone, t1 frees and the mfhi
 *  temp (our t2, target t1) is expected to fall into place -> plausibly d0.
 *  See hypotheses.md for the attack frontier on that single decision.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
