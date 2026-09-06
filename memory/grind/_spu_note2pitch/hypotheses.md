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

## s2 (2026-09-06, structural) — SOLVED

## [s2] The target's single `andi $a2,$v0,0xFFFF` after the join label .L8008BBB0 truncates BOTH arms' results, so the original source narrowed the octave base to u16 inside each arm of the sign branch rather than once at the call; writing it that way gives the widened receiver two static sets and reproduces the target exactly.
- mechanism: the per-arm narrowing makes the u32 receiver `atten` multi-set at sched1, so sched.c birthing_insn_p (tools/gcc-2.7.2/sched.c:2512-2526, reg_n_sets[i]==1) does not boost the andi to LAUNCH_PRIORITY and the pair falls to the LUID tie-break; jump2 cross-jumping then merges the two identical arm-local andi tails into the single post-join instruction.
- probe: forms vQ/vR (`atten = (u16)(0x1000 << oct);` / `(u16)(0x1000 >> oct);` in the two arms) and vN (`u16 base` per arm + `atten = base;`), sandbox _spu_note2pitch --disable all; discriminated against vF (post-join narrowing) and vS (up-arm narrowing only).
- result: vQ = 0, vR = 0, vN = 0; vF = 2, vS = 4. Sibling `_spu_2pitch` still 0. Full verify-oracle ok:true.
- verdict: CONFIRMED

## [s2] H7 (s1 frontier 1) as posed — "find a byte-neutral LIVE SECOND SET of the receiver, sourced from a prefix value or from the clamp" — is not how the second set arises: the second set comes from the arms themselves once the narrowing is per-arm, so no borrowed carrier and no FAKE construct is needed.
- mechanism: same birthing_insn_p gate; the two sets are the two arms' own assignments to the call actual, which jump2 later re-merges.
- probe: vQ/vR/vN vs s1's form L (clamp written back into the receiver, score 6) and form K (receiver reused as the call target, score 16).
- result: 0 with no borrowed carrier; the s1 candidates for the second set (prefix value, clamp variable) are moot.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with the inline _spu_2pitch chassis; no FAKE constructs present

## [s2] The divmod-coalesce-reuse-var spelling (`rem = absdiff / 1536; oct = rem; rem = absdiff - oct * 1536;`) is required to reproduce the target's quotient move `addu $a0,$v1,$zero`.
- mechanism: memory/reference/divmod-coalesce-reuse-var.md (quotient born in the reused variable's register, copied to the call-visible variable).
- probe: replace it with the plain `oct = absdiff / 1536; rem = absdiff % 1536;` on the per-arm-narrowing chassis and sandbox; also measure the `- oct * 1536` and statement-order variants.
- result: plain `/` + `%` = 0 (the quotient move appears anyway). `absdiff - oct * 1536` = 36 and remainder-statement-first = 37, so `%` and the quotient-first order are load-bearing, but the variable reuse is not. Dropped from the final body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with the per-arm-narrowing body (vR/d1); no FAKE constructs present

## [s2] H8 (s1 frontier 2, respell the sibling to make `ratio`'s init single-set) — not needed; never measured, and now moot: the residual closed from the caller side without touching the sibling, which stays byte-identical with only the GNU89 `inline` keyword added.
- verdict: CONFIRMED (moot)

## s2b (2026-09-06, permuter modality -> sweep_variants; the banned-cast rework)

## [s2b] The target's post-join `andi $a2,$v0,0xFFFF` is the callee truncating a `u16` first parameter of `_spu_2pitch`, so declaring the sibling `inline u32 _spu_2pitch(u16 atten, u32 rem)` produces it without any caller-side construct.
- mechanism: integrate.c would emit the parameter truncation inside the integrated copy rather than the caller emitting a widening.
- probe: change the sibling's first parameter to u16, sandbox both _spu_2pitch and _spu_note2pitch.
- result: sandbox _spu_2pitch = 1 (the sibling's own bytes break) and _spu_note2pitch = 22. The andi is caller-side widening.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with the s1 vF body (u16 base widened at the call); no FAKE constructs present

## [s2b] Duplicating the `_spu_2pitch` call, or the whole call+clamp+return tail, into both arms of the sign branch gives per-arm widenings that jump2 cross-jumping re-merges into the target's single post-join andi.
- mechanism: jump2 cross-jump tail merging of two identical arm tails (the mechanism s2 credited for the banned per-arm-cast form).
- probe: forms vB_call_dup_arms and vE_return_dup_arms, sandbox _spu_note2pitch --disable all.
- result: 37 and 37 (baseline 2). The integrated copy is duplicated and not re-merged.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with the inline _spu_2pitch chassis; no FAKE constructs present

## [s2b] Cast-free respellings that keep a single post-join widening (mask at the call `atten & 0xFFFF`, declaration-order permutations, a ternary actual `diff >= 0 ? up : dn`, a named local for the rem abs, the clamp as a return ternary) reach the target order.
- mechanism: none of them changes reg_n_sets of the andi's destination; sched.c adjust_priority still boosts the single-set parm-copy pseudo.
- probe: forms vD_mask_at_call, vI_declorder, vG_ternary_actual, vF_named_absrem, vH_clamp_expr, sandbox each.
- result: 2, 2, 8, 18, 6. None improves on the baseline 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with the inline _spu_2pitch chassis; no FAKE constructs present

## [s2b] Making the arms write a multi-set variable is what defeats the birthing boost, so writing them into a dead u16 PARAMETER (already set once by the call) and widening once into a fresh u32 receiver reaches the target order without any borrow.
- mechanism: if the arms' own set count mattered, a 3-set u16 param would suffice.
- probe: form vY_atten_from_cenfine (`cen_fine` written in both arms, `atten = cen_fine;` once after the join), sandbox.
- result: 2 — identical to the baseline. The set count that matters is that of the andi's DESTINATION, not of its source.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-06 with the inline _spu_2pitch chassis; no FAKE constructs present

## [s2b] Staging the widened octave attenuation through the pre-existing, currently-dead local `diff` (`diff = atten; pitch = _spu_2pitch(diff, ...);`) makes the andi's destination two-set, so sched.c adjust_priority does not boost it, the pair falls to the LUID tie-break, and the function matches byte-for-byte without any cast.
- mechanism: tools/gcc-2.7.2/sched.c:2504-2535 birthing_insn_p (`reg_n_sets[i] == 1` on a live dest) feeding adjust_priority's LAUNCH_PRIORITY boost; the backward list scheduler emits a boosted insn LAST, which is the wrong order for this pair. Family: .claude/rules/staged-value-reused-variable.md.
- probe: forms vR_diff_recv / vY_diff_recv_atten and the carrier sweep (absdiff, tgt, cen, oct, pitch), sandbox _spu_note2pitch --disable all, then sandbox _spu_2pitch and full verify-oracle.
- result: `diff` = 0; absdiff = 5; tgt = 8; pitch = 16; cen = 28; oct = 28. With `diff`: sandbox _spu_note2pitch = 0, sandbox _spu_2pitch = 0, verify-oracle ok:true.
- verdict: CONFIRMED
