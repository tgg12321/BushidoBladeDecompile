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
