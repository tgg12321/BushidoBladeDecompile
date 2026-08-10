/* func_8007C7A0 — BEST LEGITIMATE FORM (s7, 2026-08-10, rederive):
 * sandbox --disable all == 5, build_insns 50 vs target 51, 21 rules dropped,
 * cheat-asm stripped. Honest floor improved 12 -> 5 this session.
 *
 * THIS IS NOT THE 0-FORM. The byte-exact form (sandbox 0, s6) required the
 * x/tx param-alias-and-writeback pair which the layer-1 cheat-reviewer FAILED
 * and the driver BANNED (see rejected/layer1-fail-0810-1436.c and the s7
 * evidence entry). This body is the cleanest natural spelling: clamp both
 * params in place (symmetric X/Y, the SOTN/PsyQ CLAMP house style), per-arm
 * returns with block-scoped hi/lo (the s6 chassis, which is NOT banned and
 * is what dissolved the old 12-form residual).
 *
 * The single missing instruction vs target (and the 5 masked diffs that
 * cascade from it) is the X-clamp three-arm JOIN TEMP + writeback copy:
 * target routes all three X arms through $v0 (addiu v0,a2,-1 / move v0,a3 /
 * move v0,zero) then copies home with `move a3,v0`; every legitimate spelling
 * measured (7 distinct forms, s7 evidence) collapses the join — GCC 2.7.2
 * expands clamp assignments directly into the target variable's pseudo, so
 * the arms write $a3 directly (3 insns instead of 4). The join survives ONLY
 * when the source has a distinct temp written back into a live-initialized
 * variable — exactly the banned construct. Ruling requested (see outcome).
 *
 * All other 47 instructions match target 1:1 including the full Y clamp,
 * dispatch, both mask arms, cross-jumped tail, and delay-slot fills
 * (s7/target_51.s vs s7/ours_5form_50.s side-by-side).
 */

/* PsyQ 4.0 LIBGPU SYS: get_cs (static) — verbatim-linked Sony object
 * (census 2026-07-09); C ref: ground-up reconstruction (no published
 * reference matches this library build). */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            arg0 = D_8009BE78 - 1;
        }
    } else {
        arg0 = 0;
    }

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    if ((u32)(D_8009BE74 - 1) < 2U) {
        s32 hi = arg1 & 0xFFF;
        s32 lo;
        hi = hi << 12;
        lo = arg0 & 0xFFF;
        lo = lo | 0xE3000000;
        return hi | lo;
    } else {
        s32 hi = arg1 & 0x3FF;
        s32 lo;
        hi = hi << 10;
        lo = arg0 & 0x3FF;
        lo = lo | 0xE3000000;
        return hi | lo;
    }
}
