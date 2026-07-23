/* func_8001F938 — REJECTED permuter find (grind s4, permuter modality).
 *
 * Best form the permuter converged to over ~95k iterations from the clean
 * floor-8 chassis: sandbox/permuter weighted score 320 (base 615). NOT a
 * match — nowhere near 0 — and it only got there by injecting TWO cheats:
 *
 *   1. `volatile short pad;`  — a dead, no-semantic-purpose local named "pad"
 *      whose ONLY effect is forcing GCC to reserve the 8-byte stack frame that
 *      target has (addiu sp,sp,-8 / +8). Dead-vars/frame-coercion + volatile
 *      coercion family; naming ("pad") announces coercion intent. CHEAT.
 *   2. `raw_or_3 = raw_or_3;` in the else arm — a dead self-assignment
 *      (register-alloc Lever-D / duplicated-no-op). No observable effect.
 *      CHEAT.
 *
 * WHY IT'S A DEAD END (the real finding of s4):
 *   - Even WITH both cheats the score is 320: the +0x270 fold gap is UNTOUCHED
 *     (the single s16 read still folds (raw<<16)>>15 -> raw*2). The permuter
 *     never generated the banned dual-typed read (the only distance-0 form),
 *     confirming random type-mutation search does not stumble onto a clean
 *     sub-8 lever, exactly as the s1-s3 dichotomy predicts.
 *   - The 8-byte frame is NOT an independent clean lever: in the true match it
 *     is a byproduct of the dual-read's spill slot. The permuter could only
 *     manufacture the frame via the volatile-pad cheat, which buys nothing
 *     toward a byte match (still 320) and is forbidden by any spelling.
 *
 * Disposition: rejected per cheat catalog (no-new-park-categories,
 * inline-asm-policy expanded catalog). src/ kept at the clean floor-8 form
 * (candidate.c). func_8001F938 stays INCOMPLETE; the structural + permuter
 * axes are both now exhausted for a clean sub-8. Only the non-structural
 * SOTN signedness-split-family census remains (owner-gated).
 *
 * Raw find: tmp/grind/func_8001F938/s4/ws/output-320-*/source.c
 */
void func_8001F938(u8 *arg0)
{
    u32 kind_full;
    u32 kind;
    volatile short pad;            /* CHEAT: dead frame-forcer, no purpose */
    s32 val;
    s32 a2;
    s32 idx;
    s32 factor;
    kind_full = *((u16 *)(arg0 + 0x6A));
    kind = kind_full & 0xFFFFU;
    a2 = *((s16 *)(arg0 + 0x1C));
    /* ... kind dispatch unchanged (clean floor-8 body) ... */
    {
        s32 probe = *((s16 *)(arg0 + 0x270));
        s32 raw_or_3;
        raw_or_3 = probe;
        if (raw_or_3 >= 4) {
            raw_or_3 = 3;
        } else {
            raw_or_3 = raw_or_3;   /* CHEAT: dead self-assign (Lever-D) */
        }
        idx = ((raw_or_3 << 16) >> 15);
    }
    /* ... remainder unchanged ... */
}
