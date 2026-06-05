/* func_8007C97C — WIP candidate body: LEGITIMATE BASE (no sp[4] cheat).
 *
 * Round 7 (2026-06-04) UPDATE — floor LOWERED 29 → 27 via narrow integer
 * type (Lever B per .claude/rules/register-alloc-pure-c.md). u8-typed
 * temps for r/g/b1/b2 (their natural width — color channels are 0..255)
 * + explicit u32 casts in the final OR pack. Measured floor:
 *   score=27, build_insns=24, target_insns=33, rules_dropped=9.
 *
 * History: this is the inverted-null-check + m2c-shape body that was
 * committed as 0e845d2 (2026-06-01) and REVERTED in 45a7bb0 (2026-06-02)
 * because the cheat-reviewer flagged a `s32 sp[4]; sp[0..3] = ...;`
 * write-only frame coercion. The inverted-null-check is the SOTN-clean
 * structural lever; rounds 4-5 preserved it at floor 29. Round 7 added
 * the narrow-int-type lever (per [[register-alloc-pure-c]] Lever B),
 * which is documented legitimate and unambiguously NOT a cheat (it's
 * a TYPE change, not a coercion construct — color channels are u8 by
 * domain, and narrowing types is a standard SOTN-allowed technique
 * documented in [[narrow-byte-args-packed-call]] and many others).
 *
 * Apply to src/display.c (replace HEAD's func_8007C97C body), then
 * measure:
 *
 *   & tools/eng.ps1 sandbox func_8007C97C --disable all
 *
 * Documented floor: 27.
 *
 * The remaining 9-insn gap is still the same structural problem:
 * 4 dead `sw $rN, K($sp)` writes at sp[0..3] + frame alloc/dealloc + 2
 * nops + 1 lh ordering, none of which a legitimate pure-C form produces.
 * Round-7 permuter (12 min, ~6749+ iters, 6 workers) found ONLY
 * address-coercion cheats (`u32 *new_var = &temp_v0;`, `s32 *new_var =
 * &temp_v1;` — best legitimate-shape weighted score 1010 and 1095, both
 * dead-stack-write coercions via address-of-local). Both reject per
 * [[no-new-park-categories]] cheats-by-any-spelling. The permuter's
 * RANDOM mutation pass on this candidate base has been exhausted at the
 * same wall sessions 4-5 identified: dead stack writes require a
 * coercion construct, and every coercion construct is forbidden.
 *
 * Round-7 also tested 3 NEW manual structural variants from the round-7
 * base, all measured at floor 29-30:
 *   - V1 halfword-pointer alias (s16 *hp = (s16*)(arg0+4)): 29
 *   - V2 in-place OR accumulator (ret |= ...): 30
 *   - V3 u8-typed temps: 27 (this candidate)
 *   - V4 u8 + simpler casts (no (u8) before arg0[N]): 27 (same)
 *   - V5 u8 + named e2 intermediate: 27 (slightly different build_insns)
 *
 * V3 is the lowest legitimate floor found across all rounds.
 *
 * DO NOT add `s32 sp[4];` (or any other write-only local) back. DO NOT
 * add `T *new_var = &local;` address-coercion patterns — same family
 * caught by `engine/volatile_cheats.find_addr_coerced_locals` and
 * documented in [[dead-vars-local-array]] / [[inline-asm-injection]].
 *
 * The structural lever preserved from session 0e845d2:
 *   1. Inverted null-check: `if (arg0 != NULL) { body; return X; } return 0;`
 *      — replaces the FORBIDDEN goto-end accumulator from a prior session.
 *   2. m2c-shape body: single OR-return expression with explicit casts.
 *      SOTN-allowed.
 *   3. Round-7 addition: u8 temps for r/g/b1/b2 (Lever B narrow integer
 *      type, SOTN-allowed).
 */

s32 func_8007C97C(u8 *arg0) {
    if (arg0 != 0) {
        u8 r  = arg0[0] >> 3;
        u8 g  = (-*(s16*)(arg0 + 4)) >> 3;
        u8 b1 = arg0[2] >> 3;
        u8 b2 = (-*(s16*)(arg0 + 6)) >> 3;
        return ((u32)b1 << 0xF) | ((u32)r << 0xA) | 0xE2000000u | ((u32)b2 << 5) | (u32)g;
    }
    return 0;
}
