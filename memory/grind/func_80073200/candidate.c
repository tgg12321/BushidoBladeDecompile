/* func_80073200 — session 3 (structural) candidate.
 * Session 2 closed the AddPrim compile-time-fold + color-byte residuals with the S73200
 * address-taken struct + `tbl` named intermediate (floor 115 -> 24, target_insns == build_insns
 * == 203). Its frontier named a genuine register-seat/frame-size gap: target uses 4 callee-saved
 * regs (s0-s3, 96-byte frame) and keeps the repeated `0x12` literal (`s.sp2C = 0x12;`, written once
 * unconditionally after the first if/else block and again inside the `if (D_800A3580 < 2)` block)
 * live in `s3` across BOTH `func_8007352C` call blocks, while our build only spanned one block at a
 * time in a caller-saved reg (`s1`/`v1`).
 *
 * This session's lever (H5): name the repeated `0x12` literal as a single fresh local `v12`,
 * declared+initialized ONCE at the top of the function (`s32 v12 = 0x12;`) and READ at both
 * `s.sp2C = v12;` sites (never reassigned) — the SOTN-sanctioned named-intermediate / `new_var_temp`
 * family (no-new-park-categories.md "Named-intermediate declaration order", relaxed to
 * once-written/many-read by the 2026-08-31 ordinary-c-judge-decidable ruling). This is ordinary C:
 * `v12` holds a real value that is genuinely read at both AddPrim/func_8007352C call blocks.
 *
 * MEASURED EFFECT: score 24 -> 17. Confirmed the frame-size/callee-save hypothesis: with `v12`
 * declared+initialized at the top, global register allocation puts it in `s3` (matching target
 * exactly) and the prologue/epilogue now correctly save/restore `s3` with a 96-byte frame — every
 * hunk about the `sp,-96` vs `sp,-88` frame size and the `sw ra,88(sp); sw s3,84(sp)` callee-save
 * pair CLOSED. The only remaining defect this lever introduces: GCC materializes `li s3, 0x12`
 * at the very TOP of the function (position 30 in the --diff numbering, where target has a bare
 * `nop`) as well as at the correct point later (matching target's actual `li s3,0x12` position),
 * i.e. it emits ONE EXTRA instruction (target_insns 203, build_insns 204) — a rematerialization
 * of the global-allocated constant's value at the earliest legal point in the CFG, not exactly
 * where target's compiler placed it. This is NOT source-missing (target_insns/build_insns delta
 * is exactly +1, isolated to this one hunk) — every other hunk from the s2 floor-24 diff besides
 * the frame-size ones is UNCHANGED (still present at 17): the two `addiu s1,v0,12`/`sw s1,28(sp)`
 * register-seat ties (target keeps `s1`, we still get `v1`) and the D_800A3580<2 test scheduling
 * hunks (target computes the branch condition earlier into `v1`, ours computes it later into `v0`
 * with a load-delay nop) are untouched by this lever — they are a SEPARATE, still-open residual.
 *
 * MEASURED-DEAD variants of this lever (see hypotheses.md for full detail — all instance kills,
 * same chassis):
 *   - v12 declared WITHOUT an initializer (`s32 v12;`) and assigned `v12 = 0x12;` right at the
 *     point of first use (immediately before the first `s.sp2C = v12;`) — identical codegen to
 *     the PRE-v12 baseline (score 24, register choice reverts to v1/s1, no frame change). The
 *     persistent-register effect ONLY appears when the assignment is an INITIALIZER emitted at
 *     the textual top of the function body.
 *   - Re-ordering v12's DECLARATION among the other locals (moved to declare first, right after
 *     `S73200 s;`) while keeping the initializer — no change (score still 17); C declaration
 *     order among sibling locals does not affect where an initializer's assignment statement is
 *     emitted in the function body — the top-of-body initializer is always the first statement
 *     regardless of textual declaration order among co-declared locals.
 *   - Moving the initializing statement partway down (right after `s1 = base2 + 0xC;`, still well
 *     before the first if/else block) — WORSE, not better: score 22, build_insns 207 (three extra
 *     insns). Confirms the hoist is not simply "put it as early as textually possible" — the
 *     absolute-top initializer position is uniquely better than a mid-function-but-still-early one.
 *
 * REMAINING RESIDUAL (17): (1) the phantom early `li s3,0x12` rematerialization (mechanism
 * suspected: global/reload constant rematerialization for a globally-allocated pseudo with a
 * REG_EQUIV constant note — not yet root-caused against the GCC source this session); (2) the
 * pre-existing `s1`/`v1` register-seat tie at the `tbl+0xC` store (unchanged since s2); (3) the
 * D_800A3580<2 test's early-vs-late materialization into v1 vs v0 (unchanged since s2, likely
 * downstream of (1)/(2)'s register pressure). Frontier for next session: read the .greg/.combine
 * dumps for the v12 pseudo specifically (tmp/grind/func_80073200/dumps/text1b.greg) to see which
 * GCC pass performs the early rematerialization and find a C-source lever that keeps the
 * persistent-register allocation while suppressing the redundant early copy.
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
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (D_800A3580 < 2) {
        s.sp2C = v12;
        s.sp28 = 1;
        v1 = *(s32 *)((s32)D_800A35C4 + 8);
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
