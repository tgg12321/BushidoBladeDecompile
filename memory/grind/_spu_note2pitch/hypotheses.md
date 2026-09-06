# Hypothesis ledger — _spu_note2pitch

## s1 (2026-09-06, recon)
- H1 KILLED (instance): longhand copy of the 0x103B loop inside the function
  (forms B/C with an s32 abs local; D/E as a static-inline helper with an
  if-form abs). Measured 21 / 21 / 23 / 23 on the HEAD chassis, no FAKE
  constructs. Frame 8/0 vs target 16/8; the if-form abs splits the block.
- H2 CONFIRMED: sibling `_spu_2pitch` marked GNU89 `inline` is the inlinee;
  the sibling stays byte-identical (sandbox 0). Score 37 -> 18 (form A).
- H3 CONFIRMED: divmod-coalesce-reuse-var (`rem = absdiff/1536; oct = rem;
  rem = absdiff - oct*1536`) reproduces the quotient move `addu a0,v1`.
- H4 CONFIRMED: the abs is fold-const's `(rem<0)?-rem:rem` -> ABS_EXPR ->
  mips abssi2 single insn; passing it as the second actual gives score 2
  (forms F/G, both ternary spellings identical).
- H5 KILLED (instance): `base &= 0xFFFF` (u32 base), and u32-arms + u16
  round-trip (`base = atten; atten = base;`) — the mask lands on the arms' own
  pseudo (andi a2,a2). Measured 3 / 3, HEAD chassis, no FAKE.
- H6 KILLED (instance): `pitch = base; pitch = _spu_2pitch(pitch, ..)` — the
  actual overlaps the call target, integrate.c:1305 copies it, seats rotate.
  Measured 16, HEAD chassis, no FAKE.
- H7 OPEN (frontier #1): the u16->u32 mask must write a MULTI-SET user
  variable that is passed as the actual (not the call target). Form L proves
  the ordering flips (score 6) but its second set (clamp into the same var)
  costs a tail move. Find the byte-neutral live second set. Not yet measured:
  (a) the receiver's second set consumed BEFORE the arms (a value used in the
  prefix), (b) the receiver being a variable the abs or clamp already writes
  without a copy, (c) an integrate.c path where the parm-copy pseudo itself
  gets two sets (read integrate.c 1440-1460 + copy_rtx_and_substitute parm
  handling), (d) re-check the birthing test's `bb_live_regs` update rule in
  sched.c schedule_block — if the dest can be non-live at readiness there may
  be an ordering lever that needs no second set.
- H8 OPEN (frontier #2, lower priority): a byte-neutral respelling of the
  sibling `_spu_2pitch` that makes `ratio`'s init single-set (so the li is
  ALSO boosted and the LUID tie-break applies). Constraint: the loop must keep
  multiplying by the literal 0x103B (shift/add chain) and updating one running
  variable, so this looks closed; measure only if H7 dies.

## [s1] Longhand copies of the 0x103B curve loop inside _spu_note2pitch (forms B/C, and D/E as a static-inline helper with an if-form abs) reproduce the target's frame and abs placement.
- mechanism: integrate.c inline path vs local expansion: only the inlined sibling copy yields the 16-byte frame with the upper spill at 0x8($sp); an if-statement abs is a real branch that splits the block while the target's bgez/move/negu is the single mips abssi2 insn.
- probe: sandbox _spu_note2pitch --disable all on forms B, C, D, E (tmp/grind/_spu_note2pitch/s1/vB.body.c .. vE.body.c)
- result: 21 / 21 / 23 / 23; frame 8 with spill at 0($sp) in all four; abs emitted as a branch.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 (src/main.c INCLUDE_ASM replaced by the form; no FAKE constructs present)

## [s1] Marking the sibling _spu_2pitch (defined immediately above) GNU89 `inline` makes GCC 2.7.2 inline it into _spu_note2pitch and reproduces the target's loop, spill slot and frame while leaving the sibling byte-identical.
- mechanism: integrate.c expand_inline_function (save_for_inline_copying path: function is both inlined and emitted out-of-line).
- probe: form A (inline keyword + call) then sandbox _spu_2pitch --disable all with the keyword in place
- result: _spu_note2pitch 37 -> 18; _spu_2pitch sandbox = 0 with the keyword; loop/spill/frame all match.
- verdict: CONFIRMED

## [s1] The target's quotient move (addu a0,v1) and remainder reuse of v1 come from the divmod-coalesce-reuse-var spelling rem = absdiff/1536; oct = rem; rem = absdiff - oct*1536.
- mechanism: documented in memory/reference/divmod-coalesce-reuse-var.md (quotient born in the reused var's reg, copy to the call-visible var).
- probe: form A vs the plain oct = absdiff/1536; rem = absdiff - oct*1536 form
- result: the extra move appears and the divmod region matches byte-for-byte.
- verdict: CONFIRMED

## [s1] The target's bgez v1 / move v0,v1 / negu v0,v0 mid-block is the mips.md abssi2 single-insn template produced by fold-const.c turning (rem < 0) ? -rem : rem into ABS_EXPR; passing that ternary as _spu_2pitch's second actual reproduces it.
- mechanism: fold-const.c:4861-4888 (A op 0 ? A : -A -> ABS_EXPR), expr.c abs_optab, tools/gcc-2.7.2/config/mips/mips.md define_insn abssi2; single insn so sched places it inside the block.
- probe: forms F and G (both ternary spellings)
- result: score 2 for both; abs bytes and position match; remaining diff is one insn-pair order.
- verdict: CONFIRMED

## [s1] Masking the u32 base in place (base &= 0xFFFF) or round-tripping the u32 arms value through a u16 (base = atten; atten = base;) reproduces the target's andi a2,v0 ordering and seats.
- mechanism: makes the andi's destination a multi-set pseudo (not sched1-boosted) but source and destination become the same pseudo.
- probe: forms H and M, sandbox
- result: 3 and 3: andi/li order now matches but the arms write a2 and the mask is andi a2,a2; target arms write v0 and andi a2,v0.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with inline _spu_2pitch chassis; no FAKE constructs

## [s1] Reusing the result variable as the actual (pitch = base; pitch = _spu_2pitch(pitch, ...)) gives the multi-set receiver the target needs without extra copies.
- mechanism: integrate.c:1305 copies an actual that overlaps the call target into a fresh pseudo, so the receiver is not the parm register.
- probe: form K, sandbox
- result: 16: seats rotate (atten a1, i a2) plus an extra addu v1,a1 copy.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with inline _spu_2pitch chassis; no FAKE constructs
