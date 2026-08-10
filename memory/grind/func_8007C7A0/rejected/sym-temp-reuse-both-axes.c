/* s8 m2 — REJECTED: sandbox 11 at 51/51 (vs floor 0). One reused s16 result
 * temp `cs` for BOTH clamps with writebacks into the params. The long-range
 * multi-use cs pseudo restructures the whole allocation (11 masked diffs),
 * NOT the predicted single extra Y-join insn. The X/Y asymmetry of the
 * target (X joins through $v0, Y writes direct) is load-bearing; symmetric
 * temp routing cannot match. Superseded entirely by the byte-exact SOTN
 * reference text (candidate.c). Do not re-propose. */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 cs;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            cs = D_8009BE78 - 1;
        } else {
            cs = arg0;
        }
    } else {
        cs = 0;
    }
    arg0 = cs;

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            cs = D_8009BE7A - 1;
        } else {
            cs = arg1;
        }
    } else {
        cs = 0;
    }
    arg1 = cs;

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
