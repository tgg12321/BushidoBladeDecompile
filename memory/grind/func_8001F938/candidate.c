/* func_8001F938 (code6cac.c) — CLEAN best form, honest sandbox floor 8 (build_insns 105).
 * Session s2 (structural). This is the best form using ONLY reviewer-PASSED constructs
 * (no reviewer-FAILED dual-typed read). It is what src/ is left at.
 *
 * IMPORTANT CORRECTION (s2): the s1 ledger recorded "floor 5 with clean pure-C levers
 * (kind-split + branch-flip)". That 5 was NOT clean — it always relied on the guarded
 * dual-typed read `else raw_or_3 = (u16)*p;` (HEAD carried it). A FRESH adversarial
 * cheat-reviewer FAILED that construct this session (Tests 1/3/5 — no semantic purpose;
 * see rejected/guarded-dual-typed-read.c + tmp/grind/func_8001F938/s2/cheat_reviewer_verdict.txt).
 * With that construct EXCLUDED, the honest floor is 8, not 5.
 *
 * PASSED constructs kept here:
 *   1. kind-split — `u32 kind_full = lhu(0x6A); u32 kind = kind_full & 0xFFFF;`
 *      Reviewer PASS: legitimate split-init-family reuse (kind_full -> range checks,
 *      kind -> == checks; mirrors target `lhu $a1; andi $v1,$a1,0xFFFF`).
 *   2. branch-sense flip — `if (probe >= 4) raw = 3; else raw = probe;` UNAMBIGUOUS clean C.
 *
 * RESIDUAL 8 root cause (coupled fixpoint, see s2/region_0x270_analysis.txt):
 *   Target's .L8001FA60 emits TWO adjacent same-address loads `lh $v0,0x270; lhu $v1,0x270`
 *   (the lhu also fills lh's load-delay slot). GCC 2.7.2 CSE ALWAYS merges two same-address
 *   HImode reads because they are provably equal in-range (field<4 => top bit 0 => sign/zero
 *   extension identical). No semantically-purposeful pure-C form produces the second load:
 *   union member access (s2, floor 5), two-pointer alias, volatile, and both read orders all
 *   fail. The ONLY C form that emits the two loads is the dual-typed read — which the reviewer
 *   FAILS. => RULING-REQUEST: is the dual-typed read (which target PROVABLY contains) sanctioned
 *   for this function? If sanctioned, the guarded form reaches floor 5 and the unconditional
 *   form floor 2 (bi=108, still +1 vs target 107 — even then not byte-proven).
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
