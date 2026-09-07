/*
 * CANDIDATE -- func_80035280 (src/code6cac_b.c) -- s2 (2026-09-07, structural)
 *
 * MEASURED THIS SESSION on the HEAD chassis (INCLUDE_ASM baseline, target 108):
 *     THIS body                                sandbox = 39   (build 109)
 *     s1 candidate (s/dst locals, `vC`)        sandbox = 63   (build 109)
 *     byte-field-first reorder                 sandbox = 67   (build 110)
 *     three values into locals, store at end   sandbox = 93   (build  92)
 * 39 is the session floor and supersedes the s1 floor of 56.
 *
 * WHY THIS SPELLING WINS.  Writing every loop-2 memory reference with its
 * index inline -- `((u8 *)p)[i * 4 + 0x2X]` and `*(s32 *)(base + i * 8 + 4)`
 * -- instead of hoisting `u8 *s` / `u8 *dst` walker locals lets cse1 share the
 * two base computations itself, and strength reduction then produces exactly
 * the target's two givs.  The whole of loop 2 becomes register-identical to
 * asm/funcs/func_80035280.s:25B18-25B54 (dst = $a1, record = $a2, counter =
 * $a3, 0x88888889 = $t1, and the same $v0/$v1/$a0 temp rotation), and `p` lands
 * in $t0 and the flag-byte walker in $a2 exactly as the target has them.  The
 * s1 spelling put p in $t1 and the counter in $t0 and permuted the whole
 * loop-2 temp set; that was 24 points of pure naming distance.
 *
 * WHAT IS LEFT, EXACTLY -- and it is NOT a C-spelling problem.
 * The residual is the single loop.c:1631 LICM decision s1 identified: the build
 * hoists the 0x91A2B3C5 (/1800) magic into the loop-2 preheader (2 insns) and
 * then needs a `nop` in the `lw 0x4($a2)` load-delay slot, where the target
 * materialises the constant in the loop and fills that slot with its `ori`.
 * Net +1 instruction (109 vs 108) plus the $t2/$t3 shift it forces on the mfhi
 * temp.  s2 measured the gate exactly instead of deriving it (see below): the
 * constant stays in the loop only when the loop's RTL insn_count exceeds 122,
 * and the three natural spellings above measure insn_count 44, 55 and 62.
 *
 * The measured cause is a COMPILER CONFIGURATION difference, not a source
 * difference: `-msoft-float` (the PS1 has no FPU; PsyQ's cc1psx fixed the FP
 * hard registers) halves n_non_fixed_regs, halves the loop.c:532 threshold, and
 * makes THIS function compile to 108 == 108 with the target's exact loop-2
 * shape.  It is codegen-neutral for 31 of the project's 32 C files.  The one
 * blocker is func_800324D0 (same TU), which regresses 0 -> 3 under the flag.
 * Full evidence + the operator steps are in evidence.md [s2] and the frontier.
 * Do NOT spend another session searching for a loop-2 spelling with
 * insn_count >= 123; that number is now measured, not assumed.
 */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i;
    s32 v;

    p = func_80077D00();
    f = &D_80106A73;
    src = f - 3;
    v = p[8];
    v = (v & ~1) | (src[3] & 1);
    p[8] = v;
    v = (v & ~2) | (src[3] & 2);
    p[8] = v;
    v = (v & ~4) | (src[3] & 4);
    p[8] = v;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = *src;
        ((u8 *)p)[0x1D + i] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[i * 4 + 0x21] = *(s32 *)(base + i * 8 + 4) / 1800;
        ((u8 *)p)[i * 4 + 0x22] = (*(s32 *)(base + i * 8 + 4) / 30) % 60;
        ((u8 *)p)[i * 4 + 0x23] = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;
        ((u8 *)p)[i * 4 + 0x24] = base[i * 8];
    }
}
