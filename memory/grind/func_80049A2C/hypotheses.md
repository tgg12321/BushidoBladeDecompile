# Hypothesis ledger — func_80049A2C

## Frontier after s1 recon

### H1 (frontier) — replace `dummy[2]` with a live-then-DCE'd scalar (phantom-frame-slots pattern)
- **Mechanism:** per phantom-frame-slots-gcc272, GCC 2.7.2 reserves
  `get_frame_size()` bytes for locals it later register-allocates away — no
  store emitted. A live scalar assignment whose result the RA folds into a
  reg still bumps `vars` by 4 (aligned to 8). Delivers the +8 frame slot
  target has, without any dead-declaration/dead-array cheat construct.
- **Next probe:** try `int fp_slot = temp_v1 & 0xFF;` (or similar live use of an
  existing value) declared alongside the other locals; measure vars= in a cc1
  `-da` .frame dump and sandbox score. If frame goes to 0x30 with score 0,
  candidate-ready.

### H2 (frontier) — widen an existing narrow local to force a stack temp
- **Mechanism:** the current body has s16 locals (`a1_val`, `new_var2`) and
  various pointer aliases. Widening `new_var2` to s32 or introducing an s32
  local that captures an intermediate (`s32 tmp = (*p_anim) * 2;`) may push
  `get_frame_size()` above 0 for the same phantom-frame reason.
- **Next probe:** substitute each narrow local one at a time, measure sandbox
  + cc1 `.frame` vars=; correlate against the +8-frame target.

### H3 (frontier) — restructure the two obj-init blocks to share an intermediate
- **Mechanism:** the code initializes two consecutive 0x68-byte objects with a
  near-identical field pattern. A local capturing the shared computation
  (e.g. `s32 hdr = (a1_val) | ((*p_anim) * 2);` or a shared pointer) may
  land as a stack temp regardless of DCE outcome, replicating the frame
  reservation via ordinary source structure rather than a padding decl.
- **Next probe:** build a variant that hoists the `a1_val = (*p_anim) * 2`
  recomputation into a single local reused across both blocks; measure.

## Killed / eliminated (this session)
- `char new_var4;` — score contribution 0 → **just delete**.
- Empty `if (a1_val) { }` — score contribution 0 → **just delete**.
- `(new_var = new_var3)` inline-assign + `int new_var;` decl — score
  contribution 0 → **just delete** (write `obj + new_var3` directly).

## [s1] char new_var4 declaration is pure noise (no codegen effect)
- mechanism: Unused scalar; GCC DCEs the decl, no frame reservation, no insn emitted.
- probe: Delete `char new_var4;` + `(void) new_var4;` alone (keep dummy, empty-if, inline-assign). Run sandbox --disable all.
- result: score=0 (was 0). Zero delta.
- verdict: KILLED

## [s1] Empty `if (a1_val) { }` is pure noise (no codegen effect)
- mechanism: GCC evaluates a1_val (already live), empty body DCE'd, branch DCE'd.
- probe: Delete the empty-if line alone (keep dummy, new_var4, inline-assign). Run sandbox --disable all.
- result: score=0 (was 0). Zero delta.
- verdict: KILLED

## [s1] Inline-assign `(obj + (new_var = new_var3))` + `int new_var` decl is pure noise
- mechanism: new_var is a dead scalar; the assign folds; expression collapses to `obj + new_var3` which is what the plain-C spelling emits anyway.
- probe: Rewrite as `*((s16 *)(obj + new_var3)) = 0;` and drop `int new_var;` (keep dummy, new_var4, empty-if). Run sandbox --disable all.
- result: score=0 (was 0). Zero delta.
- verdict: KILLED

## [s1] s32 dummy[2] IS load-bearing; removing it alone jumps score to 12
- mechanism: Target frame = 0x30 = ALIGN8(vars) + 16 args + 24 gp-regs → vars = 1..8. dummy[2] provides 8 bytes of get_frame_size() → sp-relative offsets shift +8 across the whole body when removed.
- probe: Delete `s32 dummy[2];` + `(void) dummy;` alone (keep the other three). Run sandbox --disable all.
- result: score=12, build_insns=126, cheat_asm_stripped=396 (was 397). Frame collapse propagates as offset diffs across every sp-relative access.
- verdict: CONFIRMED

## [s1] Combined cleanup (delete all four constructs) yields the same score as removing dummy[2] alone (12), confirming dummy is the sole load-bearing item
- mechanism: Additive score contribution of the three noise constructs is 0; only dummy[2] moves the needle.
- probe: Apply all four deletions together. Run sandbox --disable all.
- result: score=12, build_insns=126.
- verdict: CONFIRMED

## [s2] Dead HImode-bitwise pair (s16 fp_a = *p_anim; s16 fp_b = (s16)temp_v1; if ((fp_a & ~fp_b) & 1) {(void)fp_a;}) inserted after the fade-check triggers the reload/alter_reg stale-ref phantom slot.
- mechanism: phantom-frame-slots-gcc272 s3 mechanism: combine eliminates the redundant HImode->SImode sign-ext on paradoxical subreg; flow's stale reg_n_refs makes alter_reg reserve a frame slot.
- probe: Applied over baseline (dummy[2] removed, sandbox=12). Two variants: (A) mixed lh + lbu-derived pair; (B) two independent lh loads via *p_anim and *((s16*)new_var8).
- result: Both A and B: score 12, build_insns 126 (unchanged from dummy-removed baseline). The if-block is fully DCE'd BEFORE combine, so no sign-ext to elide, no stale ref, no phantom.
- verdict: KILLED

## [s2] Widen a1_val from s16 to s32 forces a phantom-frame slot via changed RTL mode/register-class.
- mechanism: H2 (state.json frontier) — mode widening pushes a spill into locals via the same phantom mechanism.
- probe: s16 a1_val -> s32 a1_val; measure sandbox.
- result: Score 12, build_insns 126. No frame effect; the sh-truncating stores still allow scalar promotion.
- verdict: KILLED

## [s2] Widen new_var2 from s16 to s32 forces a phantom-frame slot.
- mechanism: H2 sibling probe.
- probe: s16 new_var2 -> s32 new_var2; measure sandbox.
- result: Score 13, build_insns 126. STRICTLY WORSE than baseline (new_var2 codegen diverges from target).
- verdict: KILLED

## [s2] Live HImode-bitwise embedded in a1_val's computation (constant-fold-neutral): a1_val = (fp_a*2) | ((fp_a & ~fp_b) & 0) survives dead-code elimination and triggers the phantom.
- mechanism: Same as H1 but with the bitwise expression flowing into a live variable used in a real store.
- probe: Inserted at the a1_val assignment site; measure sandbox.
- result: Score 12, build_insns 126. GCC's constant-fold eliminates the '& 0' clause at the tree level, before combine sees it. No phantom.
- verdict: KILLED

## [s2] H3 — share a1_val across the two obj-init blocks (compute once, drop the recomputation) forces the value into a callee-save/frame slot across the func_800417D0 call.
- mechanism: Extending a1_val's live range across a call demands a stack home or callee-save; per s1 frontier note about shared intermediate.
- probe: Removed the second `a1_val = (*p_anim) * 2;` recomputation; measure sandbox.
- result: Score 34, build_insns 123 (LOST 3 target insns). Target contains a genuine second `lh 0x0($s3); nop; sll $a1,1` sequence at 3A2CC-3A2D8 — the recomputation is real target bytes, not compiler-hallucinated. Cannot share.
- verdict: KILLED

## [s3] H4 — pinpoint via cc1 -da the exact reload/alter_reg trigger conditions in a MATCHING BUILD of func_80049A2C (running with dummy[2] in place); if any pseudo already sits in reg_n_refs>0 && reg_renumber<0 && no-hard-reg state cheat-freely, the phantom is already firing on live code and can be replicated in a form without dummy[2].
- mechanism: Directly instrument the reload pass on the working (dummy[2]-carrying) baseline via project-flag cc1 -da to dump greg/lreg/flow/combine. If a pseudo denied a hard reg surfaces, replicate its entry condition without dummy[2].
- probe: Wrote tmp/grind/func_80049A2C/s3/build_da.sh (mirrors project cc1 flags exactly: -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin) targeting src/text1b.c whole-TU with -da. Sliced func_80049A2C from text1b.i.greg / .lreg / .flow / .combine via slice.py. Ran the counterfactual by editing src to remove dummy[2] and re-dumping. Compared the two greg passes.
- result: Dummy-IN baseline: .frame vars=8, 7 pseudos (89 123 133 141 94 73 81) — ALL allocated hard regs (73→17, 81→19, 89→3, 94→3, 123/133/141→65); only spill is hard-reg $a3 for insn 135 (outgoing-arg passing, not a pseudo frame slot); no reload/alter_reg reservation emitted. Dummy-OUT counterfactual: .frame vars=0, IDENTICAL greg dispositions (same 7 pseudos, same hard-reg assignments, same conflicts, same $a3 spill for insn 135). Delta between the two greg passes: ZERO. The phantom-slot mechanism does not fire anywhere in this function's pipeline; the 8-byte slot is 100% attributable to dummy[2]'s aggregate declaration (mips.c compute_frame_size reserves 8 bytes for s32[2] unconditionally at declaration).
- verdict: KILLED
