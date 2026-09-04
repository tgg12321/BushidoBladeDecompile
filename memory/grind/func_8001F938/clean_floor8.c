/* func_8001F938 -- CLEAN FLOOR-8 CHASSIS (simplest known form), s13c 2026-09-04.
 *
 * This is the zero-FAKE, zero-borderline reference body: the s1 kind-split
 * (cheat-reviewer PASSED in s2, tmp/grind/func_8001F938/s2/cheat_reviewer_verdict.txt:4)
 * + the s1 branch-sense flip, and NOTHING else.  Measured live this session with this
 * exact text installed in src/code6cac.c:
 *     & tools/wteng.ps1 main sandbox func_8001F938 --disable all
 *     => score 8, target_insns 107, build_insns 105, rules_dropped 0, scorable true.
 *
 * WHAT CHANGED IN s13c: the previously-banked clean form carried
 * `idx = ((raw_or_3 << 16) >> 15);` -- an artificial shift pair with no semantic
 * purpose, kept since s1 as a (failed) attempt to defeat combine's fold.  s13c measured
 * that `idx = raw_or_3 * 2;` produces a BYTE-IDENTICAL function body: the extracted
 * func_8001F938 text of tmp/grind/func_8001F938/s13c/base.s and .../P0_mul2.s diff
 * empty (zero lines).  Per owner ruling 2026-08-31 Ruling 1(4) (simplest-known-form,
 * .claude/rules/ordinary-c-judge-decidable.md:61) the shift pair is therefore dropped.
 * Nothing about the residual changes: the floor is 8 either way.
 *
 * RESIDUAL (s13 decomposition, re-confirmed s13c): 6 points in the .L8001FA60 block
 * (target's second, unsigned load of +0x270 and its `sll 16 ; sra 15`) + 2 points for
 * the 8-byte phantom stack frame (asm/funcs/func_8001F938.s:11 and :117).
 *
 * The distance-0 body lives in candidate.c and is mechanically blocked (state.json
 * banned_constructs #3) despite the Judge PASS of 2026-09-04 12:39; see the s13c
 * ruling-request.
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
        idx = raw_or_3 * 2;
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
