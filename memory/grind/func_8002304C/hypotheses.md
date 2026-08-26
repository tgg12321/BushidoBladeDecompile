# Hypothesis ledger — func_8002304C

## s1 (2026-08-26, recon)

- H1 "Naming 0x1F8002B8 as a local hoists it to a prologue callee-save like
  target's s7" — mechanism: loop-invariant constant gets one pseudo + global
  alloc instead of per-use rematerialization. Probe: `s32 lim = 0x1F8002B8;`
  passed at both call sites. Result: 29 -> 19. **CONFIRMED**.
- H2 "Callee-save seats follow declaration position (stable map pos2->s5,
  pos3->s7, pos4->s6, pos5->s4); prologue emission follows init-statement
  order; the two are independently steerable" — probe: decl order
  (scratch, count, lim, scratch_d, scratch_c) with lim/scratch_d initialized
  by later statements. Result: 19 -> 12 -> 6 (with H4). **CONFIRMED**.
- H3 "step's $t0-seat + late sw are caused by sched1 hoisting the named
  local's const birth above the 0x15 compare (widened range conflicts with
  $v0/$v1 compare temps)" — dump-read (.lreg insn 309 hoisted above 313/315;
  greg: reg 158 in hard reg 8). Probe: inline ternary arg
  `(... == 0x15) ? 0x80 : 0x100` births the value in the bne delay slot.
  Result: 6 -> 1 (v0 seat + sw-before-args + a3-in-delay all match).
  **CONFIRMED**.
- H4 "Target's raw-$a0 + masked-$v1 split needs wide->narrow typing
  (s32 mode from the u16 field; u16 m = mode)" — probe: three spellings.
  Result: wide->narrow = distinct $v1 insn (floor 1); both narrow-first
  spellings collapse the copy (score 3, banked in rejected/). **CONFIRMED**
  (as the register/insn-count fix; the opcode is H5's problem).
- H5 "Some same-BB honest spelling makes our combine emit the andi instead of
  simplifying to a copy" — probe: compiler-source read (combine.c:6885-6923,
  724-732) + 3 measured spellings. Result: every current-value path proves
  <=0xFFFF from the lhu; multi-set double-read still ORs to 0xFFFF; the
  full-mask escape needs an intervening LABEL + multi-set, which the 2-insn
  same-BB gap cannot provide. **KILLED** for: plain retype sweeps, explicit
  & 0xFFFF respellings, and same-value double-read reuse (the last by
  mechanism analysis — reg_nonzero_bits ORs two lhu's to 0xFFFF, and with no
  label between set and use the current-value path short-circuits anyway).

## Live frontier (for s2)

- F1 **Label-crossing multi-set mode**: find a semantically-faithful shape
  where the masked value's pseudo is multi-set AND its last set is separated
  from the mask use by a label (combine.c:6887-6888 label_tick mismatch +
  6920-6923 full-mask fallback with reg_nonzero_bits==0, i.e. the pseudo must
  ALSO fail the recording gate at 724-732 — e.g. single-BB-classified or
  live-at-start). NOTE the recording-gate subtlety: a recorded multi-set
  pseudo ORs to 0xFFFF and still folds, so the shape must make recording NOT
  happen, not just multi-set. Next probe: sweep shapes where `m` is assigned
  in both arms of the vel_y branch / loop-carried, checking .greg
  reg_basic_block and the emitted opcode. Long odds; mechanism-precise.
- F2 **Frame/address-taken sub-word read** — MEASURED s1: `u16 m = *(u16 *)&mode;`
  scores **25** (218/216 insns — mode homed to a stack slot, sw + lhu reload
  added). **KILLED**; banked in rejected/u16-mode-with-masked-or-copied-m.c
  (variant C note). Do not re-propose any address-taken spelling of mode.
- F3 If F1/F2 measure dead: the residual is the SAME endgame the owner ruled
  terminal for func_800871D4 (decisions.md:2128 names func_8002304C as a
  shape-sibling; Gate 1 canonical scan here is tier=LOW per s1 `canonical`
  output, Gate 2 census already came back negative there). Do NOT self-escalate
  outside `escalation` modality — bank measurements and let the driver ladder.
  A permuter session (R3 allows 2 max) over the mode/m block microspace is the
  natural s2/s3 modality: the search space is one expression, so a directed
  campaign is cheap and exhaustive-ish.

## [s1] Naming 0x1F8002B8 as a local hoists it to a prologue callee-save like target s7
- mechanism: loop-invariant constant gets one pseudo + global alloc instead of per-use lui/ori rematerialization
- probe: s32 lim = 0x1F8002B8; passed at both func_8005344C sites
- result: 29 -> 19
- verdict: CONFIRMED

## [s1] Callee-save seats follow declaration position (stable map) while prologue emission follows init-statement order; independently steerable
- mechanism: GCC 2.7.2 allocation order vs RTL emission order
- probe: decl order scratch,count,lim,scratch_d,scratch_c with lim/scratch_d initialized by later statements
- result: 19 -> 12 -> 6
- verdict: CONFIRMED

## [s1] step's t0-seat + late sw come from sched1 hoisting the named local's const birth above the 0x15 compare
- mechanism: hoisted birth widens range into v0/v1 compare temps (read in .lreg: insn 309 above 313/315; greg reg 158 -> hard reg 8)
- probe: inline ternary (mode==0x15)?0x80:0x100 as the call arg births the value in the bne delay slot
- result: 6 -> 1; v0 seat, sw-before-args, a3-in-delay all match
- verdict: CONFIRMED

## [s1] Wide->narrow typing (s32 mode from u16 field; u16 m = mode) reproduces target's raw-a0 + masked-v1 register split
- mechanism: distinct HImode pseudo keeps a separate copy insn; narrow-first spellings let combine collapse to one reg
- probe: 3 spellings measured
- result: wide->narrow = floor 1; both narrow-first = 3 (insn vanishes); address-taken sub-word read = 25 (spill)
- verdict: CONFIRMED

## [s1] A same-BB honest spelling can make our combine emit andi $v1,$a0,0xffff instead of a copy
- mechanism: combine.c:6885-6923 nonzero_bits REG current-value path proves <=0xFFFF from the lhu; full-mask escape needs multi-set + label-region mismatch + recording-gate failure (combine.c:724-732); two lhu sets OR to 0xFFFF anyway
- probe: compiler-source read + 4 measured spellings (retypes, explicit &0xFFFF, address-taken)
- result: all fold to addu copy or worse; no same-BB escape found
- verdict: KILLED

## s2 (2026-08-26, structural)

## [s2] The andi residual is NOT a nonzero_bits/label-region problem; the mask survives whenever combine cannot attempt the i2->i3 substitution
- mechanism: can_combine_p refuses to substitute the zero_extend load into the AND while the load's pseudo is live past the AND (`mode - 0x17` reads it), so nonzero_bits (combine.c:6885-6923) never runs on the AND and the standalone andsi3 reaches the assembler as `andi $v1,$a0,0xffff`
- probe: read tmp/grind/func_8002304C/dumps/code6cac.combine (dump.ps1); found the surviving `(and:SI (reg/v:SI 74) (const_int 65535))` in the same TU's func_80023E40 and traced why it survives there
- result: s1's H5 KILL was scoped to the wrong gate; the escape needs wide-mode operands + a live raw pseudo, not a label_tick mismatch
- verdict: CONFIRMED (supersedes H5's KILL, which stands only for the narrow-typed spellings it actually measured)

## [s2] The original author's spelling of this block is recoverable from the copy-paste sibling func_80023E40 in the same TU
- mechanism: source fidelity, not codegen coercion — func_80023E40 (COMPLETED-C, 0 rules, matched at 6d255e79) reads the identical field obj+0x6A with `s32 a0 = *(u16*)(arg0+0x6A); s32 v1 = a0 & 0xFFFF;` and runs the identical ==8 / ==0x22 / (u32)(a0-0x17)<2 / ==0xA cascade
- probe: apply that exact idiom (both locals s32, explicit & 0xFFFF) to func_8002304C's vel_y block; measure
- result: sandbox --disable all 1 -> **0**, 216/216 insns; full verify-oracle SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
- verdict: CONFIRMED — function MATCHED in pure C

## [s2] F1 (label-crossing multi-set shape) and F3 (endgame/canonical routing) are moot
- mechanism: the residual they were aimed at no longer exists
- probe: n/a — closed by the s2 match before either was spent
- result: F1 never needed; F3's func_800871D4 endgame equivalence is now known to be a FALSE sibling for this function (there the andi was missing entirely; here it was an unfoldable-mask spelling problem)
- verdict: KILLED (as live frontier items)

## Live frontier (post-match)

- none for this function; it is MATCHED. Carry-forward for OTHER functions:
  when a target shows a redundant `andi rX, rY, 0xffff` (or any redundant
  width mask) that our build renders as a copy or deletes, check whether the
  raw pseudo is LIVE past the mask; if so, spell BOTH the raw value and the
  masked value in the SAME wide mode with an explicit `& 0xFFFF`, and grep the
  same TU for an already-matched sibling that reads the same field — the
  original's idiom is often already in the tree. This is the direct
  counter-recipe to memory/reference/redundant-byte-andi-satan0main.md (which
  covers the NARROW direction for the byte case).
