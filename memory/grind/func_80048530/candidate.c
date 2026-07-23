/* func_80048530 — CLEAN pure-C form (no pins, no __asm__, no barriers).
 * sandbox --disable all = 1  (s3, 2026-07-23). NEW FLOOR: improved from the
 * prior clean floor of 10 by (a) computing the walker as a fresh assignment
 * `arg0 = base + off` (NOT the `arg0 += off` accumulate) and (b) reading c,d
 * with the SAME walking-pointer idiom as a,b — advance arg0 by 2 after every
 * halfword and read `*arg0` (never a fixed `*(arg0+2)` offset), casting (s16)
 * at the call symmetrically with a,b.
 *
 * Why this reaches floor 1 (build 47 == target 47 insns):
 *   - `arg0 = base + off` (fresh assign) instead of `arg0 += off` (accumulate)
 *     changes the walker RA so c,d NO LONGER fold to `lh`: they emit
 *     lhu+sll+sra like a,b (the s1/s2 "combine lh-fold wall" was an artifact of
 *     the accumulate form + the fixed-offset d read, NOT a hard wall).
 *   - advancing the walker between c and d (`c=*p; p+=2; d=*p;`) kills the
 *     walker at the d-load, so GCC reuses it: `lhu v0,0(v1); lhu v1,2(v1)` —
 *     exactly target's routing (old frontier F1, now genuinely satisfied).
 *   - `move t0,v1` schedules early (prologue), `move a1,a3` lands in place.
 *
 * THE SOLE RESIDUAL (score 1): the walker-relocation add.
 *   build : addu $v1,$v1,$v0   (base-first)
 *   target: addu $v1,$v0,$v1   (off-first)
 * The two operands (off = the stored relative offset, base = arg0) compute the
 * same value; only off-FIRST matches. The only C that emits off-first WITH the
 * v1-walker routing is `arg0 = off + base` — a commutative-operand-order
 * swap (or-tree-shape-shift, FORBIDDEN; banked rejected/offbase-operand-
 * shuffle-cheat.c, byte-0). cc1psx (the original compiler) ALSO emits base-
 * first from the natural `base + off` (tmp/grind/func_80048530/s3/n1.psx.s),
 * so target required off-first *source*; no non-shuffle spelling reaches it
 * (every off-first structural variant misroutes the walker to a1/t0 and
 * cascades — see rejected/offfirst-structural-misroutes.c).
 *
 * => honest pure-C floor = 1; residual is a single commutative-operand-order
 *    RA tie whose only closer is the forbidden shuffle. Endgame-lock species
 *    (.claude/rules/endgame-lock-disposition.md): scan_hand_coded LOW 1/8,
 *    no SOTN precedent for ADD operand-order-for-codegen. Next: permuter from
 *    this floor-1 base; then owner-escalation if permuter is also dead. */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    arg0 = base + off;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (arg2 >= (u32)count) return -1;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = (s32)*(u16 *)arg0;
    arg0 += 2;
    d = (s32)*(u16 *)arg0;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
