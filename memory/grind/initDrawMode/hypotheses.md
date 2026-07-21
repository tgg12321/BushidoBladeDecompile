# Hypothesis ledger — initDrawMode

## Grind session 1 (2026-07-21, recon)

**H1 — two-arm val spelling breaks the val→$a3 tie. CONFIRMED, score 5 → 0.**
- Statement: target keeps `cmd|val` source order but allocates val=$v0 /
  cmd=$v1 (proven by the rejected `val|cmd` form which reproduced exactly that
  allocation and differed only in or rs/rt order). The wall is therefore the
  single-def `val = a3 & MASK;` tie: local-alloc combine_regs ties the andi
  dest into dying $a3. A two-arm if/else spelling
  (`if (a1) val = (a3&M)|T; else val = a3&M;`) makes the andi dest a shared
  subexpression temp / multi-def structure that combine_regs does not tie
  into $a3; the temp lands in $v0 (freed by the setlen `li v0,1` temp after
  `sb`), and the final IOR ties its result to val → `or v0,v1,v0`, target.
- Probe: first probe of the session (not permuter/enumeration-derived);
  sandbox --disable all = 0, target_insns=build_insns=11.
- Byte-neutrality: objdump of tmp/sandbox/initDrawMode/gpu.o shows ONE andi
  (0x918, delay slot) — the duplicated `a3 & MASK` does not materialize
  twice. Artifact: tmp/grind/initDrawMode/s1/sandbox0_objdump.txt.
- Family claim: duplicated-statement-into-arms (owner ruling 2026-07-01) —
  real statement per path, byte-neutral (one andi), exhaustion below,
  /* FAKE */ annotation was added.
- REVIEW STATUS: layer-1 cheat-reviewer FAILed TWICE (initial + prerequisite
  resubmission). Grounds: (a) the two arms compute DIFFERENT values, so the
  form is not an identical-statement duplication — the reviewer reads the
  sanctioned family as covering only identical-statement reg_n_refs count
  lifts (motion_SetMotion precedent), not multi-def structure that suppresses
  the combine_regs single-def tie; (b) the construct's provenance and stated
  justification are GCC-internals (tests 2/3/5). The reviewer conceded the
  exhaustion ledger is genuine, the annotation present, and byte-neutrality
  factually verified. Its next_action: surface as an owner policy question.
- DISPOSITION: RULING-GATED, not disproven. src/gpu.c reverted to the
  floor-5 natural compound form per the reviewer's next_action; the score-0
  form is preserved in candidate.c pending the owner's ruling. Do NOT
  re-measure this form and do NOT apply it to src/ until the owner rules.

**Lever-exhaustion record for the combine_regs a3-tie wall** (all measured,
sessions 1–2 + wip import; see evidence.md rejected_forms):
- natural compound form `val = a3&M; if (a1) val |= T;` → 5 (the wall itself)
- split-init `val = a3; val &= M;` → 5 (recombined)
- result-tmp `u32 result = cmd|val` → 5 (inlined)
- a3 in-place `a3 &= M; a3 |= …` → 6
- val-init-on-decl → 7
- val-computed-before-cmd → 7 (fold unaffected)
- u16-narrowed a3 param → 5 (fold unaffected)
- `val |= cmd` compound → 6; `cmd |= val` compound → 5
- hoist val above `a0[3]=1` → 6 (+1 insn)
- operand reorder `val|cmd` → 1 but REJECTED (or-tree-shape-shift cheat)
- forbidden axes not attempted: dead stores of a3 (target has no such bytes),
  volatile coercion, pins, __asm__, chain-extenders, DImode.
