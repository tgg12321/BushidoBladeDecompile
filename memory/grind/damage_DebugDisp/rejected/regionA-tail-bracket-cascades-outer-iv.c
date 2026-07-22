/* REJECTED (s8, rederive) — score 12 (from floor 6). NOT a cheat concern;
 * measured KILL of the "decouple Region A' by deepening a NON-def sum ref" axis.
 *
 * Idea (from s7 frontier item 1): the do-while(0) on `sum=0` wins Region A but
 * couples Region A' because it relocates sum's DEF to the preheader block-bottom
 * (highest LUID -> sched1 emits sum=0 LAST, opposite of target's sum-first order).
 * To decouple, raise sum's loop-depth-weighted reg_n_refs to 11 via a DIFFERENT
 * reference (the post-loop `sum == chk` compare) while keeping `sum=0` plain at
 * low LUID (so it emits first = target A' order).
 *
 * Two spellings tried, BOTH score 12:
 *   (a) do { if (sum != chk) { chkptr++; i++; offset+=0x24; if(i<3) goto } } while(0);
 *       -> the loop-note bracket raises loop_depth for chkptr++/i++/offset+= too,
 *          disturbing the matched outer IVs (i=a2, chkptr=t0, offset=a3) -> cascade.
 *   (b) the form below: bracket ONLY the compare via an intermediate `matched`,
 *       leaving the increments outside the bracket. Still 12: bracketing the
 *       compare materializes the boolean and/or reschedules the tail lw/beq region.
 *
 * CONCLUSION: the tail region (compare + outer IVs) cannot absorb a loop-note
 * bracket without cascading the outer allocation. The +1 sum ref for Region A
 * MUST come from sum's own def (the sanctioned do-while(0) on sum=0) or the
 * inner accumulate — both of which re-introduce the A'/A coupling. Reconfirms
 * s6/s7 irreducible coupling from a fresh (rederive) structural angle.
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
        s32 matched;

        j = 0;
        bp = base + offset;
        sum = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        do {
            matched = (sum == *(s32 *)((u8 *)chkptr + 0x6C));
        } while (0);
        if (!matched) {
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
