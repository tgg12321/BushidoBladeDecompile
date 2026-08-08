/* REJECTED s1 (2026-08-08): K&R (old-style) parameter definition — INERT.
 *
 * Motivation: Sony provenance (get_cs, PsyQ 4.0 LIBGPU SYS) suggested the
 * original might be a K&R definition; K&R shorts are promoted to int and
 * narrowed at entry, which could have produced different prologue RTL and a
 * different register allocation.
 *
 * Measured: identical honest sandbox scores to the ANSI forms on BOTH
 * inherited bodies — this K&R stream51 spelling scored 15 build 51 (== ANSI
 * stream51), and the K&R form of candidate.c scored 12 build 50 (== ANSI
 * candidate). GCC 2.7.2 (our canonical cc1 + flags) lowers K&R-promoted
 * short params identically to ANSI s16 prototype params for this leaf.
 * Axis dead; do not re-try other old-style spellings (register keyword on
 * params was not tried but the narrowing RTL is the same path).
 */
s32 func_8007C7A0(arg0, arg1)
s16 arg0, arg1;
{
    s16 x = arg0;
    s16 tx;
    s32 hi;
    s32 lo;
    s32 pkt;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            tx = D_8009BE78 - 1;
        } else {
            tx = x;
        }
    } else {
        tx = 0;
    }
    x = tx;

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    hi = arg1 & 0xFFF;
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        hi = arg1 & 0x3FF;
        hi = hi << 10;
        lo = x & 0x3FF;
    } else {
        hi = hi << 12;
        lo = x & 0xFFF;
    }
    pkt = lo | 0xE3000000;
    return hi | pkt;
}
