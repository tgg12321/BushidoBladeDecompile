/* REJECTED (s8, rederive) — structurally DIFFERENT shape, SCORE-EQUIVALENT to
 * candidate.c. Not a cheat; a measured KILL of the "control-flow reshape moves
 * Region A / A'" hypothesis.
 *
 * Source: fresh m2c decompile of asm/funcs/damage_DebugDisp.s reconstructs the
 * outer search loop as a NEGATED compare with the increments INSIDE the if-block
 * and a goto loop-back (no `break`):
 *     loop_1: { sum=0; bp; j; accumulate; }
 *             if (sum != chk) { chkptr++; i++; offset+=0x24; if (i<3) goto loop_1; }
 * This gives the outer loop a different basic-block layout than candidate.c's
 * `do { ...; if (sum==chk) break; incr; } while (i<3)`.
 *
 * MEASURED:
 *   - goto shape, sum=0 PLAIN (no do-while0):  sandbox --disable all = 8
 *     (Region A sum/j $a0<->$a1 swap UNSOLVED — identical to the plain break form;
 *      control-flow shape does NOT change the inner-loop allocation, as expected:
 *      the inner loop is byte-identical target vs build so sum/j refs are fixed).
 *   - goto shape + do { sum=0; } while(0):     sandbox --disable all = 6
 *     objdump residual is IDENTICAL to candidate.c: Region A solved (sum=$a0,
 *     j=$a1 in the inner loop), residual = A' preheader emit order (build j,bp,sum
 *     vs target sum,bp,j) + Region B (moves-vs-consts). The different outer BB
 *     layout does NOT change sched1's preheader LUID ordering.
 *
 * CONCLUSION: the m2c/negated-goto shape is a genuine structural alternative but
 * score- and residual-equivalent to the break form. Region A' emit order is set by
 * the preheader inits' own LUIDs (fed by the do-while0 def relocation), not by the
 * surrounding outer-loop control-flow structure. Rederive via m2c does not break
 * the s6/s7 A' coupling.
 */
s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
loop_1:
    {
        s32 sum;
        u8 *bp;
        u32 j;

        j = 0;
        bp = base + offset;
        do { sum = 0; } while (0);   /* floor-6 lever; PLAIN sum=0 -> floor 8 */
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum != *(s32 *)((u8 *)chkptr + 0x6C)) {
            chkptr++;
            i++;
            offset += 0x24;
            if (i < 3) {
                goto loop_1;
            }
        }
    }
    /* ... tail + Region B identical to candidate.c ... */
    return 1;
}
