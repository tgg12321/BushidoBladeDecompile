# `_SsVmFlush` grind ledger

## Identity and route

- Queue top on 2026-09-22: `_SsVmFlush`, `src/main.c`, initial honest distance 271
  because the function was a whole-body `INCLUDE_ASM`.
- `canonical _SsVmFlush`: verdict `C`, hand-coded tier `LOW`; ordinary pure-C target.
- Functional identity: Sony PsyQ LIBSND `VM_F`, corroborated by the call graph, symbol
  layout, target assembly, and the PsyQ 4.0 reference body in Xeeynamo/psyz
  `decomp/src/libsnd/vm_f.c`.

## Candidate history

- `candidate.c` v1: direct semantic transcription of the Sony reference adapted to
  BB2/PsyQ 3.5's API-based flush path (`SpuGetVoiceEnvelope`, `SpuSetVoiceAttr`,
  `SpuSetKey`, and `SpuSetReverbVoice`). Uses the documented `SpuVoiceAttr` layout and
  BB2's established 54-byte voice-record stride. No match-motivated constructs.

## Measurements

- v1: score 13, target 271 instructions, build 267. The four missing
  instructions came from GCC collapsing the envelope call pointer and subsequent load
  onto one induction pointer; target uses the real 54-byte `SpuVoice` record object.
- v2: replace field-symbol arithmetic with the documented Sony `SpuVoice` structure and
  correct the start-address shadow load to unsigned, as shown by target `lhu`. The
  aggregate experiment did not compile in the current TU because `main.c` still has the
  historical per-field scalar declarations; rejected without changing shared object-model
  declarations during the grind.
- v3: retain the established field declarations, but traverse the envelope output with
  the real advancing pointer passed to LIBSPU while retaining the byte offset needed by
  the voice record accesses. This emitted 273 instructions at score 23: explicit pointer
  initialization moved ahead of the guarded loop and changed the loop rotation. Rejected.
- v4: use the documented `SpuVoice` record through a typed view of the TU's historical
  scalar base declaration. This preserves one semantic record access at the use site while
  avoiding a shared declaration refactor until the candidate is proven. Initial definition
  copied the 4.0 record's 52-byte size and measured score 12 / 270 instructions; BB2's target
  and existing consumers establish a 54-byte record, so v5 adds the real trailing halfword
  and retains the direct field load spelling used throughout this TU.
- v5: score 2, 271/271 instructions. The sole scored difference was the ordering of
  the two real per-iteration `SpuVoiceAttr` initializations (`mask` and `voice`).
- v6: spell the initializations in target/source order: clear `mask`, then select `voice`.
- v6 measured score 0 / 271 instructions. Before integration, reject its provisional
  cross-symbol typed view even though it matches: retaining both the record base and field
  symbol would violate the one-object/one-handle rule.
- v7: express both envelope accesses from the sole established field symbol
  `D_800F4E1E`, with the loop's real byte offset named for the reload. This tests whether
  one truthful object handle retains the target's two consumed induction values.
- v7 measured score 6 / 272 instructions because assigning the named offset before the
  call displaced the real second argument from the call delay slot.
- v8: assign the reload offset after the call, where it is first consumed.
- v8 measured score 8 / 271 instructions; the post-call assignment inverted the two
  induction roles and moved the envelope argument out of the delay slot. Rejected.
- v9: use equivalent, natural typed arithmetic (`u16` record stride 27 for the output
  pointer; byte stride 54 for the reload) from the single field symbol. No extra local or
  object handle is introduced.
- v9 measured score 10 / 267 instructions; GCC recognized and merged the equivalent
  pointers despite the scaling difference.
- v10: derive both accesses from the established voice-record base `D_800F4E18`, using
  its documented halfword stride (27) and `envx` offset (3 halfwords). This removes the
  cross-symbol issue while presenting the two accesses in their natural typed and byte
  forms.
- v10 measured score 10 / 267 instructions; GCC again merged the two accesses.
- v11: return to the single field handle and test the one generally sanctioned
  single-level `do { } while (0)` wrapper around the real LIBSPU call. The wrapper is
  explicitly annotated and will be kept only if measurement proves it closes the loop.
- v11 measured score 50 / 267 instructions and caused a broad register rotation; rejected.
- v12: one-handle typed record access for both call and reload. This is the simplest
  semantically correct form and the calibration baseline; expected score 8 / 270.
- v12 measured score 8 / 270. Original `cc1psx` measured score 34 / 270, so the
  residual is source-side, not compiler fidelity.
- v13: sanctioned C-level typed pointer alias to the established record base; the alias
  is real and consumed by the call while the canonical global handle performs the
  post-call reload. Mechanism under test: address-materialization caching/CSE separation.
- v13 measured score 7 / 269 instructions. The alias changed base-register allocation but
  GCC still reused the record base for the post-call load, leaving the same missing
  rematerialization. Rejected.

## Rotation disposition

- Best honest form: v12, score 8 / 270, single aggregate handle.
- Original-compiler calibration: ours 8 / 270; cc1psx 34 / 270; source-side.
- A byte-matching v6 form existed, but fresh adversarial review independently rejected it
  because it used `D_800F4E18` and `D_800F4E1E` as two handles for one voice record.
- Standard direct arithmetic, typed scaling, statement-order, single-level wrapper,
  truthful typed record, and sanctioned pointer-alias variants were measured. No honest
  score-0 spelling was found. Rotate under the 2026-09-08 rule; retain this ledger and
  candidate for the next source-shape pass.

## Adversarial review checkpoint

- Fresh reviewer verdict on the v6 score-0 form: **FAIL**. The typed access through
  `D_800F4E18` and reload through `D_800F4E1E` name the same storage twice, exactly the
  forbidden one-object/two-handles family. The reviewer independently reproduced an
  honest one-handle floor of score 8 before its run allowance expired.

## Construct audit

- v1 constructs: none. All locals represent program values or the real PsyQ
  `SpuVoiceAttr` object passed to `SpuSetVoiceAttr`.
