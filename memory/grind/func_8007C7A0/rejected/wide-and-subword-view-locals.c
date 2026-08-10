/* func_8007C7A0 — REJECTED s12 (2026-08-10, structural): remaining view-
 * local width/access spellings of the X clamp, all measured on the s6
 * floor-5 chassis (honest sandbox, display.c context, target 51):
 *
 *   P3  flat else-if chain `if (arg0<0) arg0=0; else if (lim-1<arg0) ...`
 *       -> 9 @ 50 (branch region restructures; nested-if shape is
 *       load-bearing).
 *   P4  s32 wide view `s32 x = arg0;` + three statement arms
 *       -> 13 @ 49 (the s32 copy IS the sign-extension; the explicit
 *       sll/sra pair and downstream shape fold away).
 *   P5  sub-word view `s16 x = *(s16 *)&arg0;` (sanctioned family)
 *       -> 14 @ 54 (&arg0 forces a stack home: prologue sw + reloads;
 *       target's 16-byte frame is phantom, no stores).
 *
 * None reaches the join; none beats 5. Representative body (P4): */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    {
        s32 x = arg0;
        if (x >= 0) {
            if ((D_8009BE78 - 1) < x) {
                arg0 = D_8009BE78 - 1;
            } else {
                arg0 = x;
            }
        } else {
            arg0 = 0;
        }
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
