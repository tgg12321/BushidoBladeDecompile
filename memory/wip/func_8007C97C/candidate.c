/* func_8007C97C — WIP candidate body: LEGITIMATE BASE (no sp[4] cheat).
 *
 * Status: this is the inverted-null-check + m2c-shape body that was committed
 * as 0e845d2 (2026-06-01) and REVERTED in 45a7bb0 (2026-06-02) because the
 * cheat-reviewer flagged the `s32 sp[4]; sp[0..3] = ...;` write-only frame-
 * coercion as a cheat-by-spelling. THE INVERTED NULL-CHECK ITSELF IS NOT THE
 * CHEAT — it's the SOTN-clean structural lever that the original
 * goto-end-prologue-delay-slot rule was trying to encode. Reviewer's evidence
 * cited Tests 1, 2, and 6 against the sp[4] writes only.
 *
 * THIS FILE is the prior body MINUS the sp[4] writes — i.e. the legitimate
 * base that future sessions should resume from. Apply to src/display.c
 * (replace HEAD's func_8007C97C body), then measure:
 *
 *   & tools/eng.ps1 sandbox func_8007C97C --disable all
 *
 * Expect score lower than HEAD's 24 (HEAD's distance carries 5 cheat-asm pins
 * + 9 regfix substs that are stripped by the sandbox). The exact floor for
 * this legitimate base needs measurement on first apply — record it in
 * meta.json scores.candidate_floor and update notes.md.
 *
 * DO NOT add `s32 sp[4];` (or any other write-only local) back. See
 * rejected/sp4_frame_coercion.c for the FAIL form and why.
 *
 * The structural lever that's worth preserving from session 0e845d2:
 *   1. Inverted null-check: `if (arg0 != NULL) { body; return X; } return 0;`
 *      — replaces the FORBIDDEN goto-end-with-ret_val accumulator from a
 *      prior session. This shape lets GCC emit target's exact entry sequence
 *      (`bnez $a0, .L_body; addiu $sp,-0x10; j .L_end; addu $v0,$zero,$zero`).
 *   2. m2c-shape body: single OR-return expression with explicit casts on
 *      the negated halves. SOTN-allowed.
 *   3. `u32 r_e2 = (temp_a1 << 0xA) | 0xE2000000;` named intermediate —
 *      SOTN-allowed per commit 161c6c3 (named-intermediate declaration order).
 *   4. `temp_v0 = temp_v0 << 0xF;` variable reuse — SOTN-allowed per the
 *      same commit (defeat-licm var-reuse).
 *
 * The next session's job: derive whatever closes the function from THIS base
 * (which is now LEGITIMATE for the first time) without re-introducing
 * forbidden constructs. Likely candidates: directed permuter from the score-N
 * base with cheat-reviewer gating; ALLOCDBG instrumented dump to identify
 * what's blocking the natural frame layout.
 */

s32 func_8007C97C(u8 *arg0) {
    u32 temp_a1;
    s32 temp_a2;
    u32 temp_v0;
    s32 temp_v1;

    if (arg0 != NULL) {
        temp_a1 = (u8) arg0[0] >> 3;
        temp_a2 = (s32) (-*(s16 *)(arg0 + 4) & 0xFF) >> 3;
        temp_v0 = (u8) arg0[2] >> 3;
        temp_v0 = temp_v0 << 0xF;
        temp_v1 = (s32) (-*(s16 *)(arg0 + 6) & 0xFF) >> 3;
        {
            u32 r_e2 = (temp_a1 << 0xA) | 0xE2000000;
            return temp_v0 | r_e2 | (temp_v1 << 5) | temp_a2;
        }
    }
    return 0;
}
