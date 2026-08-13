/* func_80078654 — BEST FORM as of grind session 1 (2026-08-13, modality: recon).
 *
 * Honest pure-C sandbox floor: 19  (baseline at session start was 23).
 * Instruction count now EXACTLY matches target (116 == 116).
 * Zero inline asm: the session-start body carried a forbidden
 * INLINE_MOVE_ALIASING construct
 *     __asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v));
 * which has been REMOVED and replaced by the natural short-typed clamp
 * below. That single change dropped the floor 23 -> 19 AND fixed the
 * instruction count (115 -> 116), so the cheat was not merely inert, it was
 * actively worse than the honest form.
 *
 * RESIDUAL (all 19 points of it): a single 2-way callee-save allocation
 * inversion. Target puts arg0 in $s1 and var_s0 in $s0; our build puts arg0
 * in $s0 and var_s0 in $s1. Nothing else differs — see evidence.md.
 *
 * SESSION 2 (structural) left this form UNCHANGED and still best at 19: every
 * structural partition variant measured worse or equal (the base/walk-pointer
 * merge scores 22 — rejected/base-merge-walk-pointer-pri-2448.c), and the whole
 * global.c decision procedure is now read and closed (evidence.md §SESSION 2).
 * It IS applied to src/text1b_b.c as of the end of session 2.
 *
 * SESSION 3 (structural) ALSO left this form unchanged and still best at 19.
 * It measured the ledger's F4 duplication axis on both shapes this CFG admits
 * and KILLED it: jump2's find_cross_jump re-merges neither (walk-init into both
 * arms = +2 insns / sandbox 25; whole loop into both arms = +39 insns / sandbox
 * 58), and the loop body holds 6 arg0 references against 4 walk references, so
 * duplication raises the PARAMETER's priority 1.5x faster than the walk
 * pointer's (measured 5671 vs 2125 at k=2). See evidence.md SESSION 3.
 *
 * SESSION 4 (permuter) ALSO left this form unchanged and still best at 19. It
 * built a clean single-function permuter rig (tmp/grind/func_80078654/s4/mkws.sh:
 * target.o from asm/funcs/func_80078654.s + the r3000-ified prelude, so the
 * function sits at offset 0 and the score is the real weighted diff; validated
 * base score 108 whose objdump diff prints exactly the twelve known $s0/$s1
 * lines) and ran two campaigns totalling 89,084 iterations with ZERO
 * score-improving finds � chassis A random from this base (best 108 == base,
 * both saved finds semantics-changing) and chassis B seeded from the s2 H4
 * base/walk-pointer merge (base 331, best 191, strictly worse basin). The
 * permuter axis is closed; see evidence.md SESSION 4.
 *
 * NOTE FOR THE NEXT SESSION: HEAD does NOT carry this body — the s2/s3 ledger
 * commits are ledger-only, so src/text1b_b.c at HEAD still has the inherited
 * `s32 v;` + `__asm__ volatile("move %0, %1" ...)` form that scores 23. Apply
 * this file FIRST or every measurement you take will be against the stale base.
 *
 * Apply this body over src/text1b_b.c's func_80078654 (replacing the whole
 * function AND deleting the now-unused `s32 v;` local). NOTE: with these
 * edits the six existing regfix.txt rules for func_80078654 no longer line
 * up (they were written against the inline-asm body and its maspsx indices),
 * so a FULL build will not be SHA1-clean until the function reaches distance
 * 0 and those rules are retired. Grind with the sandbox, not the full build.
 */

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
