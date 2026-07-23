/* REJECTED (s3, 2026-07-23) — cheat-reviewer FAIL. sandbox --disable all = 0
 * (byte match) but ONLY via a forbidden commutative-ADD operand shuffle.
 *
 * This form is IDENTICAL to candidate.c EXCEPT the walker advance is
 *   arg0 = off + base;      // instead of  arg0 += off;
 * which flips the emitted `addu v1,v1,v0` (walker first) to `addu v1,v0,v1`
 * (off first) to match target — a byte-0 match.
 *
 * WHY IT'S A CHEAT (or-tree-shape-shift family, .claude/rules/, applied to +):
 *  - `off + base` and `base + off` compute the identical value; the chosen
 *    order has ZERO semantic content. It was picked by ENUMERATION (`base + off`
 *    was measured to NOT match). That is exactly the forbidden pattern.
 *  - Justification is purely GCC-internal: expand emits an accumulate
 *    (dest==operand) dest-first, so making the add a NON-accumulate (dest arg0
 *    != operands off,base) lets the written order survive. No program-logic
 *    reason for off-before-base.
 *  - NO sibling anchor: func_800483DC's `addu $a1,$a1,$s0` is accumulator-FIRST
 *    (the offset/idx IS the accumulator), the opposite of the accumulator-second
 *    order target needs here.
 *  - NO SOTN-master precedent for ADD operand-order-for-codegen; or-tree-shape-
 *    shift explicitly covers ADD.
 *
 * Kept only as the measured proof that the byte-0 exists and that its sole
 * closer is a forbidden shuffle -> the honest pure-C floor is 1, and the
 * residual is an endgame accumulate-operand-order tie (candidate.c KILL note). */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base;
    s32 count;
    s32 entry;
    s32 a, b, c, d;
    s32 off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    arg0 = off + base;   /* CHEAT: operand-order shuffle, no semantic anchor */
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
