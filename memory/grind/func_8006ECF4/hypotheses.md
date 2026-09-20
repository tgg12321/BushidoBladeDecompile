# Hypothesis ledger — func_8006ECF4

## Frontier (session 1, recon — nothing measured yet, no C body exists)

### F1 — Declare the array/table globals correctly BEFORE drafting any body (mandatory first move)
- Statement: `D_8009BC7C`, `D_800A3588`, `D_800A358C`, `D_800A3561` must be declared as arrays/tables (not the scalar/sub-symbol shapes currently in the header/census), sized from the union of every consumer's index bound, before writing func_8006ECF4's C body — writing the body against the wrong object model guarantees a rewrite.
- Mechanism: ordinary C declaration correctness (data model, not codegen) — ties to `no-new-park-categories.md`'s "Per-word splat symbol -> aggregate merge" family for the ones with existing SPLIT-AGGREGATE signals, but this function's asm evidence suggests the "splat scalar" framing itself may be wrong for D_800A3588/D_800A358C/D_800A3561 (see evidence.md OBJECT MODEL section) — needs cross-sibling stride confirmation, not just this function's evidence alone.
- Probe (next session): pull the exact index bounds/strides from func_8006F100, func_80070188, func_80070F78, func_8006F97C, func_8006E534 (every named xref) for each of these four symbols; if all consumers agree on stride and max index, that's prong (a) base-register/stride evidence for an aggregate-merge integration handoff; if they disagree, the symbols may need SEPARATE array declarations rather than a merge.
- Result: not yet probed.

### F2 — Draft the real C body (structural decomp) once F1's declarations are settled
- Statement: the function is a bounded loop (`for (s1 = 0; s1 < D_800A3554 + D_800A35B0 + 1; s1++)` in asm terms) with a nested lookup chain (`D_8009BC7C[D_8009BC40[(D_800A3588[i]+D_800A358C[i])*4]]` and `D_8009BC7C[D_800A3561[i*3]]`), a 6-way switch (`jtbl_800159D0`) selecting one of 5 struct-array slots (`D_800A35A8[k].field` paired with `s0[k]` at stride 0xC) or a default path, an 8-byte struct copy from `D_800A32F4` into `LoadImage`'s argument, and a call to `func_80073728` doing a read-modify-write on `arg0->0x4`. A single-argument `void func_8006ECF4(SomeStruct *arg0)` signature (NOT the 4-arg placeholder in pre-include-asm-body.c).
- Mechanism: ordinary structural C translation of the traced control flow (no lever needed at this stage — pure recovery of source shape).
- Probe (next session): write the body, `sandbox --disable all --diff`, class every hunk (source-level vs operand-only vs not-scored) before proposing any register/scheduling lever.
- Result: not yet probed (no C exists).

### F3 — jtbl_800159D0 cross-TU rodata ownership (anticipated, not yet reached)
- Statement: once F2 produces a real `switch` over the 6-case dispatch, GCC will re-synthesize a jump table that must resolve against the SAME rodata run (0x80015940-0x80015A3C) func_8006B578's ledger (sessions 7-9) already diagnosed and filed an integration-handoff recipe for (bb2.ld move + TU split around text1a_b_pre_rodata.c). Expect the same score-pinned-near-zero-by-relocation-class residual, NOT a source or register-allocation defect.
- Mechanism: `engine/score.py` masks branch/jump targets but section-relative vs external-symbol relocs against jump-table words are NOT masked identically (same class func_8006B578 hit) — see func_8006B578/candidate.c header comment for the full mechanism writeup.
- Probe (future session, once F2's body exists and scores near 0 with exactly this residual pattern): re-run func_8006B578 s9's byte-certification method (objdump -r + -h on both objects) rather than re-deriving it from scratch.
- Result: not yet probed (function has no C body).

## Judge constraints
(none — nothing has been submitted)

## Rejected forms
(none)
