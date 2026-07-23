/* func_8001F938 (code6cac.c) — BEST FORM, honest sandbox floor 5 (build_insns 108).
 * Session s1 (recon). HEAD floor was 11. This form drops it to 5 with TWO pure-C
 * levers, both applied here:
 *
 *  1. kind-split  — `u32 kind_full = lhu(0x6A); u32 kind = kind_full & 0xFFFF;`
 *     provokes target's redundant `lhu $a1,0x6A; andi $v1,$a1,0xFFFF` and fully
 *     matches the KIND comparison region ($5/$3 split + the andi). BORDERLINE:
 *     it is a redundant-mask / split-init-family construct; the 2026-06-15
 *     reviewer called it "codegen-steering but secondary, dual-use with the
 *     SANCTIONED split-init-accumulation". Needs a fresh cheat-reviewer PASS
 *     before it can be banked as the clean floor. Removing it alone raises floor
 *     back toward 11.
 *
 *  2. branch-sense flip — write the ternary as `if (probe >= 4) raw_or_3 = 3;
 *     else raw_or_3 = (u16)*p;` (a plain if/else, single guarded read). This is
 *     UNAMBIGUOUSLY clean C and flips GCC's compare from `slti;beqz` to target's
 *     `slti;bnez`. Worth ~1 distance on top of kind-split.
 *
 * RESIDUAL 5 (all trace to ONE root cause — see rejected/dual-type-probe-load.c):
 *   Target's ternary emits TWO unconditional same-address loads
 *   `lh $v0,0x270; lhu $v1,0x270`. GCC 2.7.2 CSE ALWAYS merges two same-address
 *   HImode reads into ONE load, materializing the other signedness in-register
 *   (sign->zero via `andi`, zero->sign via `sll;sra`). Measured this session:
 *     - s16-first guarded  -> lh + move + andi  (this form's shape)
 *     - u16-first          -> lhu + sll;sra     (score 10, worse)
 *   So the second `lhu` is UNREACHABLE from pure C under this compiler. The 5
 *   residual insns are: nop+move (the missing 2nd load), andi-vs-sll delay slot,
 *   and the coupled $v0-vs-$v1 register choice for the `= 3` arm (target's value
 *   lives in $v1 from the lhu; our build derives it into $v0).
 *   The only measured way past 5 (floor 2) was the UNCONDITIONAL split-into-two-
 *   variables preload, which the 2026-06-15 reviewer FAILED as codegen-steering
 *   (rejected/dual-type-probe-load.c) — and even THAT did not produce two loads,
 *   it only matched more register/branch shape.
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
            raw_or_3 = (s32)*((u16 *)(arg0 + 0x270));
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
