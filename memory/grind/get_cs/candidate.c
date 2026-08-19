/* func_8007C7A0 — BEST LEGITIMATE FORM (re-banked s11, 2026-08-10, synthesis):
 * sandbox --disable all == 5, build_insns 50 vs target 51, 21 rules dropped,
 * cheat-asm stripped 153. Re-proven at HEAD 69750c7c THIS session
 * (tmp/grind/func_8007C7A0/s6/sandbox_floor5_synthesis.json).
 *
 * THIS IS NOT THE 0-FORM, DELIBERATELY. Three distinct sandbox-0 forms exist
 * and ALL are banned by layer-1 rulings the driver enforces mechanically:
 *   1. x/tx param-alias + writeback (s6)      -> rejected/layer1-fail-0810-1436.c
 *   2. temp->param writeback, no alias (s8 m1) -> rejected/temp-param-writeback-byteproven-superseded.c
 *   3. published-SOTN ternary self-read clamp (s8-s10) -> rejected/ternary-clamp-family.c
 * All three are spellings of the same join-temp-writeback dataflow; the layer-1
 * ruling covers the family under ANY spelling. Do NOT re-propose any of them,
 * and do NOT invent a fourth spelling of the same dataflow — the s7 measurement
 * campaign (8 spellings) plus s8's m1/m2 established that EVERY C dataflow
 * reaching the last instruction routes a distinct temp back into a
 * live-initialized variable, i.e. any 0-form is in the banned family. The only
 * sanctioned continuations are the Judge's own listed axes (see ledger).
 *
 * This body is the cleanest natural spelling: clamp both params in place
 * (symmetric X/Y), per-arm returns with block-scoped hi/lo (the s6 chassis,
 * which is NOT banned and dissolved the old 12-form residual). 47/51
 * instructions match target 1:1 (prologue, both limit loads, full Y clamp,
 * dispatch, both mask arms, cross-jumped tail, delay-slot fills; side-by-side
 * in tmp/grind/func_8007C7A0/s7/target_51.s vs ours_5form_50.s). The single
 * missing instruction + 5 cascading masked diffs = the X-clamp three-arm JOIN
 * TEMP + writeback copy (target: addiu v0,a2,-1 / move v0,a3 / move v0,zero
 * then move a3,v0; ours: arms write $a3 directly, one insn fewer).
 */

/* PsyQ 4.0 LIBGPU SYS: get_cs (static) — verbatim-linked Sony object
 * (census 2026-07-09); C ref: ground-up reconstruction. */
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
