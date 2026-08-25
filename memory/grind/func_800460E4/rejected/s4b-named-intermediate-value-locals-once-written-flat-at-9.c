/* DEAD (s4b, measured): the route the 2026-08-25 06:39 ruling left explicitly
   open - "a byte-neutral (248-insn) fresh named pointer local holding a real
   consumed address stays available under the named-intermediate entry's
   prongs" - is CLOSED. Every ONCE-written named intermediate, value or
   pointer, in every order/scope, measures diffs=9 (probe4 v1-v8, probe5 w1-w7,
   probe9 k4). 248-insn byte-neutral variants exist (w1/w5/w6/w7/v6) and are
   STILL 9. Single-assignment is the disqualifier: adjust_priority's birthing
   boost keys on reg_n_sets[regno] == 1, and a once-written intermediate keeps
   the boost. Representative form: */
case 3: {
    s32 hoff = s0[s3 - 2];
    s32 loff = s0[s3 - 1];
    s1 = s2;
    s6 = (s32 *)((u8 *)s0 + ALIGN4(hoff));   /* 245 insns, diffs=9 */
    s4 = (s32 *)((u8 *)s0 + ALIGN4(loff));
    D_8009947A = 1;
    break;
}
