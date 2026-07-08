/* KILLED s31: t0-first order with BOTH temps via fresh load-then-self-shift carriers = masked 10.
 * s30 v03 confirmed single-side fresh-carrier (arg5 only) preserves floor 4.
 * Applying the pattern to BOTH sides simultaneously regresses +6 masked.
 * Fresh-carrier launch-escape is NOT composable across both temps. */
