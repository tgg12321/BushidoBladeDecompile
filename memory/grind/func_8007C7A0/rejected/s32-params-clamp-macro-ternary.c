/* REJECTED s2 (2026-08-08, structural) — sandbox 29, build_insns 31 (20 short
 * of target 51).
 *
 * Motivation: target's X-clamp routes all three arms through a $v0 join temp
 * then copies into the param/carrier home (`move $v0,$a3` in-range;
 * `move $a3,$v0` join) — the classic lowering of a CLAMP-macro ternary that
 * REASSIGNS the param (`x = x<0 ? 0 : x>lim-1 ? lim-1 : x;`), which is
 * exactly the SOTN reference get_cs's CLAMP shape transliterated onto BB2's
 * halfword-global limits. Hypothesis: symmetric ternary clamps on both axes
 * could still produce the asymmetric X/Y bytes via coalescing (Y's join
 * copies fold to self-moves when the temp gets $a1).
 *
 * Measured result: GCC 2.7.2 folds the nested ternary far more aggressively
 * than the if/else spelling — the whole function collapses to 31 insns
 * (single shared lim-1, no raw-limit saves, no carrier, compact branch
 * diamonds). The ternary is NOT how the 51-insn stream was produced.
 * Consistent with the round-1 ternary rejection (score 20 on the s16-param
 * X-clamp alone); the param-reassign + s32-param variant folds even harder.
 *
 * Verdict: KILLED. Both s32-param spellings (this + the narrow-view-locals
 * if/else form) regress the STREAM; the s16-param family of
 * candidate_stream51.c remains the only stream-exact spelling class.
 */
s32 func_8007C7A0(s32 arg0, s32 arg1)
{
    s16 x;
    s16 y;
    s32 lo;
    s32 hi;

    x = arg0;
    arg0 = (x < 0) ? 0 : (((D_8009BE78 - 1) < x) ? (D_8009BE78 - 1) : arg0);
    y = arg1;
    arg1 = (y < 0) ? 0 : (((D_8009BE7A - 1) < y) ? (D_8009BE7A - 1) : arg1);
    hi = arg1 & 0xFFF;
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        hi = arg1 & 0x3FF;
        hi = hi << 0xA;
        lo = arg0 & 0x3FF;
    } else {
        hi = hi << 0xC;
        lo = arg0 & 0xFFF;
    }
    {
        s32 pkt = lo | 0xE3000000;
        return hi | pkt;
    }
}
