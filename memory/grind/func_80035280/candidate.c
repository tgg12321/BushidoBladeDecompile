/*
 * CANDIDATE -- func_80035280 (src/code6cac_b.c) -- s4 (2026-09-07, permuter)
 *
 * FLOOR 16 -> 15 THIS SESSION. The only change against the s3 body is loop 1's
 * two byte stores, respelled from
 *     ((u8 *)p)[0x17 + i] = *src;      /  ((u8 *)p)[0x1D + i] = *src;
 * to
 *     ((u8 *)p + i)[0x17] = *src;      /  ((u8 *)p + i)[0x1D] = *src;
 * Measured on the HEAD chassis with `sandbox func_80035280 --disable all`:
 *     s3 body                       score 16, build_insns 109
 *     THIS body                     score 15, build_insns 109
 * The point is the loop-1 address add. The target emits `addu $v1, $t0, $a3`
 * (pointer first, index second); the `0x17 + i` spelling emits
 * `addu $v1, $a3, $t0`. Adding the index to the POINTER in the source, rather
 * than to the constant byte offset, puts the pointer in the first operand slot
 * of the PLUS that combine hands to the addu pattern, and the operand order
 * flips to the target's. Two other spellings of the same idea measured NO
 * change (`((u8 *)p)[i + 0x17]` and `(((u8 *)p) + 0x17)[i]`, both 31 diffs on
 * the s4 mini-TU harness against this body's 29) -- the win is specifically
 * "pointer + index, then a constant subscript".
 * A 30k-iteration decomp-permuter campaign on the s3 chassis
 * (tmp/perm_80035280, label s4-split-accum-chassis) found exactly one
 * improvement in the whole run, and it was this same transform spelled as
 * `((u8 *)p)[(unsigned long long)(0x17 + i)]` -- an independent confirmation
 * that the loop-1 address add was a real point and that nothing else in this
 * basin is reachable by local mutation.
 *
 * EVERYTHING THE s3 HEADER SAID ABOUT THE FLAG BLOCK AND `i = 0;` STILL HOLDS
 * (local-alloc.c:472 reg_n_deaths == 1 eligibility; the counter-zeroing slot).
 * Read the s3 candidate header in git history / evidence.md FACT 12-18 for it.
 *
 * WHAT IS LEFT, EXACTLY: 15 points, ALL of them downstream of ONE decision --
 * loop.c:1631 hoisting the 0x91A2B3C5 (/1800) magic into the loop-2 preheader.
 * The residual diff is: +2 preheader insns (lui/ori into $t2), -2 in-loop insns
 * (the target's lui/ori at the loop top), +1 `nop` in the `lw 0x4($a2)` load
 * delay slot that the target's `ori` fills, and then ten register-name
 * differences because our hoisted constant occupies $t2 and displaces the mfhi
 * temp to $t3 (target: mfhi $t2 five times, plus the five `addu`s that read it).
 * Kill the hoist and all 15 go together.
 *
 * s4 RE-DERIVED THE GATE INDEPENDENTLY FROM loop.c AND SHARPENED IT: the move
 * condition is `threshold * savings * m->lifetime >= insn_count`. All three
 * factors on the left are pinned, not just threshold:
 *   threshold  = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)  (loop.c:532)
 *                = 2 * 61 = 122; loop 2 has no call and a call cannot be added
 *                without a `jal` (s1 kill), and n_non_fixed_regs is compiler
 *                configuration (.claude/rules/no-compiler-divergence.md).
 *   savings    = m->savings = n_times_used[regno] (loop.c:793), and
 *                n_times_used is a bcopy of n_times_set (loop.c:597) -- i.e.
 *                the number of SETS of the constant pseudo inside the loop, not
 *                the number of uses. For a single constant load that is exactly
 *                1, and it can only go UP (more sets = more eager hoisting).
 *   m->lifetime = luid span of the pseudo (loop.c:791); the const load sits
 *                immediately before its single mult, so it is already the
 *                minimum 1, and it too can only go up.
 * So 122 * 1 * 1 = 122 and the ONLY C-movable term is insn_count, which must
 * reach >= 123 (s2 measured the flip between 120 and 123). This body's loop 2
 * is 55 real insns; the largest natural spelling ever measured here is 62.
 */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i;
    s32 flags;
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    i = 0;
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
    for (; i < 3; i++) {
        ((u8 *)p + i)[0x17] = *src;
        ((u8 *)p + i)[0x1D] = *src;
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
