# Hypothesis ledger — func_800480C0

## s1 — recon (2026-09-02)

- **H1 CONFIRMED — sibling transplant.** Porting the COMPLETED-C `func_80047FBC` body
  (two constructs: `volatile u32 pre_pad[8]` phantom-frame pad + `arg0 = 0;` dead store)
  extended to 6 args reproduces the target byte-for-byte: full build SHA1 == oracle.
  Measured: sandbox 20 (pad stripped) / build MATCH (pad honoured). Artifacts:
  `tmp/grind/func_800480C0/s1/{ladder.txt,build.log,cand1_build.txt}`.
- **H2 CONFIRMED — the 20-insn honest residual is 100% frame-offset.** Diff of sandbox
  build vs target: every mismatching insn is an sp-relative offset differing by 0x20; the
  instruction order and all non-sp operands match. No RA / scheduler / cse residual
  exists once the frame is 0x58.
- **H3 KILLED (instance) — removing the `arg0 = 0` dead store measures 32 on this chassis.**
  cse2 canonical-reg substitution rebinds the base copies and cascades. measured_on:
  HEAD dd2808d5, pre_pad present-but-stripped, single arm. The store is load-bearing.
- **Not attempted (deliberately):** honest frame producers for the 32-byte region. The
  region has zero stores in the target, so the WRITTEN-array carve-out is inapplicable
  by rule text (`.claude/rules/dead-vars-local-array.md:5`), and the identical-layout
  siblings spent 12+ sessions / ~26.5k permuter iters measuring honest producers inert
  (`docs/grind/decisions.md:7401-7660`, `src/text1b.c:20-36` header). Re-activation
  trigger: a frame-forensics finding that the original declared a LIVE 8-word local here.

## Frontier (for the operator / next session)
1. INTEGRATION HANDOFF filed (docs/grind/decisions.md 2026-09-02 entry): add
   `"func_800480C0": frozenset({("pre_pad", 8)})` to `_SANCTIONED_UNWRITTEN_PADS`, re-run
   sandbox (expect 0), verify-oracle, layer-2 cheat-reviewer, `queue done`.
2. Nothing else remains: the instruction stream is already byte-identical.

## [s1] Transplanting the COMPLETED-C func_80047FBC body (volatile pre_pad[8] + arg0=0 dead store) extended to 6 args reproduces func_800480C0 byte-for-byte
- mechanism: identical prologue/loop idiom; phantom 32-byte vars region reserved by function.c assign_stack_local; cse2 canonical-reg fold defeated by the param dead store
- probe: apply body to src/text1b.c; canonical; sandbox --disable all; full build
- result: canonical C/20; sandbox 20 (74/74, all 20 residual = sp-offset deltas, pad stripped); build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH
- verdict: CONFIRMED

## [s1] With the pad honoured the honest residual is purely frame-offset: no RA/scheduler/cse residual remains
- mechanism: frame 0x38 vs 0x58 shifts every sp-relative operand by 0x20
- probe: objdump diff of sandbox build vs target
- result: 20 mismatching insns, each an sp-offset delta; instruction order and all other operands identical
- verdict: CONFIRMED

## [s1] Removing the arg0 = 0 dead store (pad still stripped) measures 32 on this chassis, so the store is load-bearing for this body
- mechanism: cse2 canonical-register substitution over {arg0,p,base_addr} rebinds addu $s2 / addu $s0 to $a0 and cascades
- probe: delete the store; sandbox --disable all
- result: 32 vs 20 with the store
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD dd2808d5, candidate body with volatile pre_pad[8] present but detector-stripped, arg0=0 removed
