/* REJECTED s13 (driver session 8, 2026-08-10): named-limit local on the
 * 5-chassis. sandbox --disable all = 16 @ build 51 (vs floor 5 @ 50).
 * Reaches the target insn COUNT (51) but wrecks the stream: the hoisted
 * xlim pseudo restructures both clamp regions' allocation. The s5 kill of
 * named limits was measured only on the 12-form (CSE tie at 12); this
 * measurement closes the named-limit axis on the 5-chassis too, in a
 * different failure mode. Do not re-propose named-limit spellings on any
 * chassis of this function. */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 xlim = D_8009BE78 - 1;

    if (arg0 >= 0) {
        if (xlim < arg0) {
            arg0 = xlim;
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
