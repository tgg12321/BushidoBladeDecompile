/* func_80073200 — session 2 (structural) candidate.
 * Session 1 wrote the first C body (score 115, target_insns 203 / build_insns 131) with
 * bare stack scalars sp18/1C/20/24/28/2C/30/34/38/3C/40/41/42/43. This session's H3 lever
 * (frontier item #2 from s1) replaced those bare scalars with an explicit local struct
 * (S73200, field-for-field identical layout to the already-established S_69AE4 /
 * S_69F80 structs used elsewhere in this same TU with func_80073728/func_8007352C) whose
 * address is taken and passed to func_80073728/func_8007352C. This is the SAME construct
 * already on main for func_80069AE4/func_80069F80/func_8005D46C/func_8005FA98 in this file
 * (ordinary C — ADDRESS-TAKEN AGGREGATE, ordinary struct declaration, no cheat).
 * Effect measured this session: score 115 -> 26 (build_insns 131 -> 203, now EQUAL to
 * target_insns 203) — the prior build was constant-folding struct-field values across the
 * four func_80073728(&s, N) calls because each field was a SEPARATE bare-scalar C variable
 * unrelated (in GCC's alias analysis) to the one whose address escaped (sp18); as ONE
 * struct, taking &s makes the WHOLE aggregate address-escape, so GCC can no longer treat
 * s.sp2C's value as unaffected by the intervening calls and reloads it from memory exactly
 * like target — closing hunks 15-18/21-23/31-33 (the AddPrim compile-time-fold hunks named
 * in s1 hypothesis H3) essentially for free, matching the s1 ledger's cascade prediction.
 * A second small lever (naming `tmp + 0xC` / `idx + 0xC` as a local `tbl` before storing to
 * s.sp1C, matching the same `tbl = p1 + 0xC; s.sp1C = tbl;` shape already used at
 * src/text1b.c:6249-6250/6257-6258 and 6352-6353 for the SAME S_69F80/S_69AE4-family
 * structs) dropped the remaining two source-level hunks (target's `addiu s1,v0,12; sw
 * s1,28(sp)` vs our inlined `addiu v0,v0,12; sw v0,28(sp)`) to operand-only (register-seat)
 * ties: score 26 -> 24.
 *
 * REMAINING RESIDUAL (24, all register-seat/scheduling ties + one still-open source hunk):
 * target uses 4 callee-saved regs (s0-s3, frame 96) where ours uses 2 (s0,s1 not s0-s3,
 * frame 88) — an 8-byte/one-extra-callee-save gap (hunks 1/2/10/11/21/22). Also one
 * apparent duplicate D_800A3580<2 re-test (hunk 16, 5 inserted insns) whose net effect is
 * likely a scheduling/alignment artifact of the register-seat difference above, not an
 * independent missing statement (target_insns == build_insns == 203, so nothing is
 * structurally missing/extra — every remaining hunk is a REPLACE/reorder, not an
 * insert/delete pair of unequal total length). NOT yet root-caused this session — next
 * session's frontier.
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
    s.sp2C = 0x12;
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
        s.sp2C = 0x12;
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
