/* REJECTED: `if (D_800A38DC != 5)` with exchanged arms scores WORSE (19-38 vs
   13-16). Target's branch is `bne v1,5` (fall-through = the ==5 if-arm), so
   the source MUST be `if (D_800A38DC == 5)` with the E6A4/*6 body in the
   if-arm. Swapping the sense flips fall-through and regresses register/branch
   alignment across every duplicated/hoisted chassis tried. */
