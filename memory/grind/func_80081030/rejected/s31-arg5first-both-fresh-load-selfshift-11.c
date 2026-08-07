/* KILLED s31: arg5-first with BOTH shifts via fresh load-then-self-shift = masked 11.
 * s30 v03 proved single-side (arg5 only) fresh-carrier avoids launch penalty at floor 4;
 * applying the pattern symmetrically to BOTH temps + swapping source order to arg5-first
 * still pays the arg5-first-seats-trade coupling (+7 masked). The launch-pathology
 * escape does NOT compose with source-order flip. See s31 notes. */
