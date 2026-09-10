/* REJECTED (s1, 2026-09-10): a named intermediate for the D_800A3590[] read
   does NOT defeat loop.c's strength reduction of the array-address giv.
   Measured 101 -> 101 (no change) on the IconC70=0x20 chassis.
       s32 h;
       h = D_800A3590[var_s0];
       prim.p_static = t + (h << 4);
   vs the plain  prim.p_static = t + (D_800A3590[var_s0] << 4);
   Both produce "giv at 319 reduced to (reg:SI 159)" and both burn a 7th
   callee-saved register ($s3 = induction pointer into D_800A3590), leaving
   frame 0x88 where target is 0x80. The reduction decision is made in
   loop.c strength_reduce on the RTL giv, which is identical either way. */
