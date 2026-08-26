/* func_8001F938 (src/code6cac.c) - CLEAN FLOOR-8 FORM. Honest sandbox distance 8
 * (build_insns 105 vs target_insns 107, rules_dropped 0), RE-MEASURED ON THE LIVE
 * CHASSIS in grind session s11 (escalation/disposition, 2026-08-25). Zero regfix/asmfix
 * rules, zero cheat-asm, zero inline asm, zero volatile, zero FAKE constructs, zero
 * constructs from any sanctioned or unsanctioned coercion family. This is the form the
 * Judge's standing constraint orders src/ to be kept at; src/ itself ships
 * INCLUDE_ASM("asm/funcs", func_8001F938) per the asm-until-matched policy.
 *
 * WHY THIS FILE WAS ROLLED BACK (s11, 2026-08-25). The previous contents of candidate.c
 * were the distance-0 body built on `s16 dmg = *((s16 *)(arg0 + 0x270)); if (dmg >= 4)
 * dmg = 3; idx = dmg * 2;`. That body measures 0 and full-build-verifies, but the
 * layer-1 cheat-reviewer FAILed it TWICE (docs/grind/decisions.md 2026-08-25 23:08 and
 * 23:29) as a fifth/sixth spelling of the pre-banned +0x270 signedness-split /
 * dual-typed-view fold-defeat family, and ruled that the intervening 23:20 "narrowing"
 * ruling is not a legitimate authorization because a frozen family is owner-only to
 * extend (.claude/rules/judge-sole-gate.md). The driver has since made BOTH the
 * construct AND that 23:20 entry mechanically banned constructs for this function: a
 * candidate-ready whose self-vet re-declares either is discarded as an invalid session
 * before the Judge ever runs. The distance-0 body is preserved verbatim in
 * memory/grind/func_8001F938/rejected/layer1-fail-0825-2329.c - do NOT re-install it,
 * in any spelling, without a NEW owner ruling.
 *
 * THE RESIDUAL (unchanged since s3; pass-attributed in s6/s7). Target's .L8001FA60
 * emits TWO same-address loads - asm/funcs/func_8001F938.s:82-83,
 *      lh   $v0, 0x270($a0)     <- sign-extended, feeds `slti $v0,$v0,4`
 *      lhu  $v1, 0x270($a0)     <- raw halfword, feeds `sll 16 ; sra 15`
 * This form emits ONE. The 2-instruction deficit (105 vs 107) IS the whole residual.
 * s6/s7 named the pass: GCC 2.7.2 `combine` / simplify_shift_const, gated on
 * num_sign_bit_copies of the shift OPERAND, and distance 0 additionally requires the
 * 16-sign-bit-copy operand to arrive as a SECOND MEMORY load.
 *
 * OWNER DIRECTIVE (2026-08-24, "solver modality recommended") - EXECUTED IN s11, verdict
 * NOT APPLICABLE. ra_solver answers "which register" (tools/ra_solver/README.md:1-10) and
 * sched_solver answers "which order" (tools/sched_solver/README.md:1-16); both are
 * count-preserving models over a fixed insn set. This residual is an insn-COUNT deficit
 * (a load that our C never causes cc1 to emit), not a register seat and not an emission-
 * order tie, so neither solver can express it. No solver run was spent.
 *
 * ENDGAME-LOCK GATES, both RE-MEASURED in s11 and both FAILING:
 *   (a) canonical-asm: `python3 tools/scan_hand_coded.py --single func_8001F938` =
 *       tier LOW, score 0/8, "no strong hand-coded indicators" (S1..S8 all clear).
 *   (b) SOTN-master precedent for the signedness-split / redundant dual-typed-read
 *       CSE-defeat family: NONE IN HAND. docs/reference/sotn-construct-index.md (1056
 *       lines, commit aa53500226ee84be763f3e8702b27de06456b3a7) has no signedness /
 *       dual-typed / same-address class at all; the F2 census of 2026-07-01 returned
 *       NOT ESTABLISHED and no session since has produced a file:line citation.
 * Disposition filed in docs/grind/decisions.md, 2026-08-25 entry
 * "func_8001F938 - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27)".
 */

void func_8001F938(u8 *arg0)
{
    u32 kind_full;
    u32 kind;
    s32 val;
    s32 a2;
    s32 idx;
    s32 factor;
    kind_full = *((u16 *)(arg0 + 0x6A));
    kind = kind_full & 0xFFFFU;
    a2 = *((s16 *)(arg0 + 0x1C));
    if (kind == 0x11 || kind == 0xF ||
        ((u32)((s32)kind_full - 0x1C)) < 2U ||
        ((u32)((s32)kind_full - 0x1E)) < 2U ||
        ((u32)((s32)kind_full - 0x20)) < 2U ||
        kind == 0xE || kind == 0x2C || kind == 0xD ||
        kind == 0x7 || kind == 0x33 || kind == 0x14)
    {
        goto clamp;
    }
    if (kind == 0x2) { goto rangecheck; }
    if (kind == 0x1B) { goto rangecheck; }
    if (kind == 0x28) { goto rangecheck; }
    if (kind != 0x26) { goto defaultpath; }
rangecheck:
    val = *((s16 *)(arg0 + 0x40));
    if (val < ((s32)(*((u8 *)(arg0 + 0xA1))))) { goto check_outer; }
    if (val > ((s32)(*((u8 *)(arg0 + 0xA3))))) { goto check_outer; }
    goto clamp;
check_outer:
    if (val < ((s32)(*((u8 *)(arg0 + 0xA2))))) { goto multpath_start; }
    if (val > ((s32)(*((u8 *)(arg0 + 0xA4))))) { goto multpath_start; }
clamp:
    *((s16 *)(arg0 + 0x44)) = 0x1000;
    return;
multpath_start:
    if ((*((s16 *)(arg0 + 0x26C))) == 0)
    {
        s32 f = *((s16 *)(arg0 + 0x274));
        a2 = (a2 * f) >> 12;
    }
    {
        s32 probe = *((s16 *)(arg0 + 0x270));
        s32 raw_or_3;
        if (probe >= 4) {
            raw_or_3 = 3;
        } else {
            raw_or_3 = probe;
        }
        idx = ((raw_or_3 << 16) >> 15);
    }
    factor = *((s16 *)((arg0 + 0x276) + idx));
    a2 = (a2 * factor) >> 12;
defaultpath:
    {
        s32 vv0 = *((s16 *)(arg0 + 0x26E));
        s32 vv1 = *((s16 *)(arg0 + 0x272));
        s32 sum = vv0 + vv1;
        s32 sum_or_3 = (sum < 4) ? sum : 3;
        idx = sum_or_3 * 2;
    }
    factor = *((s16 *)((arg0 + 0x27E) + idx));
    a2 = (a2 * factor) >> 12;
    *((s16 *)(arg0 + 0x44)) = (s16)a2;
}
