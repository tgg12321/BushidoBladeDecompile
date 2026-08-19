/* REJECTED s14 (2026-08-10, permuter, driver session 9): hoisted-cond+limit
 * pair in the X-clamp outer arm — the best find of the s9 DIRECTED campaign
 * (permuter weighted 500 vs base ~700, ws permuter/c7a0_s9_directed,
 * output-500-1). Honest sandbox in display.c context: 19 @ 49 (baseline
 * 5 @ 50) — KILLED. GCC folds `cnd` into the branch and the pre-computed
 * `lim` collapses an insn (49 < target 51); the join temp does NOT
 * materialize. The permuter's weighted metric rewarded the named
 * intermediates, but the honest masked distance is 14 worse than the floor.
 * All constructs here are live/legitimate (no banned-family dataflow):
 * the kill is codegen, not policy. */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    int cnd;
    int lim;

    if (arg0 >= 0) {
        cnd = (D_8009BE78 - 1) < arg0;
        lim = D_8009BE78 - 1;
        if (cnd) {
            arg0 = lim;
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
