/* func_80073200 — session 4 (permuter) candidate, UNCHANGED by session 5.
 * Session 5 (permuter, modality) tried two hypotheses against this exact
 * chassis and killed both (see hypotheses.md [s5]): (1) restructuring the
 * D_800A35C4 pointer read to match target's load-then-offset-load split
 * INSIDE the if-block (worse: 16 -> 17); (2) the s5 permuter campaign's
 * best find, staging the `D_800A3580 < 2` test through the reused `idx`
 * local before its real job (worse on the REAL sandbox: 16 -> 19, despite
 * a better permuter-metric score of 425 vs 545 — a clean instance of the
 * permuter-score-vs-honest-sandbox mismatch). This file is still the
 * correct floor-16 body to resume from; see hypotheses.md for the
 * untried frontier note on the D_800A35C4 address-split question.
 */
/* func_80073200 — session 4 (permuter) candidate.
 * Session 3 closed the frame-size/callee-save gap with `v12` (named-intermediate
 * for the repeated 0x12 literal, floor 24 -> 17) but left three residuals open:
 * (1) a phantom early `li s3,0x12` rematerialization at diff position 30 (target
 * has a bare nop there), (2) the pre-existing `s1`/`v1` register-seat tie at the
 * two `tbl+0xC` stores, (3) the `D_800A3580 < 2` test's early-vs-late
 * materialization (target computes it earlier into v1, ours later into v0 with an
 * extra load-delay nop).
 *
 * This session ran a directed permuter campaign (tools/perm_80073200, workspace
 * built fresh this session since none existed — see setup notes in evidence.md)
 * seeded on the session-3 chassis. It found (output-545-1, permuter score
 * 645 -> 545) that hoisting the `D_800A35C4 + 8` address computation used inside
 * the `if (D_800A3580 < 2)` block out to a fresh local declared once, assigned
 * unconditionally right before the if, and read once inside it:
 *
 *     new_var = (s32)D_800A35C4 + 8;
 *     ...
 *     if (D_800A3580 < 2) {
 *         ...
 *         v1 = *(s32 *)new_var;   // was: v1 = *(s32 *)((s32)D_800A35C4 + 8);
 *
 * measurably closes residual (3): re-measured against the real engine sandbox
 * (not just the permuter's own weighted metric), this drops the honest floor
 * 17 -> 16 with build_insns == target_insns == 203 (was 204). This is the
 * SOTN-sanctioned named-intermediate / `new_var_temp` family
 * (no-new-park-categories.md "Named-intermediate declaration order", relaxed to
 * once-written/many-read by the 2026-08-31 ordinary-c-judge-decidable ruling;
 * `new_var` is literally the SOTN precedent's own shipped identifier for this
 * class, docs/reference/sotn-construct-index.md:649). Ordinary C: the local
 * holds a real, genuinely-consumed pointer value.
 *
 * Full diff after this lever (sandbox --disable all --diff, 17 hunks, 7
 * source-level / 6 operand-only / 4 not-scored):
 *   - Residual (1) UNCHANGED: hunk1 (`target nop` vs `ours li s3,0x12` @ pos 30)
 *     and hunk9 (target's real `li s3,0x12` @ pos 114 that we don't re-emit) are
 *     byte-identical to the s3 floor-17 diff. The v12 rematerialization-placement
 *     mechanism is NOT touched by this lever — still the #1 frontier item.
 *   - Residual (2) UNCHANGED: hunks 10/11/16/17 (`addiu s1,v0,12`/`sw s1,28(sp)`
 *     vs `v1`) are byte-identical to before.
 *   - Residual (3) PARTIALLY CLOSED: hunk12 changed from `target lh v1,0(gp) /
 *     ours nop` to `target lh v1,0(gp) / ours lw v1,0(gp)` (still source-level,
 *     narrower gap), and the downstream insert-group (hunk14) shrank from 5
 *     inserted instructions to 4 (the trailing `li v0,1` duplicate is gone) —
 *     net -1 instruction. The early/late materialization split between v1/v0
 *     is NOT fully closed, just narrowed.
 *
 * A second permuter finding (output-560-1, worse: permuter score 560) respelled
 * the same `if` condition as `if ((D_800A3580 + 1) <= 2)` — REJECTED: no
 * semantic purpose (opaque arithmetic on a compare, pure fold-defeat), worse
 * score than the real fix, cheat-smell per the T1/T2/T3 checklist. Banked at
 * rejected/s4-opaque-arith-branch-cond.c.
 *
 * REMAINING RESIDUAL (16): (1) the phantom early `li s3,0x12` (unchanged,
 * highest-value frontier item — needs .greg/.combine dump analysis of the v12
 * pseudo's REG_EQUIV rematerialization, per s3's frontier note); (2) the
 * `s1`/`v1` register-seat tie (unchanged); (3) the narrowed but not-yet-closed
 * `D_800A3580 < 2` test materialization (`lw v1,0(gp)` vs target `lh v1,0(gp)` —
 * width mismatch, likely another sub-word-read lever candidate; separately the
 * `lh v0,0(gp)` duplicate-read block at hunk14 is the SAME test computed a
 * second time later for the branch itself, still not merged with the first).
 */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    s8 sp40, sp41, sp42, sp43;
} S73200;
void func_80073200(s32 arg0) {
    S73200 s;
    s32 *ctx;
    s32 base1;
    s32 base2;
    s32 s1;
    s32 tmp;
    s32 v1;
    s32 idx;
    s8 var_v0;
    s32 tbl;
    s32 v12 = 0x12;
    s32 new_var;

    s.sp30 = 0;
    s.sp34 = 0;
    s.sp38 = 0x100;
    s.sp3C = 0x100;
    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    base1 = *(s32 *)((s32)ctx + 0xC);
    s.sp18 = base1;
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(base1, 0), 0);
    AddPrim(D_800A374C + 0x70, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    s.sp40 = 1;
    base2 = *(s32 *)((s32)ctx + 0x10);
    s.sp18 = base2;
    s1 = base2 + 0xC;
    if (D_800A3580 < 4) {
        s.sp28 = 1;
        if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
            s.sp41 = 0xBC;
            var_v0 = 0x78;
        } else {
            s.sp41 = 0xA8;
            var_v0 = 0x6E;
        }
        s.sp42 = var_v0;
        s.sp43 = 0x14;
        s.sp2C = 0x14;
        s.sp1C = s1;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        s.sp1C = s1 + 8;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        s.sp1C = s1 + 0x10;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 2);
        s.sp1C = s1 + 0x18;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 3);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, 0x60, 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    } else {
        s.sp41 = 0x32;
        s.sp42 = 0x32;
        s.sp43 = 0x5A;
    }
    s.sp34 = 0;
    s.sp30 = 0;
    s.sp2C = v12;
    s.sp28 = 0;
    tmp = *(s32 *)((s32)ctx + 0x14);
    s.sp18 = tmp;
    tbl = tmp + 0xC;
    s.sp1C = tbl;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x60), 0);
    AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
    new_var = (s32)D_800A35C4 + 8;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (D_800A3580 < 2) {
        s.sp2C = v12;
        s.sp28 = 1;
        v1 = *(s32 *)new_var;
        idx = *(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4);
        s.sp18 = idx;
        tbl = idx + 0xC;
        s.sp1C = tbl;
        s.sp20 = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x20), 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    }
}
