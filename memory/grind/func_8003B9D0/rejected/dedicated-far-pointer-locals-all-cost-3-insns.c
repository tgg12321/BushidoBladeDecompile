/* REJECTED (session 4, K10) — four DEDICATED-pointer spellings of the +0x44C
 * access.  All four DO suppress the cse symbol fold (all three displaced sites
 * come out as target's `1100($base)` register+displacement), but all four
 * compile to 188 insns vs target's 185 — three instructions too many — and the
 * one measured in the sandbox scored 6, i.e. no better than the session-3
 * floor.  Measured with the full-TU pipeline
 * (tmp/grind/func_8003B9D0/s4/sweep2.py + sweep3.py, exact Makefile flags).
 *
 *   variant                                        regdisp_1100  insns  sandbox
 *   00 candidate (saved_44c = eda[0x226])                    0     188      6
 *   01 block-scope `s16 *far;`, assigned after the 0-read    3     188      6
 *   02 block-scope `s16 *far;`, assigned before the 0-read   3     188      -
 *   06 block-scope `s16 *far = &eda[0x226];` (init at decl)  3     188      -
 *   07 block-scope `u8 *far` + `*(s16 *)far`                 3     188      -
 *   A  function-scope `s16 *far;`, all 3 accesses via it     3     188      -
 *   B  function-scope `s16 *far;`, read only via it          3     188      -
 *   -- vs the ACCEPTED form: stage through the existing `u8 *p` scratch local
 *   C  p = (u8 *)&eda[0x226]; saved_44c = *(s16 *)p;         3     185      0
 *   D  same with `p` declared `s16 *` and the later uses cast 3    185      -
 *
 * So the fold-suppression and the instruction count are TWO SEPARATE effects.
 * Any derived-address local suppresses the fold; only staging it through the
 * function's already-live scratch pointer also lands on target's length.  A
 * fresh dedicated local costs an extra live pointer across the call and three
 * insns of address bookkeeping.
 *
 * Recorded because the dedicated-pointer spelling is the "obvious clean" form
 * and a future session will reach for it first; it is measured dead.
 */

/* the representative rejected body (variant A) */
void func_8003B9D0_rejected_dedicated_far(void) {
#if 0
    s16 *far;   /* function-scope dedicated pointer */
    ...
        if (qf & 0x30) {
            s16 *eda = &D_80101EDA;
            saved_first = eda[0];
            far = &eda[0x226];
            saved_44c = *far;
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) *far = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            *far = saved_44c;
        }
#endif
}
