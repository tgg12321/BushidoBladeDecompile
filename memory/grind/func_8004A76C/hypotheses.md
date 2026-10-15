# Hypothesis ledger — func_8004A76C

## s1 (recon)

### H1 — a GCC file-scope global-register-variable (`register T x asm("$16")`) could
reproduce the unsaved-`$s0` S7 shape from ordinary C, making this a decompilable (not
whole-body-asm) function.

- mechanism: `register` storage class + `asm("$N")` binds a C variable to a fixed GPR for
  the whole TU, which would make a "callee-saved register used without save" shape
  reproducible from C instead of requiring hand asm.
- probe: this exact question was raised and closed by `docs/audits/INLINE-ASM-AUDIT-2026-10-01.md`
  item A6 (read in full this session). Re-confirmed this session: (1) the sole caller
  `func_8004A4E0` is independently whole-body hand asm (ABI evidence in
  `memory/grind/func_8004A4E0/evidence.md` s1/s18, hands `$s0` live across the `jal` with no
  GCC-emittable mechanism); (2) the loop body is hand-software-pipelined (`rtpt` for
  iteration N+1 issued before iteration N's stores retire, entry `j` jumps into the unrolled
  loop's second half to prime the pipeline) — not a shape any GCC 2.7.2 scheduler emits from
  a straight-line C loop; (3) a global register variable is itself a register pin, banned by
  `.claude/rules/completion-bar.md` item 2 (the wall) regardless of whether it reproduces
  bytes.
- result: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A76C.s (39 insns), no FAKE carrier (no C body exists to
  carry one)
- predicate_cite: docs/audits/INLINE-ASM-AUDIT-2026-10-01.md:93 (A6 row, "CLOSED, no
  change" — caller is whole-body asm, loop is hand-pipelined, global-register-var is itself
  a banned pin)

## Frontier (next session / next modality)

1. hypothesis: the ASM-WHOLE canonical grant is correct and the 2026-10-08 reopen's question
   ("decide afresh: hand-written asm vs compiled C") is already answered for this function.
   mechanism: `scan_hand_coded --single` tier=STRONG 1/8 (S7) directly on func_8004A76C —
   unlike sibling func_8004A4E0 (LOW 0/8, Judge-FAILed on exactly the STRONG-tier bar at
   docs/grind/decisions.md:1320), this function clears the scanner-STRONG requirement on its
   own evidence, plus the closed A6 audit naming it specifically.
   next_probe: in `escalation` modality (not before — mandated-modality rule forbids
   disposing here), file the owner-gated/decisions.md entry citing A6 + the STRONG scan
   tier as the gate evidence, and the re-activation triggers (an engine signal change, or
   sibling func_8004A4E0/func_8004A808/func_8004A940 movement).
2. hypothesis: func_8004A808 and func_8004A940 (same TU, same ABI cluster, also reopened)
   will scan the same way — worth checking their scan tier before any of the three is
   escalated, since a consistent STRONG result across the cluster strengthens the record.
   mechanism: shared callee-saved-register ABI cluster per func_8004A4E0's audit.
   next_probe: `scan_hand_coded.py --single func_8004A808` / `func_8004A940` (quick, no build).

## [s1] A GCC file-scope global-register-variable (register T x asm("$16")) could reproduce the unsaved-$s0 S7 shape from ordinary C, making this function decompilable rather than whole-body hand asm.
- mechanism: register storage class + asm("$N") binds a C variable to a fixed GPR for the whole TU, which in principle could make a 'callee-saved register used without save' shape emittable from C.
- probe: Re-read docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6 (which raised this exact question) plus func_8004A4E0's own canonical audit (caller ABI evidence) and the raw asm/funcs/func_8004A76C.s body (software-pipelining shape).
- result: A6 already closed this with three independent reasons: (1) the sole caller func_8004A4E0 is itself whole-body hand asm handing $s0 live across the jal in a way GCC cannot emit; (2) the loop is hand software-pipelined (rtpt for iteration N+1 issued before iteration N's stores retire, entry j jumps into the unrolled loop's second half) — not a GCC-schedulable shape; (3) a global register variable is itself a register pin, independently banned by the wall regardless of byte reproduction.
- verdict: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A76C.s (39 insns); no C body exists so no FAKE carrier is present
- predicate_cite: docs/audits/INLINE-ASM-AUDIT-2026-10-01.md:93

## s2 (structural)

### H2 — structural-modality levers (block-local var splits, declaration order, type
narrowing, statement re-association) require an existing C body with a sandbox-measured
non-zero distance to operate on; this function has neither.

- mechanism: every structural technique transforms source text to change codegen under a
  fixed semantic; with no C body (`src/main/3AB48.c:30` is a bare `INCLUDE_ASM` stub) there
  is no source to transform and no `--diff` hunk classification (source-level / operand-only
  / not-scored) to target. This is not "structural work tried and ineffective" — it is
  structural work with no object to apply it to, same as s1 already found for recon.
- probe: re-ran `canonical func_8004A76C` (still ASM-WHOLE, same reason) and `sandbox
  func_8004A76C --disable all` (still `{"score":37,"no_c_body":true,"build_insns":0}`,
  `--diff` still "no diff: function is still whole-body INCLUDE_ASM") to confirm nothing
  changed since s1 before concluding the modality is inapplicable here.
- result: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c (INCLUDE_ASM stub, unchanged), no FAKE carrier (no C
  body exists)

### H3 — the func_8004A4E0 custom-ABI cluster (func_8004A76C/func_8004A808/func_8004A940)
will scan STRONG uniformly, strengthening the cluster-wide escalation record ahead of any one
member's escalation session (s1 frontier item 2).

- mechanism: shared callee-saved-register ABI violation across the cluster, per
  func_8004A4E0's own canonical audit — each callee leaks unsaved `$sN` state back across its
  `jal` boundary to the caller.
- probe: `scan_hand_coded.py --single func_8004A808` and `--single func_8004A940` (no build
  needed).
- result: CONFIRMED. func_8004A808: tier=STRONG 3/8 (S3+S4+S7, unsaved `$s0`). func_8004A940:
  tier=STRONG 3/8 (S3+S4+S7, unsaved `$s0`-`$s7`, all eight). All three callees now carry an
  independently-measured STRONG verdict; only the caller func_8004A4E0 scans LOW, which is the
  asymmetry already on record from its own Judge FAIL.

## Frontier (next session / next modality) — unchanged from s1, now with cluster scan done

1. hypothesis: the ASM-WHOLE canonical grant is correct and the reopen's question is already
   answered for this function specifically.
   mechanism: scan_hand_coded STRONG 1/8 (S7) measured directly on func_8004A76C, plus the
   closed A6 audit, plus (new this session) both sibling callees independently confirming
   STRONG on the same S7 signal.
   next_probe: in `escalation` modality (not before), file the docs/grind/decisions.md
   disposition entry citing A6 + the cluster-wide STRONG scan results, then return
   owner-gated.
2. (s1 frontier item 2 is now closed — see H3 above. No remaining frontier item from it.)

## [s2] Structural-modality levers (block-local var splits, declaration order, type narrowing, statement re-association) require an existing C body with a sandbox-measured non-zero distance to operate on; this function has neither.
- mechanism: src/main/3AB48.c:30 is a bare INCLUDE_ASM stub (canonical gate = ASM-WHOLE); no C source exists to transform and no --diff hunk classification (source-level/operand-only/not-scored) exists to target.
- probe: Re-ran `canonical func_8004A76C` and `sandbox func_8004A76C --disable all [--diff]` to confirm nothing changed since s1 before concluding the modality is inapplicable.
- result: canonical still ASM-WHOLE ('whole-function canonical-asm grant in inline_asm_canonical.txt'); sandbox still {"score":37,"no_c_body":true,"build_insns":0}; --diff still reports no diff exists.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c (INCLUDE_ASM stub, unchanged this session), no FAKE carrier (no C body exists)

## [s2] func_8004A808 and func_8004A940 (same jal-cluster, same custom callee-saved-register ABI reopened alongside func_8004A76C) will scan STRONG on scan_hand_coded the same way, strengthening a cluster-wide record ahead of any one member's escalation.
- mechanism: Shared callee-saved-register ABI violation across the func_8004A4E0/func_8004A76C/func_8004A808/func_8004A940 cluster: each callee leaves $sN registers unsaved and relies on the caller (itself whole-body hand asm) to treat them as live across the jal, a shape no GCC-emitted callee can produce.
- probe: scan_hand_coded.py --single func_8004A808 and --single func_8004A940 (no build needed).
- result: func_8004A808: tier=STRONG score=3/8 (S3 no-spills, S4 front-loads, S7 unsaved $s0 -- 'GCC-impossible, decisive'). func_8004A940: tier=STRONG score=3/8 (same S3/S4/S7 triplet, S7 firing on all eight callee-saves $s0-$s7, the maximal shape). Both independently confirm the S7 signal already measured on func_8004A76C itself; only the cluster's caller func_8004A4E0 scans LOW (0/8), the asymmetry already on record from its own s18/s18b Judge FAIL.
- verdict: CONFIRMED

## s3 (permuter)

### H4 — the permuter requires an existing C candidate (base + variations) to mutate against
the target asm; with no C body for func_8004A76C (ASM-WHOLE canonical, s1/s2 already
established), there is nothing for `tools/permuter_annotate.py` / `permuter_campaign.py` to
operate on, same inapplicability s2 found for structural modality.

- mechanism: the permuter's mutation loop needs a compiling C source file as its base
  candidate to perturb; `memory/grind/func_8004A76C/candidate.c` is a 15-line comment-only
  stub documenting why no C body exists, not compilable C, so there is no base for a
  campaign to seed from, and `--diff` has no hunks to direct a `--hint` at.
- probe: re-ran `canonical func_8004A76C` (still ASM-WHOLE, same `inline_asm_canonical.txt`
  grant) and `sandbox func_8004A76C --disable all` (still `{"score":37,"no_c_body":true,
  "build_insns":0}`) this session before concluding the permuter has no object either.
- result: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c (INCLUDE_ASM stub, unchanged this session) and
  memory/grind/func_8004A76C/candidate.c (comment-only stub, unchanged), no FAKE carrier (no
  C body exists)

## Frontier (unchanged from s2 — permuter modality adds no new angle)

1. hypothesis: the ASM-WHOLE canonical grant is correct and the reopen's question is already
   answered for this function specifically.
   mechanism: scan_hand_coded STRONG 1/8 (S7) measured directly on func_8004A76C, the closed
   A6 audit, and both sibling callees (func_8004A808, func_8004A940) independently confirming
   STRONG on the same S7 signal (s2/H3).
   next_probe: in `escalation` modality (not before), file the docs/grind/decisions.md
   disposition entry citing A6 + the cluster-wide STRONG scan results, then return
   owner-gated.

## s3 (permuter)

### H2 — a decomp-permuter campaign can be constructed for func_8004A76C to search for a
C-level match of the diverging region.

- mechanism: `tools/decomp-permuter/import.py` builds a permuter workspace from a C file by
  locating either (a) a `GLOBAL_ASM`-embedded function body in the given `c_file`, or (b) a
  standalone function definition compiled by `make`. Both require an actual C function body
  for `func_8004A76C` to exist somewhere in the tree.
- probe: ran `sandbox func_8004A76C --disable all --diff` this session
  (`tmp/grind/func_8004A76C/s3/sandbox_diff.txt`) — confirms `"no_c_body": true`, `build_insns: 0`,
  and "no diff: function is still whole-body INCLUDE_ASM — there is no C to compare" (floor 37,
  unchanged). Then ran `tools/decomp-permuter/import.py src/main/3AB48.c func_8004A76C` directly
  (WSL venv, `tmp/grind/func_8004A76C/s3/import_attempt.txt`): it fails immediately with
  "Failed to find GLOBAL_ASM for function func_8004A76C" — `src/main/3AB48.c:30` is a bare
  `INCLUDE_ASM("asm/funcs", func_8004A76C);` statement with no C body and no GLOBAL_ASM wrapper
  for this function, so there is nothing for `import.py` to extract a base.c from.
- result: a permuter workspace cannot be constructed for this function in its current state —
  not a judgment call, a mechanical tool failure captured verbatim. This is independent
  confirmation (by direct tool attempt, not just ledger citation) of the same fact s1/s2
  already established from the canonical-gate and GTE-asm angle: this chassis has zero bytes of
  C to permute.
- verdict: KILLED
  - kill_scope: instance
  - measured_on: current `src/main/3AB48.c` INCLUDE_ASM stub for func_8004A76C (unchanged this
    session), no C body exists, no FAKE carrier (no C exists to carry one)

## [s3] A decomp-permuter campaign can be constructed for func_8004A76C to search for a C-level match of the diverging region.
- mechanism: tools/decomp-permuter/import.py requires either a GLOBAL_ASM-embedded C function body or a standalone C function definition for the target symbol in the given c_file to build a permuter workspace (base.c/compile.sh/target.o).
- probe: Ran `sandbox func_8004A76C --disable all --diff` (tmp/grind/func_8004A76C/s3/sandbox_diff.txt): no_c_body=true, build_insns=0, 'no diff: function is still whole-body INCLUDE_ASM'. Then ran `python3 tools/decomp-permuter/import.py src/main/3AB48.c func_8004A76C` directly in the WSL venv (tmp/grind/func_8004A76C/s3/import_attempt.txt).
- result: import.py fails immediately: 'Failed to find GLOBAL_ASM for function func_8004A76C'. src/main/3AB48.c:30 is a bare INCLUDE_ASM("asm/funcs", func_8004A76C); statement with no C body and no GLOBAL_ASM wrapper, so there is no base.c the permuter tooling can extract. This is a direct mechanical tool-failure confirmation, not a re-derivation of the ledger's prior canonical-gate reasoning.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub for func_8004A76C (unchanged this session); no C body exists; no FAKE carrier possible (no C to carry one)

## [s4] enumerate — a systematic spelling sweep (`spelling_enum.py` / `sweep_variants.py`) over an `/* ENUM-BEGIN */.../* ENUM-END */` region can search the diverging block's C spelling space for func_8004A76C.
- mechanism: `spelling_enum.py` requires an existing C candidate body with the differing block written out in fully-named statement-per-line form between ENUM-BEGIN/ENUM-END markers; it mutates/permutes THAT region's statements and operand order.
- probe: re-confirmed this session (not re-derived from ledger text alone): `canonical func_8004A76C` → `{"verdict": "ASM-WHOLE", ...}` (whole-function canonical-asm grant in inline_asm_canonical.txt); `sandbox func_8004A76C --disable all` → `{"score": 37, "no_c_body": true, "build_insns": 0}`, scoring the bare `INCLUDE_ASM("asm/funcs", func_8004A76C);` stub at src/main/3AB48.c:30. There is no C statement anywhere in this function to wrap in ENUM-BEGIN/END markers — the entire 37-insn target is GTE (cop2) + hand-software-pipelined asm (s1 H1, class kill) with zero C bytes. `spelling_enum.py` has no region to operate on; running it would require first inventing a synthetic C body, which is not a spelling of this function's actual code and would misrepresent what the function does (completion-bar item 3, honest code).
- result: enumerate modality is mechanically inapplicable to this function for the same reason structural (s2) and permuter (s3) modalities were — there is no C body. This is the third independent modality to hit the identical gate (canonical ASM-WHOLE grant, no_c_body=true), reinforcing s1's class kill rather than adding a new one.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub for func_8004A76C (unchanged this session); no C body exists; no FAKE carrier possible (no C to carry one)

## [s4] A systematic spelling sweep (spelling_enum.py / sweep_variants.py over an /* ENUM-BEGIN */.../* ENUM-END */ region) can search func_8004A76C's diverging C spelling space for an exact-match form.
- mechanism: spelling_enum.py requires an existing C candidate body with the differing block written out in fully-named statement-per-line form between ENUM-BEGIN/ENUM-END markers; it permutes that region's statement order and operands.
- probe: Re-measured this session: `canonical func_8004A76C` -> verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt'. `sandbox func_8004A76C --disable all` -> {"score": 37, "no_c_body": true, "build_insns": 0}, scoring the bare INCLUDE_ASM("asm/funcs", func_8004A76C); stub at src/main/3AB48.c:30. There is no C statement anywhere in this function to mark with ENUM-BEGIN/END; the entire 37-insn target is GTE cop2 + hand software-pipelined asm (s1 H1 class kill, docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6). Inventing a synthetic C body to feed the enumerator would misrepresent the function under completion-bar item 3 (honest code).
- result: enumerate modality is mechanically inapplicable to this function for the identical reason structural (s2) and permuter (s3) modalities were inapplicable: there is no C body to operate on. Fourth independent modality (recon/structural/permuter/enumerate) to hit the same ASM-WHOLE + no_c_body=true gate.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub for func_8004A76C (unchanged this session); no C body exists; no FAKE carrier possible (no C to carry one)

## s5 (synthesis)

## [s5] KILL RE-AUDIT — the s2/s3/s4 instance kills could be void because their chassis or FAKE state changed.
- mechanism: an instance kill measured under a FAKE carrier or older chassis is re-testable.
- probe: compared each kill's measured_on with the current src/main/3AB48.c:30 (still the bare INCLUDE_ASM stub); checked for any FAKE construct in any banked form (none: no C form was ever banked before s5), so tools/fake_ablate.py has no FAKE to ablate.
- result: all three kills were measured on the identical stub with no FAKE; none is void. The weakest foreclosure is A6 reason (b) (pipelining), re-audited below.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub, no FAKE construct present in any banked form

## [s5] A natural honest GTE loop (gte.h macros, vertex pointer as 4th arg) approaches the target.
- mechanism: GTE ops as canonical gte.h islands, GCC emits the GPR skeleton.
- probe: sandbox --disable all --diff --candidate tmp/grind/func_8004A76C/s5/honest_gte_loop.c
- result: 33, 3 source-level hunks, 0 operand-only; per-vector addiu bases, unpipelined, $a3 base; rtpt stripped by the scorer. Wrong ABI ($a3 vs unsaved $s0). Banked rejected/honest-loop-wrong-abi-a3.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate file tmp/grind/func_8004A76C/s5/honest_gte_loop.c over the 3AB48 stub chassis, no FAKE construct

## [s5] A6 reason (b) — the hand software-pipelining (jump into loop second half, rtpt N+1 before N's stores) has no C form.
- mechanism: goto-into-middle C reproduces the rotated CFG; GCC keeps label order.
- probe: sandbox --disable all --diff --candidate tmp/grind/func_8004A76C/s5/pipelined_goto.c
- result: 30; GPR skeleton (sh -6/-4/-2(a2), mfc2 $17/$18/$19, branch-into-middle j, tail sh 0/2/4) aligns with the target; residual is only stripped GTE statements + $s0 base. Reason (b) is REFUTED as a standalone blocker — pipelining is expressible in C. Grant still stands on (a) caller ABI + (c) pin. Banked rejected/pipelined-goto-wrong-abi-stripped-gte.c.
- verdict: CONFIRMED

## [s5] The sandbox can score a C body of func_8004A76C that contains its rtpt.
- mechanism: --disable all keeps only header-exact PINNED gtemacro units; rtpt (.word 0x4A280030) is not among DMPSX_WORDS.
- probe: grep `c2 0x280030` on the ours side of both s5 sandbox outputs.
- result: absent in both; rtpt stripped (and local multi-insn ldv3c/stsxy3c stripped in probe B). Scorer cannot measure this function's GTE body as written with the current pinned set.
- verdict: KILLED
- kill_scope: instance
- measured_on: engine/gtemacro.py pinned set as of 2026-10-15, both s5 candidate files, no FAKE construct

## Frontier (reset, s5)
1. ASM-WHOLE grant correct on A6 reasons (a)+(c) (caller-owned unsaved $s0 ABI; reaching it from C needs a global register var = pin). next: escalation-modality disposition entry citing s5's corrected reason set.
2. Scorer gap: pin gte_rtpt / gte_ldv3c / gte_stsxy3c header-exact units (operator engine: commit) — would let probe B be measured fully; still capped by the $s0 wall.

## [s5] The s2/s3/s4 instance kills are void because their chassis or FAKE state changed since they were measured.
- mechanism: Instance kills measured under a FAKE carrier or older chassis are re-testable.
- probe: Compared each measured_on with current src/main/3AB48.c:30 (bare INCLUDE_ASM stub); searched banked forms for FAKE constructs (none ever banked), so fake_ablate.py had nothing to ablate.
- result: All three were measured on the identical stub with no FAKE; none is void.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub, no FAKE construct in any banked form

## [s5] A natural do/while GTE loop using gte.h macros with the vertex pointer as a 4th argument approaches the target bytes.
- mechanism: GTE ops as gte.h islands; GCC emits the GPR skeleton.
- probe: sandbox func_8004A76C --disable all --diff --candidate tmp/grind/func_8004A76C/s5/honest_gte_loop.c
- result: Score 33, 3 source-level hunks, 0 operand-only: per-vector addiu bases, no pipelining, $a3 base vs unsaved $s0, rtpt stripped. Signature asserts a false ABI against the hand-asm caller.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s5/honest_gte_loop.c over the 3AB48 stub chassis, no FAKE construct

## [s5] A6 audit reason (b): the hand software-pipelined loop shape (j into second half, rtpt N+1 before N's sh stores) has no C spelling.
- mechanism: goto-into-middle C reproduces the rotated CFG and GCC preserves label order.
- probe: sandbox func_8004A76C --disable all --diff --candidate tmp/grind/func_8004A76C/s5/pipelined_goto.c
- result: Score 30; the GPR skeleton (sh -6/-4/-2(a2), mfc2 $17-$19, branch-into-middle, tail sh 0/2/4) aligns with target; residual is only stripped GTE statements plus the $s0 base. Reason (b) is refuted; the canonical grant still stands on reasons (a) caller-owned $s0 ABI and (c) global-register-var pin.
- verdict: CONFIRMED

## [s5] The sandbox scores a C body of func_8004A76C with its gte_rtpt() retained under the 2026-10-15 pinned gtemacro set.
- mechanism: --disable all keeps only header-exact PINNED gtemacro units; rtpt's .word 0x4A280030 is not in DMPSX_WORDS.
- probe: grep for c2 0x280030 on the ours side of both s5 sandbox outputs.
- result: Absent in both outputs; rtpt stripped, and probe B's local multi-insn ldv3c/stsxy3c stripped too.
- verdict: KILLED
- kill_scope: instance
- measured_on: engine/gtemacro.py pinned set 2026-10-15, both s5 candidate files, no FAKE construct

## s6 (solver)

## [s6] KILL RE-AUDIT — the closest banked form (rejected/pipelined-goto-wrong-abi-stripped-gte.c) scores differently on the current chassis.
- mechanism: instance kills are chassis-relative; re-measure before new probes.
- probe: sandbox func_8004A76C --disable all --diff --candidate memory/grind/func_8004A76C/rejected/pipelined-goto-wrong-abi-stripped-gte.c (tmp/grind/func_8004A76C/s6/sandbox_reaudit_pipelined.txt). No FAKE construct in the form, so fake_ablate.py has nothing to ablate.
- result: score 30, 5 source-level / 0 operand-only / 0 not-scored hunks — identical to s5. Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c stub chassis + rejected/pipelined-goto-wrong-abi-stripped-gte.c, no FAKE construct

## [s6] The sandbox residual of the pipelined form is an RA/SCHED residual the solver models can search.
- mechanism: inverse_compose.py classify triages PRE-RA / RA / SCHED.
- probe: inverse_compose.py classify main/3AB48 func_8004A76C --target-object build/src/main/3AB48.o --ours-object tmp/sandbox/func_8004A76C/main/3AB48.o (run inside WSL; the Windows-side run dies on missing objdump). Output tmp/grind/func_8004A76C/s6/classify_pipelined.txt.
- result: FIRST DIVERGENCE: PRE-RA, "no backend" — the instruction multiset differs because the scorer strips every lwc2/swc2/rtpt asm unit. The RA/sched solvers cannot be applied to the sandbox stream.
- verdict: KILLED
- kill_scope: instance
- measured_on: sandbox .o of rejected/pipelined-goto-wrong-abi-stripped-gte.c over the stub chassis, no FAKE construct

## [s6] With the GTE asm retained (raw instrumented cc1, no scorer stripping), the pipelined goto form reproduces the target except for the $s0 base.
- mechanism: compile the s5 form directly with tools/gcc-2.7.2/build/cc1 -da (Makefile CC_FLAGS) so nothing is stripped; read .greg dispositions.
- probe: tmp/grind/func_8004A76C/s6/probe_pipe.c -> probe_pipe.s + probe_pipe.i.{greg,lreg,dbr,...} (cc.sh).
- result: PARTIAL. The full CFG, lwc2/swc2/rtpt/mfc2 placement, j-into-middle with `addu v,24` in the delay slot, sh -6/-4/-2 and tail sh 0/2/4 all match. Five residual classes remain: (1) vertex base $a3 vs unsaved $s0 (ABI, s1 class kill); (2) mfc2 temps get $2,$3,$8 (greg: 76 in 2, 77 in 3, 78 in 8) vs target $t0,$t1,$t2; (3) no `nop; nop` before the first rtpt (gte.h:60 gte_rtpt has none; target has the 2-insn lwc2->cop2 hazard pad only on the first rtpt); (4) GCC's dbr fills the `j $31` delay slot with the last sh, target has `sh; jr ra; nop`; (5) target puts `addiu a1,12` before `beqz a0` and `addiu a0,-1` in its delay slot (decrement on both paths); GCC puts the sxy increment in the delay slot and the decrement on the fall-through.
- verdict: CONFIRMED

## [s6] Freeing $a3 (vertex pointer off the arg registers) lets GCC place the mfc2 temps in $t0-$t2.
- mechanism: global.c first-fit scan (no REG_ALLOC_ORDER for mips) takes the lowest free regno.
- probe: tmp/grind/func_8004A76C/s6/probe_tail.c (diagnostic only: vertex ptr through a file-scope pointer, `if (n-- != 0)`), raw cc1 -da; banked rejected/diag-global-vtx-z-temps-a3-t0-t1.c.
- result: z temps got $7,$8,$9 (greg "76 in 8 77 in 9 78 in 3 82 in 2" with $2 taken by a `move $2,$4` copy the post-decrement test created and $3 by the vertex pointer). Every placement follows ascending first fit over whatever is free.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s6/probe_tail.c raw cc1 (no scorer), diagnostic global-pointer chassis, no FAKE construct

## [s6] When nothing occupies $v0, $v1 or $a3 across the mfc2 temps' live ranges (the target bytes reference none of them), GCC 2.7.2 global allocation places those temps in $t0/$t1/$t2.
- mechanism: tools/gcc-2.7.2/global.c:1057 find_reg scans hard regs 0..FIRST_PSEUDO_REGISTER in ascending order (`int regno = i;` — mips defines no REG_ALLOC_ORDER) and takes the first one that is free and mode-OK; the temps cross the loop back edge so local-alloc does not take them, and they have no copy to a hard reg so no preference reorders the scan.
- probe: the two raw-cc1 dispositions above (probe_pipe: 2,3,8; probe_tail: 7,8,9) plus the predicate read at global.c:1050-1075.
- result: the target's t0/t1/t2 with $2,$3,$7 untouched is not a first-fit outcome. The only byte-invisible occupants would be asm clobber lists naming $2/$3/$7 on the cop2 statements, which assert a GPR effect rtpt/lwc2 do not have (completion-bar honesty). Independent corroboration of the hand-written-asm reading (with the `/* handwritten instruction */` mfc2 markers and the unfilled jr delay slot).
- verdict: KILLED
- kill_scope: class
- measured_on: raw cc1 compiles of probe_pipe.c and probe_tail.c (no scorer stripping), no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:1057

## Frontier (s6)
1. ASM-WHOLE grant stands on (a) the caller-owned unsaved $s0 ABI and (c) the global-register-var pin, now joined by (d): the mfc2 temps' $t0-$t2 seats are not a first-fit outcome (global.c:1057). next: escalation-modality disposition entry citing s5 + s6.
2. Scorer gap (operator engine): pin the gte_rtpt / gte_ldv3c / gte_stsxy3c units so the sandbox can score GTE bodies; s6 raw-cc1 shows the remaining residual is classes (1)-(5) above.

## [s6] The closest banked form (rejected/pipelined-goto-wrong-abi-stripped-gte.c) scores differently on the current chassis than its s5 kill recorded.
- mechanism: Instance kills are chassis-relative and must be re-measured.
- probe: sandbox --disable all --diff --candidate on that form; no FAKE present so fake_ablate has nothing to ablate
- result: score 30, 5 source-level hunks, same as s5; kill stands
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c stub chassis + rejected/pipelined-goto-wrong-abi-stripped-gte.c, no FAKE construct

## [s6] The sandbox residual of the pipelined goto form is an RA or scheduler residual the ra_solver/sched_solver models can search.
- mechanism: inverse_compose.py classify triages PRE-RA/RA/SCHED.
- probe: inverse_compose.py classify main/3AB48 func_8004A76C --target-object build/src/main/3AB48.o --ours-object tmp/sandbox/func_8004A76C/main/3AB48.o (in WSL)
- result: FIRST DIVERGENCE: PRE-RA, no backend; the scorer strips the lwc2/swc2/rtpt asm, so the instruction multiset differs
- verdict: KILLED
- kill_scope: instance
- measured_on: sandbox .o of rejected/pipelined-goto-wrong-abi-stripped-gte.c over the stub chassis, no FAKE construct

## [s6] With the GTE asm retained (raw instrumented cc1, no scorer stripping), the s5 pipelined goto form reproduces the target apart from the $s0 base.
- mechanism: A direct cc1 -da compile avoids the scorer's stripping and exposes the true GPR skeleton and allocation.
- probe: tmp/grind/func_8004A76C/s6/probe_pipe.c compiled with cc.sh; read probe_pipe.s and .greg
- result: Partial: CFG, GTE op placement, j-into-middle with the delay-slot increment, and sh offsets all match. Five residuals: $a3 vs $s0 base; mfc2 temps $2,$3,$8 vs $t0-$t2; no nop-nop before the first rtpt; dbr fills the jr delay slot with the last sh; n-- / sxy+=3 delay-slot placement
- verdict: CONFIRMED

## [s6] Moving the vertex pointer off $a3 (file-scope pointer diagnostic) puts the mfc2 temps in $t0-$t2.
- mechanism: global.c first-fit takes the lowest free regno.
- probe: tmp/grind/func_8004A76C/s6/probe_tail.c raw cc1 -da
- result: temps got $7,$8,$9; $2 was taken by a move copy from the post-decrement test and $3 by the vertex pointer
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s6/probe_tail.c raw cc1, diagnostic global-pointer chassis, no FAKE construct

## [s6] When nothing occupies $v0, $v1 or $a3 across their live ranges (the target bytes reference none of them), GCC 2.7.2 global allocation places the three mfc2 temps in $t0/$t1/$t2.
- mechanism: global.c find_reg scans hard regs in ascending regno (mips defines no REG_ALLOC_ORDER) and takes the first free, mode-OK one. The temps cross the loop back edge, so global allocation owns them, and with no hard-reg copy no preference reorders the scan.
- probe: Two raw-cc1 dispositions (probe_pipe gave 2,3,8; probe_tail gave 7,8,9) plus a read of global.c:1050-1075
- result: t0-t2 with $2/$3/$7 free is not a first-fit outcome. The only byte-invisible occupant would be an asm clobber list on the cop2 ops, which claims a GPR effect those ops do not have. This corroborates the hand-written-asm reading
- verdict: KILLED
- kill_scope: class
- measured_on: raw cc1 compiles of probe_pipe.c and probe_tail.c (no scorer stripping), no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:1057

## s7 (forensics)

## [s7] KILL RE-AUDIT: rejected/pipelined-goto-wrong-abi-stripped-gte.c on the current stub chassis.
- mechanism: instance kills are chassis-relative; re-measure the closest banked form before new probes.
- probe: sandbox func_8004A76C --disable all --diff --candidate memory/grind/func_8004A76C/rejected/pipelined-goto-wrong-abi-stripped-gte.c -> tmp/grind/func_8004A76C/s7/sandbox_reaudit_pipelined.txt. The form has no FAKE construct, so tools/fake_ablate.py has nothing to ablate.
- result: score 30, 5 source-level / 0 operand-only / 0 not-scored hunks, identical to s5/s6. The kill stands (the sandbox still strips every lwc2/swc2/rtpt unit, so the score measures only the GPR skeleton).
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub chassis + rejected/pipelined-goto-wrong-abi-stripped-gte.c, no FAKE construct

## [s7] The nop-nop pad before the first rtpt only (the loop rtpt has none) is reachable from verbatim PsyQ macros.
- mechanism: PsyQ 4.0 INLINE_C.H defines gte_rtpt() with two leading nops (tmp/gteplan/psyq400/x/INCLUDE/INLINE_C.H:489) and gte_rtpt_b() as the bare command word under "Type 2 functions without nop" (same file :814; post-DMPSX 0x4A280030 form at tmp/gteplan/ext_dmpsx/inline_c.h:825). gte_rtpt() at the entry plus gte_rtpt_b() in the loop is the natural PsyQ spelling of "the first transform needs the lwc2->cop2 hazard pad, the pipelined one does not".
- probe: tmp/grind/func_8004A76C/s7/p_f.c (raw cc1 -da, Makefile flags) -> p_f.s / p_f.body.txt.
- result: entry `lwc2 x6; nop; nop; rtpt; j .L2; addu v,24` and the loop `rtpt` without nops match the target skeleton exactly. s6 residual (3) is closed as a source-spelling question, not a compiler one.
- verdict: CONFIRMED

## [s7] Moving `sxy += 3` above the three mfc2 reads lets reorg put the n decrement in the beqz delay slot (the target's slot content).
- mechanism: sched.c:1979 makes every volatile asm (the gte.h mfc2/lwc2/swc2 macros) a full dependence barrier, so the source order of `sxy += 3` relative to the mfc2s survives into dbr. With an asm immediately before the branch, stop_search_p (reorg.c:744) ends the backward scan and the thread fill steals `n--` (a0 is dead at the exit label).
- probe: p_b.c (sxy += 3 before the mfc2s) vs p_base.c (s6 order), raw cc1 -da; .dbr dumps and *.body.txt in tmp/grind/func_8004A76C/s7/.
- result: p_b gives `swc2 x3; addu a1,12; mfc2 x3; beq a0,.L4; addu a0,-1`: the slot matches the target, but addu a1 now sits BEFORE the mfc2s (target: after). p_base gives `mfc2 x3; beq; addu a1,12` with `addu a0,-1` on the fall-through. p_c (sxy += 3 moved into the loop arm) gives slot `addu a0,-1` with addu a1 after the branch. p_e (`if (n-- != 0)`) adds a `move $2,$4` copy before the branch. None gives the target's `mfc2 x3; addiu a1,12; beqz a0; addiu a0,-1`.
- verdict: CONFIRMED

## [s7] GCC 2.7.2 dbr emits the target's `mfc2 x3; addiu a1,a1,12; beqz a0,exit; addiu a0,a0,-1` (an eligible ALU insn directly before a conditional branch while the slot holds a decrement of the branch's own operand).
- mechanism: reorg.c fill_simple_delay_slots runs first for every branch and scans BACKWARD from it (loop at reorg.c:2978); stop_search_p (reorg.c:719-744) halts only on a label, jump, SEQUENCE or asm. The immediately preceding `addiu a1,a1,12` neither references nor sets anything the branch sets or needs (it touches only a1; the beqz reads a0), so it passes the conflict test at reorg.c:2990, and mips.md:125 define_delay accepts it (dslot=no, length=1): it is moved into the slot. The a0 decrement cannot come from before the branch (it sets a resource the branch needs); it can only enter the slot by thread stealing, which runs only when the backward scan filled nothing. Both cannot hold in one dbr output, and no pass after dbr reorders insns. Source ordering only chooses WHICH insn precedes the branch; the predicate is about that insn.
- probe: p_base/p_b/p_c/p_e/p_f raw cc1 + reading reorg.c:2861-3030, reorg.c:719-749 and mips.md:125-133. BB2_DBR_DEBUG produced no DBRDBG lines from tools/gcc-2.7.2/build/cc1 (that binary lacks the instrumentation), so dispositions were read from the .dbr dumps.
- result: every measured ordering gives one of the two GCC shapes (slot = preceding addu a1, or addu a1 elsewhere + slot = a0 decrement); the target's combination never appears. Independent corroboration of hand-scheduled `.set noreorder` asm, joining s1 ($s0 ABI) and s6 ($t0-$t2 first-fit).
- verdict: KILLED
- kill_scope: class
- measured_on: raw tools/gcc-2.7.2/build/cc1 -O2 compiles of s7 p_base/p_b/p_c/p_e/p_f (no scorer stripping), no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reorg.c:2990

## [s7] A plain (non-volatile) store tail leaves the `jr ra` delay slot as nop, as in the target (sh; sh; sh; jr ra; nop).
- mechanism: the mips (return) jump_insn (p_f.i.dbr insn 105, parallel[(return) ...]) goes through the same backward scan; the final `sh z3,4(sz)` is eligible and conflict-free, so it moves into the slot.
- probe: p_base, p_b, p_c, p_e, p_f (plain stores) and p_d.c = p_base with `volatile s16 *sz` (diagnostic only).
- result: every plain-store form ends `sh; sh; j $31; sh`. With a volatile-qualified sz the store stays out (`sh; sh; sh; j $31` + nop) and the loop is otherwise unchanged, so the only measured way to leave the slot empty is volatile on an ordinary RAM output buffer, which is volatile coercion outside the volatile catalog (completion-bar wall).
- verdict: KILLED
- kill_scope: instance
- measured_on: raw cc1 compiles of s7 p_base/p_b/p_c/p_e/p_f (plain stores) and p_d (volatile sz diagnostic), no FAKE construct

## Frontier (s7)
1. Residual set of the closest form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) is now four items, three of them class-killed (s1 $s0 ABI, s6 global.c:1057 seats, s7 reorg.c:2990 slot) and the fourth (jr slot) reachable only through forbidden volatile. next: escalation-modality disposition entry citing s1/s6/s7, keeping the ASM-WHOLE canonical GTE grant.
2. Scorer gap (operator engine): pin gte_rtpt / gte_rtpt_b / gte_ldv3c / gte_stsxy3c units so the sandbox can score GTE bodies.

## [s7] KILL RE-AUDIT: rejected/pipelined-goto-wrong-abi-stripped-gte.c scores differently on the current stub chassis than its s5/s6 kill recorded.
- mechanism: Instance kills are chassis-relative and must be re-measured before new probes.
- probe: sandbox func_8004A76C --disable all --diff --candidate on that form (tmp/grind/func_8004A76C/s7/sandbox_reaudit_pipelined.txt); no FAKE in the form, so fake_ablate has nothing to ablate.
- result: Score 30, 5 source-level / 0 operand-only / 0 not-scored hunks, identical to s5/s6. Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c INCLUDE_ASM stub chassis + rejected/pipelined-goto-wrong-abi-stripped-gte.c, no FAKE construct

## [s7] The nop-nop pad before the first rtpt only (loop rtpt has none) is reachable from verbatim PsyQ macros: gte_rtpt() at entry, gte_rtpt_b() in the loop.
- mechanism: PsyQ 4.0 INLINE_C.H:489 gte_rtpt emits nop,nop,cop2; INLINE_C.H:814 gte_rtpt_b ('Type 2 functions without nop') emits the bare command word.
- probe: tmp/grind/func_8004A76C/s7/p_f.c compiled with raw tools/gcc-2.7.2/build/cc1 -da (Makefile flags) -> p_f.s / p_f.body.txt.
- result: Entry 'lwc2 x6; nop; nop; rtpt; j .L2; addu v,24' and the nop-less loop rtpt match the target skeleton exactly; s6 residual (3) is closed as a source spelling.
- verdict: CONFIRMED

## [s7] Placing sxy += 3 above the three mfc2 reads makes reorg fill the beqz delay slot with the n decrement (the target's slot content).
- mechanism: sched.c:1979 makes each volatile asm a full dependence barrier, so source order survives to dbr; an asm right before the branch stops the backward scan (reorg.c:744) and the thread fill steals n-- (a0 dead at the exit label).
- probe: p_b.c vs p_base.c (plus p_c: increment in loop arm; p_e: if (n-- != 0)), raw cc1 -da, dumps in tmp/grind/func_8004A76C/s7/.
- result: p_b: 'swc2 x3; addu a1,12; mfc2 x3; beq a0; [addu a0,-1]' -- slot matches but addu a1 moved before the mfc2s. p_base: slot = addu a1. p_c: slot = a0 decrement, addu a1 after the branch. p_e: extra move $2,$4 copy.
- verdict: CONFIRMED

## [s7] GCC 2.7.2 dbr emits the target's 'mfc2 x3; addiu a1,a1,12; beqz a0; [addiu a0,a0,-1]' sequence (an eligible conflict-free ALU insn directly before a conditional branch while the slot holds a decrement of the branch operand).
- mechanism: reorg.c fill_simple_delay_slots runs first and scans backward from each branch; stop_search_p (reorg.c:719-744) stops only at a label, jump, SEQUENCE or asm. The preceding addiu a1 neither references nor sets the branch's resources (reorg.c:2990 test) and mips.md:125 define_delay accepts it, so it is moved into the slot. The a0 decrement can only reach the slot by thread stealing, which runs only when the backward scan filled nothing. No pass after dbr reorders insns.
- probe: raw cc1 -da compiles p_base/p_b/p_c/p_e/p_f and the .dbr dumps, read against reorg.c:2861-3030 and mips.md:125-133.
- result: Every measured ordering yields one of the two GCC shapes (slot = preceding addu a1, or addu a1 elsewhere with slot = a0 decrement); the target's combination never appears. A fourth independent hand-scheduled-asm signal alongside the $s0 ABI (s1) and the $t0-$t2 seats (s6).
- verdict: KILLED
- kill_scope: class
- measured_on: raw tools/gcc-2.7.2/build/cc1 -O2 compiles of s7 p_base/p_b/p_c/p_e/p_f (no scorer stripping), no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reorg.c:2990

## [s7] A plain (non-volatile) store tail leaves the jr ra delay slot as nop, as in the target's 'sh; sh; sh; jr ra; nop'.
- mechanism: The mips (return) jump_insn (p_f.i.dbr insn 105) goes through the same backward scan; the final sh is eligible and conflict-free.
- probe: p_base/p_b/p_c/p_e/p_f (plain stores) and diagnostic p_d.c (volatile s16 *sz).
- result: Every plain-store form ends 'sh; sh; j $31; [sh]'. Only volatile-qualified sz left the slot empty -- volatile coercion of an ordinary RAM buffer, outside the volatile catalog.
- verdict: KILLED
- kill_scope: instance
- measured_on: raw cc1 compiles of s7 p_base/p_b/p_c/p_e/p_f (plain stores) and p_d (volatile sz diagnostic), no FAKE construct

## s8 (rederive)

### H-s8a — an independent tool (m2c) run fresh on this function's asm produces a C signature that cannot place the asm's real third input ($s0) as a formal parameter, reproducing the s1 ABI finding via a different method.
- mechanism: m2c infers C parameters from $a0-$a3 usage per MIPS o32-derived convention; it has no representation for a value live-in via a non-argument register ($s0), same constraint GCC's own call-ABI expects on the C side (s1 class kill).
- probe: `python3 tools/m2c/m2c.py -t mips-ido-c -f func_8004A76C asm/funcs/func_8004A76C.s` (tmp/grind/func_8004A76C/s8/m2c_out.c).
- result: m2c emits `void func_8004A76C(s32 arg0, void *arg2)` — two params only; $s0 never appears as a declared variable, only inside opaque `M2C_ERROR` GTE stubs. GTE ops (lwc2/swc2/mfc2/rtpt) are all unknown-instruction stubs, as expected (no C form).
- verdict: KILLED
- kill_scope: instance
- measured_on: m2c (mips-ido-c target) fresh decompile of asm/funcs/func_8004A76C.s this session, no FAKE construct, no C body on either side
- (not a class kill: this is one tool's heuristic on one asm file, re-confirming the already-standing s1 class kill rather than establishing a new predicate.)

### H-s8b — the 461-entry gcc2.7.2-psx decomp.me corpus cached under tmp/decomp_me_corpus/ (downloaded by the func_8004A4E0 s12 session) contains a GTE-pipelined scratch close enough to transplant a structurally different C shape from.
- mechanism: shingle-similarity search over cached decomp.me scratches, same tool/corpus the sibling used.
- probe: `python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8004A76C.s --corpus tmp/decomp_me_corpus --top 10` (tmp/grind/func_8004A76C/s8/decomp_me_search.txt).
- result: best similarity 0.011 against RecordButtonsToDevkit (unrelated devkit I/O), no GTE-shaped scratch anywhere in top 10. Null result.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/decomp_me_corpus (461 scratches, cached 2026-10-15 by func_8004A4E0 s12) against asm/funcs/func_8004A76C.s, no FAKE construct

### H-s8c — func_8004A4E0's candidate.c or the completed `puts` module contains a transplantable code block for this function (mandatory sibling-ledger check).
- mechanism: sibling-transplant rule (dispatch SIBLING LEDGERS section) requires reading and attempting transplant before any own probe.
- probe: read memory/grind/func_8004A4E0/candidate.c in full this session; read src/main/psxsdk/libc2/puts.c's relationship to this function (shares nothing but being in the same binary).
- result: func_8004A4E0's candidate.c is comments-only over its own unmodified INCLUDE_ASM stub (zero C statements — same "nothing to transplant" state s11/s13-17 of that ledger already found for itself). puts.c is an unrelated libc routine with zero shared code shape (string output vs GTE transform). No block exists on either side to transplant.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_8004A4E0/candidate.c at HEAD (written 2026-10-15 17:03) + src/main/psxsdk/libc2/puts.c, no FAKE construct

## [s8] An independent tool (m2c) run fresh on this function's asm produces a C signature that cannot place the asm's real third input ($s0) as a formal parameter, reproducing the s1 ABI finding via a different method.
- mechanism: m2c infers C parameters from $a0-$a3 usage per MIPS o32-derived convention; it has no representation for a value live-in via a non-argument register ($s0), the same constraint GCC's own call ABI expects on the C side (s1 class kill).
- probe: python3 tools/m2c/m2c.py -t mips-ido-c -f func_8004A76C asm/funcs/func_8004A76C.s
- result: m2c emits 'void func_8004A76C(s32 arg0, void *arg2)' -- two params only; $s0 never appears as a declared variable, only inside opaque M2C_ERROR GTE stubs. All GTE ops (lwc2/swc2/mfc2/rtpt) are unknown-instruction stubs as expected.
- verdict: KILLED
- kill_scope: instance
- measured_on: m2c (mips-ido-c target) fresh decompile of asm/funcs/func_8004A76C.s this session, no FAKE construct, no C body on either side

## [s8] The 461-entry gcc2.7.2-psx decomp.me corpus cached under tmp/decomp_me_corpus/ contains a GTE-pipelined scratch close enough to transplant a structurally different C shape from.
- mechanism: shingle-similarity search over cached decomp.me scratches, same tool/corpus the func_8004A4E0 sibling used.
- probe: python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8004A76C.s --corpus tmp/decomp_me_corpus --top 10
- result: best similarity 0.011 against RecordButtonsToDevkit (unrelated devkit I/O), no GTE-shaped scratch anywhere in top 10 -- null result.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/decomp_me_corpus (461 scratches, cached 2026-10-15 by func_8004A4E0 s12) against asm/funcs/func_8004A76C.s, no FAKE construct

## [s8] func_8004A4E0's candidate.c or the completed puts module contains a transplantable code block for this function (mandatory sibling-ledger check).
- mechanism: sibling-transplant rule requires reading and attempting transplant before any own probe.
- probe: read memory/grind/func_8004A4E0/candidate.c in full this session; compared src/main/psxsdk/libc2/puts.c's relationship to this function.
- result: func_8004A4E0's candidate.c is comments-only over its own unmodified INCLUDE_ASM stub (zero C statements). puts.c is an unrelated libc routine sharing no code shape. No block exists on either side to transplant.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_8004A4E0/candidate.c at HEAD (written 2026-10-15 17:03) + src/main/psxsdk/libc2/puts.c, no FAKE construct

## s9 (forensics)

### H-s9a — KILL RE-AUDIT: the closest banked form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) still measures as banked on the current stub chassis, with no FAKE construct present.
- mechanism: mandatory re-audit (floor flat 3+ sessions); a kill measured on a stale chassis or under a FAKE carrier would be void.
- probe: `sandbox func_8004A76C --disable all --diff --candidate memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c` (tmp/grind/func_8004A76C/s9/sandbox_psyq_pair.txt); `python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate <same>` (s9/fake_ablate_psyq_pair.txt).
- result: score 32, 5 hunks, all source-level (lwc2/swc2/rtpt asm stripped as unpinned gtemacro units; mfc2 temps v0/v1/a1 vs target t0-t2; `bnez` vs target `beqz`; jr slot holds `sh` vs target nop). fake_ablate: "no FAKE-annotated constructs found; nothing to ablate". The s7 rejection stands on the current chassis. Stub chassis itself re-measured: score 37, no C body (s9/sandbox_stub.txt).
- verdict: CONFIRMED

### H-s9b — some PsyQ inline GTE header macro (a header-exact unit, not a pin) writes the mfc2 results into $8-$10 or clobbers $2/$3 across the z live range, which would give the target's $t0-$t2 seats without a register pin.
- mechanism: global.c first-fit (s6) takes v0/v1 first; only a hard-register clobber/output in a header-exact macro could exclude them honestly.
- probe: grep of all 8 cached header copies (tmp/gteplan/ext_{dmpsx,nugget,psn00b,sh,xg}/inline_{c,o,n}.h, 23 mfc2 lines each) for mfc2 into $8/$9/$10 and for gte_rtpt/gte_rtpt_b/gte_ldv3c/gte_stsxy3c clobber lists.
- result: the only $9/$10 hits are `mfc2 $12, $9` / `mfc2 $13, $10` (GTE IR1/IR2 into $t4/$t5 -- operands reversed from the needed shape); no macro reads GTE 17-19 into a GPR at all (gte_stsz3/gte_stsz3c use swc2 $17-$19). inline_o.h clobbers are "$12","$13","$14","$15","memory" only; no macro clobbers $2/$3.
- verdict: KILLED
- kill_scope: instance
- measured_on: 8 cached PsyQ/psn00b/nugget inline header copies under tmp/gteplan/, no C body, no FAKE construct

### H-s9c — an explicit trailing `return;` after the final sz stores keeps a return label in front of return_internal and so leaves the jr $ra slot empty.
- mechanism: reorg.c:2969 backward scan stops at a CODE_LABEL (stop_search_p, reorg.c:732-733).
- probe: s9/q_a.c = s7 p_f + trailing `return;`, raw instrumented cc1 via s9/cc.sh.
- result: q_a.s is identical to p_f.s at the tail (`sh $2,0($6); sh $3,2($6); j $31; sh $8,4($6)`): the jump to return_label is a jump-to-next, deleted by jump opt together with the label, so nothing stops the scan.
- verdict: KILLED
- kill_scope: instance
- measured_on: s9/q_a.c raw cc1 (tools/gcc-2.7.2/build/cc1 -O2 -G0, $a3 vertex-pointer chassis), no FAKE construct

### H-s9d — a `return` inside the loop (exit arm `if (n == 0) { stores; return; }` inside `for (;;)`) leaves a live CODE_LABEL before return_internal, empties the jr slot, and keeps the rest of the target shape.
- mechanism: the in-loop return emits a jump to return_label that is not a jump-to-next, so the label survives jump2 and stop_search_p (reorg.c:732-733) halts the reorg.c:2969 scan before any sh.
- probe: s9/q_c.c, raw instrumented cc1; read q_c.i.jump2 / q_c.i.dbr (code_label 107 sits directly before return_internal insn at dbr lines 243-252).
- result: mechanism CONFIRMED: `.L1: j $31` printed under `.set reorder` with no filled slot (the assembler supplies the nop, as in the target). Target shape LOST: the exit arm is laid out inline after `bne $4,$0,.L5` (target: `beqz` to a trailing store block), it ends `j .L1; sh $2,6($3)` (2 extra insns), sz uses a second pointer `$3 = sz-2`, and mfc2 temps are $9/$8/$2 each copied by an extra `move` to $12/$11/$10. Net further from the target than p_f. Banked rejected/return-in-loop-label-empty-jr-slot-extra-jump.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s9/q_c.c raw cc1 ($a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm), no FAKE construct

## [s9] KILL RE-AUDIT: rejected/psyq-rtpt-pair-reorg-slot-residual.c still measures as banked on the current stub chassis with no FAKE construct present
- mechanism: mandatory re-audit after 3 flat sessions
- probe: sandbox --disable all --diff --candidate on the rejected form; tools/fake_ablate.py on the same file
- result: score 32, 5 source-level hunks (GTE asm stripped as unpinned units, v0/v1/a1 mfc2 temps, bnez vs beqz, jr slot holds sh); fake_ablate found nothing to ablate; stub chassis 37
- verdict: CONFIRMED

## [s9] A header-exact PsyQ inline GTE macro writes GTE regs 17-19 into $8-$10 or clobbers $2/$3, giving the target's $t0-$t2 mfc2 seats without a pin
- mechanism: global.c first-fit takes v0/v1 unless a hard-register output/clobber excludes them
- probe: grep of 8 cached inline_c/inline_o/inline_n header copies under tmp/gteplan/ for mfc2 into $8-$10 and gte_rtpt/ldv3c/stsxy3c clobber lists
- result: only mfc2 $12,$9 / $13,$10 (GTE IR regs into $t4/$t5); no macro moves GTE 17-19 into GPRs; inline_o clobbers are $12-$15 + memory only
- verdict: KILLED
- kill_scope: instance
- measured_on: 8 cached PsyQ/psn00b/nugget inline header copies in tmp/gteplan/, no C body, no FAKE construct

## [s9] An explicit trailing return after the final sz stores keeps a return label before return_internal and leaves the jr ra slot empty
- mechanism: reorg.c:2969 backward scan stops at a CODE_LABEL (stop_search_p reorg.c:732)
- probe: s9/q_a.c = s7 p_f + trailing return, raw instrumented cc1
- result: tail identical to p_f (j $31 with sh in slot): the jump to return_label is jump-to-next, deleted with its label
- verdict: KILLED
- kill_scope: instance
- measured_on: s9/q_a.c raw cc1, $a3 vertex-pointer chassis, no FAKE construct

## [s9] An in-loop return (if (n == 0) { stores; return; } inside for(;;)) empties the jr slot while keeping the target layout
- mechanism: live return_label before return_internal halts the reorg.c:2969 scan via stop_search_p reorg.c:732
- probe: s9/q_c.c raw instrumented cc1; read q_c.i.jump2 and q_c.i.dbr
- result: slot empty (mechanism confirmed, code_label 107 directly before return_internal) but the exit arm is inline after bne with an extra j .L1 + slot, a second sz pointer and three extra moves; further from target than p_f. Banked rejected/return-in-loop-label-empty-jr-slot-extra-jump.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s9/q_c.c raw cc1, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct

## s10 (forensics)

## [s10] The residuals of the closest banked forms (s7 p_f, s9 q_c) are a divergence of the decompals port cc1 from the original PsyQ cc1psx, so cc1psx would emit the target's $t0-$t2 seats / empty jr slot from the same C.
- mechanism: if cc1psx (GCC 2.7.2.SN.1) allocated or filled delay slots differently from tools/gcc-2.7.2, the s6/s7/s9 class kills (global.c:1057 first-fit, reorg.c:2969 scan) would be port-relative, not compiler-relative.
- probe: tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -msoft-float on s7/p_f.i and s9/q_c.i (tmp/grind/func_8004A76C/s10/psx.sh); diff the function bodies against s7/p_f.s and s9/q_c.s with label syntax ($L vs .L) normalised.
- result: both bodies byte-identical to the port's output apart from label spelling: p_f.psx.s keeps mfc2 $2/$3/$8, `beq $4,$0` with `addu $4,$4,-1` in the slot, sh $8 in the jr slot; q_c.psx.s identical to q_c.s. The residuals are what the original compiler produces too.
- verdict: KILLED
- kill_scope: instance
- measured_on: s7/p_f.i and s9/q_c.i preprocessed TUs, original tools/cc1psx.exe via cc1psx_wrapper.sh vs tools/gcc-2.7.2/build/cc1, $a3 vertex-pointer chassis, no FAKE construct

## [s10] PASS-INPUT ENUMERATION for the mfc2 seats: some C-side input shape of the z temporaries (narrow type, declaration order, array, fused asm, sz element type) makes global.c skip $2/$3 and seat the three mfc2 results in $8/$9/$10.
- mechanism: global.c find_reg first-fit over call-used regs (s6 class kill, global.c:1057); the only C-side inputs are the pseudos' modes, count, conflict sets and allocno order.
- probe: six variants of s7/p_f.c generated by s10/gen.py, each compiled with port cc1 (s10/cc.sh, full -da dumps) and cc1psx (s10/psx.sh): v1 s16 z1..z3; v2 u16; v3 reversed declaration order (z3, z2, z1); v4 s32 z[3]; v5 one asm with three "=r" outputs; v6 s32 *sz with s16 arithmetic.
- result: v1/v2 collapse to mfc2 $2,$2,$2 (each narrowed temp dies at its own sh); v4 gives $3,$2,$2; v3/v5/v6 give $2,$3,$8. None gives $8,$9,$10; port == cc1psx on all six (s10/port_vs_psx.txt). Target seats remain outside every measured input.
- verdict: KILLED
- kill_scope: instance
- measured_on: six p_f-derived variants (s10/v1_s16..v6_szs32.c), $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, both port cc1 and cc1psx, no FAKE construct

## [s10] The residuals of the closest banked forms (s7 p_f, s9 q_c) are a divergence of the decompals port cc1 from original cc1psx, so cc1psx would emit the target $t0-$t2 seats or empty jr slot from the same C.
- mechanism: If cc1psx allocated (global.c first-fit) or filled delay slots (reorg.c:2969 scan) differently, the s6/s7/s9 kills would be port-relative.
- probe: tools/cc1psx_wrapper.sh on s7/p_f.i and s9/q_c.i (s10/psx.sh); bodies diffed against port output with label syntax normalised.
- result: Byte-identical apart from $L/.L label spelling: mfc2 $2/$3/$8, beq slot holds n--, jr slot holds sh $8 (p_f); q_c identical too.
- verdict: KILLED
- kill_scope: instance
- measured_on: s7/p_f.i + s9/q_c.i, tools/cc1psx.exe via cc1psx_wrapper.sh vs tools/gcc-2.7.2/build/cc1, $a3 vertex-pointer chassis, no FAKE construct

## [s10] One of six C-side input shapes of the z temporaries (s16, u16, reversed declaration, z[3] array, fused three-output mfc2 asm, s32 sz) makes global.c seat the mfc2 results in $8/$9/$10.
- mechanism: global.c find_reg first-fit (global.c:1057); C-side inputs are pseudo modes, count, conflicts and allocno order.
- probe: s10/gen.py generated v1..v6 from s7/p_f.c; each compiled with port cc1 (-da dumps) and cc1psx.
- result: s16/u16 -> $2,$2,$2; z[3] -> $3,$2,$2; reversed/fused/s32-sz -> $2,$3,$8; none $8,$9,$10; port == cc1psx on all six.
- verdict: KILLED
- kill_scope: instance
- measured_on: six p_f-derived variants s10/v1_s16..v6_szs32.c, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, port cc1 and cc1psx, no FAKE construct

## s11 (rederive)

- statement: A fresh, independent m2c run on asm/funcs/func_8004A76C.s (distinct process from s8's
  cached invocation) emits a different two-parameter signature or gives $s0 a formal-parameter
  slot, opening a new ABI-honest C path.
- probe: `python3 tools/m2c/m2c.py asm/funcs/func_8004A76C.s` (tmp/grind/func_8004A76C/s11/m2c_out.c).
- result: identical to s8 — `void func_8004A76C(s32 arg0, void *arg2)`, no $s0 parameter, every
  cop2/GTE opcode (`lwc2`,`rtpt`,`mfc2 $17-19`,`swc2`) lowered to `M2C_ERROR`.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/m2c/m2c.py fresh invocation this session, no FAKE construct, no C chassis (m2c output only)

- statement: The closest banked rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) was
  killed by a FAKE-propped chassis, so re-measuring it clean would change its verdict.
- probe: `python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate
  memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c`, then `sandbox
  func_8004A76C --disable all --candidate <same file> --diff` on the current chassis.
- result: fake_ablate reports zero FAKE constructs to ablate (file carries none). Diff re-measure:
  5/5 hunks source-level — the sandbox strips the entire unpinned gte_rtpt/lwc2/swc2/mfc2 block as
  cheat-asm, so "ours" is missing the whole GTE core, not showing a register-seat residual. This
  is the s5/s10 engine/gtemacro.py-pin gap re-confirmed on the current chassis, not a new form.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD, $a3 vertex-pointer
  chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct (confirmed via fake_ablate)

## [s11] A fresh, independent m2c run on asm/funcs/func_8004A76C.s (distinct process from s8's cached invocation) emits a different two-parameter signature or gives $s0 a formal-parameter slot, opening a new ABI-honest C path.
- mechanism: m2c's MIPS-I decompiler model vs the GTE-interpreter custom $s0-live-in ABI
- probe: python3 tools/m2c/m2c.py asm/funcs/func_8004A76C.s (tmp/grind/func_8004A76C/s11/m2c_out.c)
- result: Identical to s8: void func_8004A76C(s32 arg0, void *arg2), no $s0 parameter, every cop2/GTE opcode (lwc2, rtpt, mfc2 $17-19, swc2) lowered to M2C_ERROR.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/m2c/m2c.py fresh invocation this session, no FAKE construct, no C chassis (m2c output only)

## [s11] The closest banked rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) was killed by a FAKE-propped chassis, so re-measuring it clean on the current chassis would change its verdict.
- mechanism: engine/gtemacro.py unpinned-unit cheat-asm stripping of gte_rtpt/lwc2/swc2/mfc2
- probe: fake_ablate.py --func func_8004A76C --candidate rejected/psyq-rtpt-pair-reorg-slot-residual.c, then sandbox --disable all --candidate <same> --diff
- result: fake_ablate reports zero FAKE constructs to ablate. --diff on current chassis: 5/5 hunks source-level — the sandbox strips the whole unpinned GTE block, same engine-pin gap already banked at s5/s7/s10, not a new residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct (confirmed via fake_ablate)

## [s12] (rederive) A third fresh m2c run, plus a freshness re-check of sibling func_8004A4E0's candidate.c (moved to floor 163/23 sessions since s8 last read it at floor ~unspecified), opens a new C/transplant path.
- mechanism: m2c's MIPS-I model vs the GTE-interpreter custom $s0-live-in ABI; sibling transplant per SIBLING LEDGERS protocol
- probe: python tools/m2c/m2c.py asm/funcs/func_8004A76C.s (this session, independent of s8/s11 cached runs); read memory/grind/func_8004A4E0/candidate.c head (s1..s20 entries)
- result: m2c output byte-identical in structure to s8/s11 — `void func_8004A76C(s32 arg0, void *arg2)`, no $s0 formal param, every cop2 op (lwc2 x6, rtpt, mfc2 $17-19, swc2 x3) still M2C_ERROR, same duplicated-loop-prologue/in-loop-conditional-return shape already rejected (return-in-loop-label-empty-jr-slot-extra-jump.c). func_8004A4E0's candidate.c confirms (s8 of that ledger, still current at its s20) it has NO C body at all — pure INCLUDE_ASM comment banner, nothing to transplant; its own jtbl cross-function-target class kill is unrelated to this function's body shape. No new path opened by either axis.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/m2c/m2c.py fresh invocation this session (no FAKE construct, no C chassis produced); func_8004A4E0/candidate.c read at its current HEAD state (floor 163, 23 sessions, still zero C)

## [s12] A third fresh m2c run on asm/funcs/func_8004A76C.s (distinct process from s8/s11) emits a different two-parameter signature or gives $s0 a formal-parameter slot, opening a new ABI-honest C path.
- mechanism: m2c's MIPS-I decompiler model vs the GTE-interpreter custom $s0-live-in ABI
- probe: python tools/m2c/m2c.py asm/funcs/func_8004A76C.s
- result: Byte-identical in structure to s8/s11: void func_8004A76C(s32 arg0, void *arg2), no $s0 formal param, every cop2/GTE opcode (lwc2 x6, rtpt, mfc2 $17-19, swc2 x3) lowered to M2C_ERROR, same duplicated-loop-prologue/in-loop-conditional-return shape as the already-rejected return-in-loop-label-empty-jr-slot-extra-jump.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/m2c/m2c.py fresh invocation this session, no FAKE construct, no C chassis produced (m2c output only)

## [s12] Sibling func_8004A4E0's candidate.c, re-read at its current state (23 sessions and a floor move to 163 since s8 last checked it), now contains a C body or block worth transplanting onto func_8004A76C's chassis.
- mechanism: sibling-transplant protocol (SIBLING LEDGERS section of brief)
- probe: Read memory/grind/func_8004A4E0/candidate.c head and s1/s8/s20 entries
- result: func_8004A4E0/candidate.c is still a pure INCLUDE_ASM comment banner (ASM-WHOLE canonical GTE grant, owner ruling Q40) -- zero C bytes, nothing structurally present to transplant. Its own class kill (jtbl entries living inside func_8004A940's body, s8 of that ledger) is about cross-function jump-table targets, unrelated to this function's GTE-op body shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_8004A4E0/candidate.c at its current HEAD (floor 163, 23 sessions since its s1), no FAKE construct, no C chassis
