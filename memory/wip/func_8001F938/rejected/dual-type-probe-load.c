/* REJECTED — cheat-reviewer FAIL 2026-06-15
 *
 * Verdict: dual-typed unconditional read of same address at +0x270 (s16 `probe`
 * + u16 `v`) is codegen-steering with no semantic purpose — produces distinct
 * RTL nodes solely to influence GCC's instruction selection for the `<<16`
 * shift path. Fails Tests 1 (no semantic purpose) and 2 (human-programmer).
 * Not covered by SOTN-accepted list nor by split-init-accumulation sanctioning.
 *
 * Caveat for the next session: HEAD's pure-C body ALSO contains a dual-typed
 * read at +0x270 (s16 `probe` for the `<4` guard + u16 read inside the ternary
 * for the saturated value). The reviewer flagged the *unconditional split-into-
 * separate-variables* form here; whether the same dual-typed read in HEAD's
 * guarded-ternary form is sanctioned (since it's already in main) is a
 * `needs-decision` question.
 *
 * Measured: sandbox --disable all = 2, build_insns = 108, target_insns = 107.
 * 1 extra instruction (the `andi $v1, $v0, 0xFFFF` that materializes the u16
 * form from the lh-loaded $v0); without that extra insn, count would match.
 * Kind area perfect (matched lhu $a1; andi $v1, $a1, 0xFFFF); ternary inner
 * matched target's addiu+sll pattern (no lui-fold). Lowest score reached
 * across 41 variants this session.
 *
 * Also flagged as a SECONDARY concern but not deciding: `u32 kind_full = lhu;
 * u32 kind = kind_full & 0xFFFFU;` (redundant 0xFFFF mask of an already-
 * zero-extended lhu). The reviewer noted this looks intentional codegen
 * provocation but is dual-use with split-init-accumulation, which IS sanctioned.
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
        s32 v = *((u16 *)(arg0 + 0x270));
        s32 t;
        if (probe >= 4) {
            v = 3;
        }
        t = v << 16;
        idx = t >> 15;
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
