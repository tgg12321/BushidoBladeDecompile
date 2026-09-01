/* REJECTED s1: candidate.c form with `n = s[0]; s[0] = n + 4;` moved ABOVE `cnt -= avail`
   (intended to invalidate the cnt==n cse equivalence so the subtract is in-place) — scored 20,
   WORSE than candidate's 13. It DID produce the in-place `subu a2,a2,t0` and sched1 hoisted it
   to the arm top like target, but the cnt entry-copy got scheduled INTO the avail-load delay
   slot (`move a2,a1` after the lw), keeping hard $a1 live past the avail load, so avail
   conflicts h5 and can never seat $a1; seats rotated to s→$a3/avail→$t0/cnt→$a2.
   Same 20 for the variant with a fresh merged `w` local (w = s[0]; ... w = *w) instead of
   param reuse + `n -= avail`: w seats $a1 and n's pref is purged (n→$a2 ✓ mechanism), but the
   copy-into-delay-slot problem produces the same avail/h5 conflict. Target has an UNFILLED
   load-delay nop (0x2E094) with both entry copies above the lw — our sched1 breaks that tie
   the other way. The lever for the residual is making the n-copy stay above the avail load
   (or the equivalent seating), not source-order shuffling of the subtract. */
