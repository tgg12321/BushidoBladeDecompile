# Hypothesis ledger — func_8007C86C

## s1 (2026-08-10, RECON, git HEAD 2d6270da)

- **H1 — the sibling func_8007C7A0's floor-5 chassis transfers to this twin
  with only the constant swapped (0xE3000000 -> 0xE4000000).** Mechanism:
  both functions are verbatim-linked static routines (get_cs / get_ce) from
  the same Sony PsyQ 4.0 LIBGPU SYS module; the census proved bit-exact
  provenance, so the original C differs only in the packet command constant.
  Probe: applied the sibling's candidate.c with the constant swapped to
  src/display.c, ran `sandbox func_8007C86C --disable all`. Result: **5 @ 50
  build vs 51 target** (from HEAD's 20 @ 51; old ledger floor was 12).
  Exact lock-step with the sibling's numbers (5 @ 50, 21 rules dropped).
  **VERDICT: CONFIRMED.** Floor 12 -> 5.

- **H2 — the residual at floor 5 is the identical banned join-temp-writeback
  family, not a twin-specific gap.** Mechanism: if the twins are lock-step,
  the missing insn must be the same X-clamp three-arm $v0 join + `move a3,v0`
  writeback the sibling's ledger proved requires a temp-routed-back-into-a-
  live-variable dataflow (banned by layer-1 under any spelling). Probe:
  disassembled the sandbox object (s1/ours_5form_50.txt) and compared against
  target (s1/target_51.s) instruction by instruction. Result: Y-clamp,
  dispatch (incl. the wide-mask `andi $v1,$a1,0xFFF` landing in the dispatch
  branch delay slot via cross-jump), and shared OR tail match 1:1; the ONLY
  divergence is the X-clamp join region — target routes all three arms
  through $v0 then copies `move $a3,$v0`; ours writes $a3 directly in each
  arm (one insn fewer, branch-sense + delay-slot cascade = the 5 masked
  diffs). **VERDICT: CONFIRMED.** The twin inherits the sibling's residual
  verbatim; no twin-specific axis exists at floor 5.

## [s1] The sibling func_8007C7A0's floor-5 chassis transfers to this twin with only the GPU command constant swapped (0xE3000000 -> 0xE4000000)
- mechanism: Both are verbatim-linked static routines (get_cs / get_ce) from the same Sony PsyQ 4.0 LIBGPU SYS module (census 2026-07-09); the original C can differ only in the packet constant
- probe: Applied the sibling's candidate.c with the constant swapped to src/display.c; sandbox func_8007C86C --disable all at HEAD 2d6270da
- result: 5 @ 50 build insns vs target 51 (HEAD baseline 20 @ 51; old ledger floor 12) — exact lock-step with the sibling's 5 @ 50
- verdict: CONFIRMED

## [s1] The floor-5 residual is the identical X-clamp three-arm join-temp + writeback family the sibling's layer-1 rulings ban, not a twin-specific gap
- mechanism: If the twins are lock-step, the missing insn must be target's routing of all three X-clamp arms through $v0 followed by `move a3,v0` — which the sibling's ledger (s12 P2 theorem, 19 structural kills, ~118k permuter iters) proved requires the banned temp-routed-back-into-live-variable dataflow under any spelling
- probe: Disassembled the sandbox object and compared instruction-by-instruction against asm/funcs/func_8007C86C.s (artifacts in tmp/grind/func_8007C86C/s1/)
- result: 47/51 target insns match 1:1 (Y clamp, dispatch incl. delay-slot wide mask, cross-jumped OR tail); the only divergence is the X-join region — target has the $v0 join + move a3,v0, ours writes $a3 directly per arm
- verdict: CONFIRMED
