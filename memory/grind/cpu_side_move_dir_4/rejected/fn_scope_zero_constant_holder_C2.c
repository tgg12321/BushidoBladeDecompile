/* s47 C2 KILLED: `s32 zero = 0;` fn-body-top + `v0 = zero;` at success arm.
   Constant-holder shape (SOTN DispSamnailWindow archetype): zero must
   survive across the debug_printf + cdrom_ClearIrq calls to reach the
   success arm read, forcing a callee-save reservation.
   Result: masked=10 (+8), build_insns=163 (+3 insns).
   Mechanism: keeping zero live across block=3's calls costs a callee-save
   reserve + extra move insns; the alloc web falls out of h5 basin.
   The mechanism does add fn-scope refs to a hard-reg, but the cost of
   preserving is >> the LUID/LAUNCH benefit. */
s32 zero = 0;
/* ... v0 = zero; at success arm ... */
