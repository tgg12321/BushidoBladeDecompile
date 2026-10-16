# Hypothesis ledger — func_8004A76C

> COMPACTED 2026-10-15 by `grindlib.py compact-ledger` (67,913 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show e0514a635:memory/grind/func_8004A76C/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
> Look one up only when its index line bears on your probe. Append below as usual.

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
… (preamble truncated — full text in git, above)

## Current state (at compaction)
floor 37 at s12; 12 sessions; live frontier (state.json is authoritative):
  - The ASM-WHOLE canonical GTE grant (already recorded in inline_asm_canonical.txt) is the correct, final disposition; eight modalities now (recon/structural/permuter/enumerate/synthesis/solver/forensics x2/rederive x3) pl…
  - Pinning gte_rtpt / gte_rtpt_b / gte_ldv3c / gte_stsxy3c units lets the sandbox score GTE bodies, which would let the closest rejected form (re-confirmed clean of FAKE constructs at s11) be re-measured as a real residual…

## Class-level kills (binding until the cited predicate changes)

## [s1] A GCC file-scope global-register-variable (register T x asm("$16")) could reproduce the unsaved-$s0 S7 shape from ordinary C, making this function decompilable rather than whole-body hand asm.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/INLINE-ASM-AUDIT-2026-10-01.md:93
- result: A6 already closed this with three independent reasons: (1) the sole caller func_8004A4E0 is itself whole-body hand asm handing $s0 live across the jal in a way GCC cannot emit; (2) the loop is hand software-pipelined (rtpt for iteration N+1 issued before iteration N's stores retire, entry j jumps into the unrolled loop's second half) — not a GCC-schedulable shape; (3) a global register variable i…

## [s6] When nothing occupies $v0, $v1 or $a3 across the mfc2 temps' live ranges (the target bytes reference none of them), GCC 2.7.2 global allocation places those temps in $t0/$t1/$t2.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/global.c:1057
- result: the target's t0/t1/t2 with $2,$3,$7 untouched is not a first-fit outcome. The only byte-invisible occupants would be asm clobber lists naming $2/$3/$7 on the cop2 statements, which assert a GPR effect rtpt/lwc2 do not have (completion-bar honesty). Independent corroboration of the hand-written-asm reading (with the `/* handwritten instruction */` mfc2 markers and the unfilled jr delay slot).

## [s6] When nothing occupies $v0, $v1 or $a3 across their live ranges (the target bytes reference none of them), GCC 2.7.2 global allocation places the three mfc2 temps in $t0/$t1/$t2.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/global.c:1057
- result: t0-t2 with $2/$3/$7 free is not a first-fit outcome. The only byte-invisible occupant would be an asm clobber list on the cop2 ops, which claims a GPR effect those ops do not have. This corroborates the hand-written-asm reading

## [s7] GCC 2.7.2 dbr emits the target's `mfc2 x3; addiu a1,a1,12; beqz a0,exit; addiu a0,a0,-1` (an eligible ALU insn directly before a conditional branch while the slot holds a decrement of the branch's own operand).
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/reorg.c:2990
- result: every measured ordering gives one of the two GCC shapes (slot = preceding addu a1, or addu a1 elsewhere + slot = a0 decrement); the target's combination never appears. Independent corroboration of hand-scheduled `.set noreorder` asm, joining s1 ($s0 ABI) and s6 ($t0-$t2 first-fit).

## [s7] GCC 2.7.2 dbr emits the target's 'mfc2 x3; addiu a1,a1,12; beqz a0; [addiu a0,a0,-1]' sequence (an eligible conflict-free ALU insn directly before a conditional branch while the slot holds a decrement of the branch operand).
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/reorg.c:2990
- result: Every measured ordering yields one of the two GCC shapes (slot = preceding addu a1, or addu a1 elsewhere with slot = a0 decrement); the target's combination never appears. A fourth independent hand-scheduled-asm signal alongside the $s0 ABI (s1) and the $t0-$t2 seats (s6).

## Confirmed levers

## [s2] func_8004A808 and func_8004A940 (same jal-cluster, same custom callee-saved-register ABI reopened alongside func_8004A76C) will scan STRONG on scan_hand_coded the same way, strengthening a cluster-wide record ahead of any one member's escalation.
- verdict: CONFIRMED
- result: func_8004A808: tier=STRONG score=3/8 (S3 no-spills, S4 front-loads, S7 unsaved $s0 -- 'GCC-impossible, decisive'). func_8004A940: tier=STRONG score=3/8 (same S3/S4/S7 triplet, S7 firing on all eight callee-saves $s0-$s7, the maximal shape). Both independently confirm the S7 signal already measured on func_8004A76C itself; only the cluster's caller func_8004A4E0 scans LOW (0/8), the asymmetry alre…

## [s5] A6 reason (b) — the hand software-pipelining (jump into loop second half, rtpt N+1 before N's stores) has no C form.
- verdict: CONFIRMED
- result: 30; GPR skeleton (sh -6/-4/-2(a2), mfc2 $17/$18/$19, branch-into-middle j, tail sh 0/2/4) aligns with the target; residual is only stripped GTE statements + $s0 base. Reason (b) is REFUTED as a standalone blocker — pipelining is expressible in C. Grant still stands on (a) caller ABI + (c) pin. Banked rejected/pipelined-goto-wrong-abi-stripped-gte.c.

## [s5] A6 audit reason (b): the hand software-pipelined loop shape (j into second half, rtpt N+1 before N's sh stores) has no C spelling.
- verdict: CONFIRMED
- result: Score 30; the GPR skeleton (sh -6/-4/-2(a2), mfc2 $17-$19, branch-into-middle, tail sh 0/2/4) aligns with target; residual is only stripped GTE statements plus the $s0 base. Reason (b) is refuted; the canonical grant still stands on reasons (a) caller-owned $s0 ABI and (c) global-register-var pin.

## [s6] With the GTE asm retained (raw instrumented cc1, no scorer stripping), the pipelined goto form reproduces the target except for the $s0 base.
- verdict: CONFIRMED
- result: PARTIAL. The full CFG, lwc2/swc2/rtpt/mfc2 placement, j-into-middle with `addu v,24` in the delay slot, sh -6/-4/-2 and tail sh 0/2/4 all match. Five residual classes remain: (1) vertex base $a3 vs unsaved $s0 (ABI, s1 class kill); (2) mfc2 temps get $2,$3,$8 (greg: 76 in 2, 77 in 3, 78 in 8) vs target $t0,$t1,$t2; (3) no `nop; nop` before the first rtpt (gte.h:60 gte_rtpt has none; target has th…

## [s6] With the GTE asm retained (raw instrumented cc1, no scorer stripping), the s5 pipelined goto form reproduces the target apart from the $s0 base.
- verdict: CONFIRMED
- result: Partial: CFG, GTE op placement, j-into-middle with the delay-slot increment, and sh offsets all match. Five residuals: $a3 vs $s0 base; mfc2 temps $2,$3,$8 vs $t0-$t2; no nop-nop before the first rtpt; dbr fills the jr delay slot with the last sh; n-- / sxy+=3 delay-slot placement

## [s7] The nop-nop pad before the first rtpt only (the loop rtpt has none) is reachable from verbatim PsyQ macros.
- verdict: CONFIRMED
- result: entry `lwc2 x6; nop; nop; rtpt; j .L2; addu v,24` and the loop `rtpt` without nops match the target skeleton exactly. s6 residual (3) is closed as a source-spelling question, not a compiler one.

## [s7] Moving `sxy += 3` above the three mfc2 reads lets reorg put the n decrement in the beqz delay slot (the target's slot content).
- verdict: CONFIRMED
- result: p_b gives `swc2 x3; addu a1,12; mfc2 x3; beq a0,.L4; addu a0,-1`: the slot matches the target, but addu a1 now sits BEFORE the mfc2s (target: after). p_base gives `mfc2 x3; beq; addu a1,12` with `addu a0,-1` on the fall-through. p_c (sxy += 3 moved into the loop arm) gives slot `addu a0,-1` with addu a1 after the branch. p_e (`if (n-- != 0)`) adds a `move $2,$4` copy before the branch. None gives…

## [s7] The nop-nop pad before the first rtpt only (loop rtpt has none) is reachable from verbatim PsyQ macros: gte_rtpt() at entry, gte_rtpt_b() in the loop.
- verdict: CONFIRMED
- result: Entry 'lwc2 x6; nop; nop; rtpt; j .L2; addu v,24' and the nop-less loop rtpt match the target skeleton exactly; s6 residual (3) is closed as a source spelling.

## [s7] Placing sxy += 3 above the three mfc2 reads makes reorg fill the beqz delay slot with the n decrement (the target's slot content).
- verdict: CONFIRMED
- result: p_b: 'swc2 x3; addu a1,12; mfc2 x3; beq a0; [addu a0,-1]' -- slot matches but addu a1 moved before the mfc2s. p_base: slot = addu a1. p_c: slot = a0 decrement, addu a1 after the branch. p_e: extra move $2,$4 copy.

## [s9] KILL RE-AUDIT: rejected/psyq-rtpt-pair-reorg-slot-residual.c still measures as banked on the current stub chassis with no FAKE construct present
- verdict: CONFIRMED
- result: score 32, 5 source-level hunks (GTE asm stripped as unpinned units, v0/v1/a1 mfc2 temps, bnez vs beqz, jr slot holds sh); fake_ablate found nothing to ablate; stub chassis 37

## Instance kills and other entries (index; chassis-relative, re-testable)

## [s2] Structural-modality levers (block-local var splits, declaration order, type narrowing, statement re-association) require an existing C body with a sandbox-measured non-zero distance to operate on; this function has neit…
- verdict: KILLED (instance) — canonical still ASM-WHOLE ('whole-function canonical-asm grant in inline_asm_canonical.txt'); sandbox still {"score":37,"no_c_body":true,"build_insns":0}; --di…

## [s3] A decomp-permuter campaign can be constructed for func_8004A76C to search for a C-level match of the diverging region.
- verdict: KILLED (instance) — import.py fails immediately: 'Failed to find GLOBAL_ASM for function func_8004A76C'. src/main/3AB48.c:30 is a bare INCLUDE_ASM("asm/funcs", func_8004A76C); sta…

## [s4] enumerate — a systematic spelling sweep (`spelling_enum.py` / `sweep_variants.py`) over an `/* ENUM-BEGIN */.../* ENUM-END */` region can search the diverging block's C spelling space for func_8004A76C.
- verdict: KILLED (instance) — enumerate modality is mechanically inapplicable to this function for the same reason structural (s2) and permuter (s3) modalities were — there is no C body. Th…

## [s4] A systematic spelling sweep (spelling_enum.py / sweep_variants.py over an /* ENUM-BEGIN */.../* ENUM-END */ region) can search func_8004A76C's diverging C spelling space for an exact-match form.
- verdict: KILLED (instance) — enumerate modality is mechanically inapplicable to this function for the identical reason structural (s2) and permuter (s3) modalities were inapplicable: there…

## [s5] KILL RE-AUDIT — the s2/s3/s4 instance kills could be void because their chassis or FAKE state changed.
- verdict: KILLED (instance) — all three kills were measured on the identical stub with no FAKE; none is void. The weakest foreclosure is A6 reason (b) (pipelining), re-audited below.

## [s5] A natural honest GTE loop (gte.h macros, vertex pointer as 4th arg) approaches the target.
- verdict: KILLED (instance) — 33, 3 source-level hunks, 0 operand-only; per-vector addiu bases, unpipelined, $a3 base; rtpt stripped by the scorer. Wrong ABI ($a3 vs unsaved $s0). Banked re…

## [s5] The sandbox can score a C body of func_8004A76C that contains its rtpt.
- verdict: KILLED (instance) — absent in both; rtpt stripped (and local multi-insn ldv3c/stsxy3c stripped in probe B). Scorer cannot measure this function's GTE body as written with the curr…

## [s5] The s2/s3/s4 instance kills are void because their chassis or FAKE state changed since they were measured.
- verdict: KILLED (instance) — All three were measured on the identical stub with no FAKE; none is void.

## [s5] A natural do/while GTE loop using gte.h macros with the vertex pointer as a 4th argument approaches the target bytes.
- verdict: KILLED (instance) — Score 33, 3 source-level hunks, 0 operand-only: per-vector addiu bases, no pipelining, $a3 base vs unsaved $s0, rtpt stripped. Signature asserts a false ABI ag…

## [s5] The sandbox scores a C body of func_8004A76C with its gte_rtpt() retained under the 2026-10-15 pinned gtemacro set.
- verdict: KILLED (instance) — Absent in both outputs; rtpt stripped, and probe B's local multi-insn ldv3c/stsxy3c stripped too.

## [s6] KILL RE-AUDIT — the closest banked form (rejected/pipelined-goto-wrong-abi-stripped-gte.c) scores differently on the current chassis.
- verdict: KILLED (instance) — score 30, 5 source-level / 0 operand-only / 0 not-scored hunks — identical to s5. Kill stands.

## [s6] The sandbox residual of the pipelined form is an RA/SCHED residual the solver models can search.
- verdict: KILLED (instance) — FIRST DIVERGENCE: PRE-RA, "no backend" — the instruction multiset differs because the scorer strips every lwc2/swc2/rtpt asm unit. The RA/sched solvers cannot…

## [s6] Freeing $a3 (vertex pointer off the arg registers) lets GCC place the mfc2 temps in $t0-$t2.
- verdict: KILLED (instance) — z temps got $7,$8,$9 (greg "76 in 8 77 in 9 78 in 3 82 in 2" with $2 taken by a `move $2,$4` copy the post-decrement test created and $3 by the vertex pointer)…

## [s6] The closest banked form (rejected/pipelined-goto-wrong-abi-stripped-gte.c) scores differently on the current chassis than its s5 kill recorded.
- verdict: KILLED (instance) — score 30, 5 source-level hunks, same as s5; kill stands

## [s6] The sandbox residual of the pipelined goto form is an RA or scheduler residual the ra_solver/sched_solver models can search.
- verdict: KILLED (instance) — FIRST DIVERGENCE: PRE-RA, no backend; the scorer strips the lwc2/swc2/rtpt asm, so the instruction multiset differs

## [s6] Moving the vertex pointer off $a3 (file-scope pointer diagnostic) puts the mfc2 temps in $t0-$t2.
- verdict: KILLED (instance) — temps got $7,$8,$9; $2 was taken by a move copy from the post-decrement test and $3 by the vertex pointer

## [s7] KILL RE-AUDIT: rejected/pipelined-goto-wrong-abi-stripped-gte.c on the current stub chassis.
- verdict: KILLED (instance) — score 30, 5 source-level / 0 operand-only / 0 not-scored hunks, identical to s5/s6. The kill stands (the sandbox still strips every lwc2/swc2/rtpt unit, so the…

## [s7] A plain (non-volatile) store tail leaves the `jr ra` delay slot as nop, as in the target (sh; sh; sh; jr ra; nop).
- verdict: KILLED (instance) — every plain-store form ends `sh; sh; j $31; sh`. With a volatile-qualified sz the store stays out (`sh; sh; sh; j $31` + nop) and the loop is otherwise unchang…

## [s7] KILL RE-AUDIT: rejected/pipelined-goto-wrong-abi-stripped-gte.c scores differently on the current stub chassis than its s5/s6 kill recorded.
- verdict: KILLED (instance) — Score 30, 5 source-level / 0 operand-only / 0 not-scored hunks, identical to s5/s6. Kill stands.

## [s7] A plain (non-volatile) store tail leaves the jr ra delay slot as nop, as in the target's 'sh; sh; sh; jr ra; nop'.
- verdict: KILLED (instance) — Every plain-store form ends 'sh; sh; j $31; [sh]'. Only volatile-qualified sz left the slot empty -- volatile coercion of an ordinary RAM buffer, outside the v…

## [s8] An independent tool (m2c) run fresh on this function's asm produces a C signature that cannot place the asm's real third input ($s0) as a formal parameter, reproducing the s1 ABI finding via a different method.
- verdict: KILLED (instance) — m2c emits 'void func_8004A76C(s32 arg0, void *arg2)' -- two params only; $s0 never appears as a declared variable, only inside opaque M2C_ERROR GTE stubs. All…

## [s8] The 461-entry gcc2.7.2-psx decomp.me corpus cached under tmp/decomp_me_corpus/ contains a GTE-pipelined scratch close enough to transplant a structurally different C shape from.
- verdict: KILLED (instance) — best similarity 0.011 against RecordButtonsToDevkit (unrelated devkit I/O), no GTE-shaped scratch anywhere in top 10 -- null result.

## [s8] func_8004A4E0's candidate.c or the completed puts module contains a transplantable code block for this function (mandatory sibling-ledger check).
- verdict: KILLED (instance) — func_8004A4E0's candidate.c is comments-only over its own unmodified INCLUDE_ASM stub (zero C statements). puts.c is an unrelated libc routine sharing no code…

## [s9] A header-exact PsyQ inline GTE macro writes GTE regs 17-19 into $8-$10 or clobbers $2/$3, giving the target's $t0-$t2 mfc2 seats without a pin
- verdict: KILLED (instance) — only mfc2 $12,$9 / $13,$10 (GTE IR regs into $t4/$t5); no macro moves GTE 17-19 into GPRs; inline_o clobbers are $12-$15 + memory only

## [s9] An explicit trailing return after the final sz stores keeps a return label before return_internal and leaves the jr ra slot empty
- verdict: KILLED (instance) — tail identical to p_f (j $31 with sh in slot): the jump to return_label is jump-to-next, deleted with its label

## [s9] An in-loop return (if (n == 0) { stores; return; } inside for(;;)) empties the jr slot while keeping the target layout
- verdict: KILLED (instance) — slot empty (mechanism confirmed, code_label 107 directly before return_internal) but the exit arm is inline after bne with an extra j .L1 + slot, a second sz p…

## [s10] The residuals of the closest banked forms (s7 p_f, s9 q_c) are a divergence of the decompals port cc1 from the original PsyQ cc1psx, so cc1psx would emit the target's $t0-$t2 seats / empty jr slot from the same C.
- verdict: KILLED (instance) — both bodies byte-identical to the port's output apart from label spelling: p_f.psx.s keeps mfc2 $2/$3/$8, `beq $4,$0` with `addu $4,$4,-1` in the slot, sh $8 i…

## [s10] PASS-INPUT ENUMERATION for the mfc2 seats: some C-side input shape of the z temporaries (narrow type, declaration order, array, fused asm, sz element type) makes global.c skip $2/$3 and seat the three mfc2 results in $8…
- verdict: KILLED (instance) — v1/v2 collapse to mfc2 $2,$2,$2 (each narrowed temp dies at its own sh); v4 gives $3,$2,$2; v3/v5/v6 give $2,$3,$8. None gives $8,$9,$10; port == cc1psx on all…

## [s10] The residuals of the closest banked forms (s7 p_f, s9 q_c) are a divergence of the decompals port cc1 from original cc1psx, so cc1psx would emit the target $t0-$t2 seats or empty jr slot from the same C.
- verdict: KILLED (instance) — Byte-identical apart from $L/.L label spelling: mfc2 $2/$3/$8, beq slot holds n--, jr slot holds sh $8 (p_f); q_c identical too.

## [s10] One of six C-side input shapes of the z temporaries (s16, u16, reversed declaration, z[3] array, fused three-output mfc2 asm, s32 sz) makes global.c seat the mfc2 results in $8/$9/$10.
- verdict: KILLED (instance) — s16/u16 -> $2,$2,$2; z[3] -> $3,$2,$2; reversed/fused/s32-sz -> $2,$3,$8; none $8,$9,$10; port == cc1psx on all six.

## [s11] A fresh, independent m2c run on asm/funcs/func_8004A76C.s (distinct process from s8's cached invocation) emits a different two-parameter signature or gives $s0 a formal-parameter slot, opening a new ABI-honest C path.
- verdict: KILLED (instance) — Identical to s8: void func_8004A76C(s32 arg0, void *arg2), no $s0 parameter, every cop2/GTE opcode (lwc2, rtpt, mfc2 $17-19, swc2) lowered to M2C_ERROR.

## [s11] The closest banked rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) was killed by a FAKE-propped chassis, so re-measuring it clean on the current chassis would change its verdict.
- verdict: KILLED (instance) — fake_ablate reports zero FAKE constructs to ablate. --diff on current chassis: 5/5 hunks source-level — the sandbox strips the whole unpinned GTE block, same e…

## [s12] (rederive) A third fresh m2c run, plus a freshness re-check of sibling func_8004A4E0's candidate.c (moved to floor 163/23 sessions since s8 last read it at floor ~unspecified), opens a new C/transplant path.
- verdict: KILLED (instance) — m2c output byte-identical in structure to s8/s11 — `void func_8004A76C(s32 arg0, void *arg2)`, no $s0 formal param, every cop2 op (lwc2 x6, rtpt, mfc2 $17-19,…

## [s12] A third fresh m2c run on asm/funcs/func_8004A76C.s (distinct process from s8/s11) emits a different two-parameter signature or gives $s0 a formal-parameter slot, opening a new ABI-honest C path.
- verdict: KILLED (instance) — Byte-identical in structure to s8/s11: void func_8004A76C(s32 arg0, void *arg2), no $s0 formal param, every cop2/GTE opcode (lwc2 x6, rtpt, mfc2 $17-19, swc2 x…

## [s12] Sibling func_8004A4E0's candidate.c, re-read at its current state (23 sessions and a floor move to 163 since s8 last checked it), now contains a C body or block worth transplanting onto func_8004A76C's chassis.
- verdict: KILLED (instance) — func_8004A4E0/candidate.c is still a pure INCLUDE_ASM comment banner (ASM-WHOLE canonical GTE grant, owner ruling Q40) -- zero C bytes, nothing structurally pr…

## Latest session s13 (verbatim)

## [s13] Mandated kill re-audit holds on current chassis: the closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) has zero FAKE constructs and still scores 5/5 source-level hunks, all from the sandbox stripping the unpinned lwc2/swc2/mfc2/c2 GTE opcodes wholesale.
- probe: python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c; then sandbox func_8004A76C --disable all --candidate <same file> --diff
- result: fake_ablate: "no FAKE-annotated constructs found ... nothing to ablate" (re-confirms s9/s11). --diff: target 37 insns / ours 16 insns / 5 hunks, 5 source-level, 0 operand-only, 0 not-scored. All five hunks are the lwc2 vertex loads, the c2 rtpt opcode, the swc2 stores, and the mfc2 reads -- i.e. exactly the GTE macro block engine/gtemacro.py strips because gte_rtpt/gte_ldv3c/gte_stsxy3c (and the raw c2 .word) are not PINNED units. No new residual; same operator-only engine-pin gap as s5/s7/s9/s10/s11.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct present

## [s13] This session's mandated modality is structural (block-local var splits, declaration order, type narrowing, statement re-association). That axis is already exhaustively covered for the one place it could bite -- the z1/z2/z3 mfc2-result temporaries -- by s10's six-variant sweep (v1 s16, v2 u16, v3 reversed declaration order, v4 s32 z[3] array, v5 fused one-asm three-output, v6 s32 *sz with s16 arithmetic): none of the six reach the target's $t0-$t2 seats, and port cc1 == cc1psx on all six. A fresh s16-narrowing attempt this session (tmp/grind/func_8004A76C/s13/v1_s16_z.c) is byte-identical to s10's v1 and was not separately measured to avoid duplicating a already-exhausted probe.
- probe: Read s10's hypotheses.md entries (lines 542-574) and rejected/psyq-rtpt-pair-reorg-slot-residual.c; diffed the planned s13 type-narrowing variant against s10's v1 by hand -- identical construct (s32->s16 on z1,z2,z3).
- result: no untried structural lever remains on the GPR-skeleton side of this chassis; the only scored divergence (all 5 hunks, s13 above) is the unpinned-GTE-macro strip, which is an engine/gtemacro.py pin gap, not a C-shape question any structural technique (var split, decl order, narrowing, re-association) can move.
- verdict: KILLED
- kill_scope: instance
- measured_on: s10's six-variant sweep (s10/v1_s16..v6_szs32.c) re-read this session, no FAKE construct, same $a3 chassis

## [s13] The closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) is free of FAKE constructs and still scores 5/5 source-level hunks on the current chassis, all from the sandbox stripping the unpinned lwc2/c2-rtpt/swc2/mfc2 GTE opcodes wholesale.
- mechanism: engine/gtemacro.py strips any gte_rtpt/gte_ldv3c/gte_stsxy3c (or raw c2 .word) block that is not in its PINNED-units table; the strip removes the whole block, which is why every scored hunk is source-level rather than operand-only.
- probe: python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c; then sandbox func_8004A76C --disable all --candidate <same file> --diff
- result: fake_ablate: zero FAKE constructs to ablate. --diff: target 37 insns / ours 16 insns / 5 hunks, 5 source-level, 0 operand-only, 0 not-scored -- all five are the stripped lwc2/c2/swc2/mfc2 GTE opcode block. Identical conclusion to s9/s11.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct present

## [s13] This session's mandated structural axis (block-local var splits, declaration order, type narrowing, statement re-association) has no untried probe left on this chassis: the one place it could move the scored residual -- the z1/z2/z3 mfc2-result temporaries -- was already exhaustively swept by s10's six variants (s16, u16, reversed declaration order, s32 z[3] array, fused one-asm three-output, s32 *sz with s16 arithmetic), none of which reach the target's $t0-$t2 seats.
- mechanism: global.c first-fit allocno seating depends on the live-range shape at the mfc2 call sites, which s10 already varied across six independent C-side input shapes with both port cc1 and cc1psx; a fresh structural permutation this session (s16-narrowing) was hand-diffed against s10's v1 and found byte-identical in construct, so it was not separately re-measured.
- probe: Read memory/grind/func_8004A76C/hypotheses.md s10 entries (six-variant sweep) and rejected/psyq-rtpt-pair-reorg-slot-residual.c; compared the planned s13 structural variant (tmp/grind/func_8004A76C/s13/v1_s16_z.c) against s10's v1 by hand.
- result: s13's planned type-narrowing variant is construct-identical to s10's already-killed v1; no other structural lever (decl order, block scoping, re-association) has a live register-seat question left to attack, since the scored residual is entirely the unpinned-GTE-macro strip, not a GPR-skeleton allocation question.
- verdict: KILLED
- kill_scope: instance
- measured_on: s10's six-variant sweep (s10/v1_s16..v6_szs32.c) re-read this session, no FAKE construct, same $a3 chassis

## [s14] Mandated kill re-audit (floor flat 3+ sessions): re-ablate the closest rejected
form and re-confirm the structural-axis exhaustion finding of s13 holds unchanged.
- mechanism: same as s13 — engine/gtemacro.py strips any gte_rtpt/gte_ldv3c/gte_stsxy3c
  (or raw c2 .word) block not in its PINNED-units table, so the entire scored residual is
  an unpinned-GTE-macro-strip artifact, not a GPR-skeleton allocation question any
  structural technique (var split, decl order, narrowing, re-association) can move.
- probe: python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate
  memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c; sandbox
  func_8004A76C --disable all --candidate memory/grind/func_8004A76C/candidate.c --diff
  (current chassis: the INCLUDE_ASM stub is the live candidate.c).
- result: fake_ablate confirms zero FAKE-annotated constructs in the closest rejected
  form (same as s13's reading — nothing to ablate, so the re-audit cannot change the
  verdict). sandbox --diff on the current candidate.c (the stub) errors
  "func_8004A76C not found in tmp/sandbox/.../3AB48.o" — the cheat-disabled build drops
  the symbol entirely once the GTE asm block is stripped, confirming the function cannot
  even be SCORED past the unpinned-macro gap on the current stub, let alone moved by a
  structural C lever. No new structural probe exists beyond s10's six-variant sweep
  (already re-confirmed construct-identical and exhausted at s13).
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD (fake_ablate
  re-run, zero FAKE constructs) + candidate.c stub (sandbox --diff error), $a3
  vertex-pointer chassis, no FAKE construct present in either file

## [s14] The mandated kill re-audit (floor flat 3+ sessions) on the closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c), re-run this session with fake_ablate and a fresh sandbox --diff attempt on the current candidate.c, produces no change from s13's reading: zero FAKE constructs to ablate, and the current candidate.c (INCLUDE_ASM stub) cannot even be diffed because the cheat-disabled build drops func_8004A76C entirely once the unpinned GTE macro block is stripped.
- mechanism: engine/gtemacro.py strips any gte_rtpt/gte_ldv3c/gte_stsxy3c (or raw c2 .word) block not in its PINNED-units table; on the stub this removes the whole scorable body, so the sandbox's cheat-disabled .o never contains the symbol.
- probe: python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c; then tools/wteng.ps1 main sandbox func_8004A76C --disable all --candidate memory/grind/func_8004A76C/candidate.c --diff
- result: fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'. sandbox --diff: error 'func_8004A76C not found in tmp/sandbox/func_8004A76C/main/3AB48.o' (pipeline truncated after cheat-asm strip). Both results are identical in substance to s13's banked conclusion; the re-audit changes nothing.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD (fake_ablate re-run) + memory/grind/func_8004A76C/candidate.c stub (sandbox --diff this session), $a3 vertex-pointer chassis, no FAKE construct present in either file

## [s14] No untried structural lever (block-local var split, declaration order, type narrowing, statement re-association) exists this session beyond s10's six-variant mfc2-seat sweep, which s13 already re-confirmed is construct-identical and exhausted.
- mechanism: The only place a structural lever could move the scored residual -- the z1/z2/z3 mfc2-result temporaries feeding $t0-$t2 allocno seats via global.c first-fit -- was already exhaustively swept across six independent C-side shapes at s10, with no match; the rest of the scored diff is the unpinned-GTE-macro strip, which is an engine pin gap, not a C-shape question.
- probe: Re-read s10's six-variant sweep entries and s13's structural-exhaustion entry in hypotheses.md; cross-checked against this session's fake_ablate + sandbox --diff re-measurement (hypothesis above) to confirm the chassis has not moved.
- result: No new structural variant distinguishable from s10's set; the scored gap remains entirely attributable to the unpinned GTE macro strip, confirmed again this session.
- verdict: KILLED
- kill_scope: instance
- measured_on: s10's six-variant sweep (s10/v1_s16..v6_szs32.c) and s13's exhaustion entry, re-read this session; same $a3 chassis; no FAKE construct

## [s15] ENUMERATION (mandated `enumerate` modality): systematic spelling sweep of the mfc2/sh/branch skeleton that survives gte_rtpt/ldv3c/stsxy3c macro stripping (the region of hunks 3-5 in the s7 rejected form's --diff) finds zero improvement across every distinct statement-order spelling of that region.
- mechanism: tools/spelling_enum.py enumerates every byte-preserving reordering (anchor-swap + commutative-swap axes) of a marked C region. Ran it on the `mid:` block of rejected/psyq-rtpt-pair-reorg-slot-residual.c (gte_stsxy3c call, sxy+=3, the three gte_mfc2 reads, and the if(n!=0){...goto loop} re-entry) -- the one place the surviving (non-macro) skeleton has any statement-order freedom. 8 anchors, 3 assignments, 0 named locals -> 6 distinct spellings with `--no-swaps`; re-running WITH the commutative-swap axis enabled produced the identical 6 (no swappable commutative pairs in this region), so the exhaustive count for this block is 6, not 6x2.
- probe: `python3 tools/spelling_enum.py --candidate tmp/grind/func_8004A76C/s15/enum_src.c --out tmp/grind/func_8004A76C/s15/enum[_swap] [--no-swaps]` (6 variants each path) then `tools/wteng.ps1 main sandbox func_8004A76C --disable all --candidate tmp/grind/func_8004A76C/s15/enum/v{0..5}.c` for each.
- result: ENUMERATION: 6 spellings, best 32, 6 at the floor. All six variants score exactly 32 -- identical to the un-enumerated rejected form and to s10's six independently-authored structural variants. Zero hit, zero movement. This is consistent with (not merely repeating) the s7/s9/s11/s13/s14 finding: the scored residual is dominated by the wholesale-stripped lwc2/c2-rtpt/swc2 macro blocks (hunks 1/2/4, categorically absent from the cheat-stripped build regardless of C spelling), and the register-seat choice in the surviving skeleton ($v0/$v1/$a1 ours vs $t0-$t2 target) is a side effect of global.c first-fit allocating over the SHRUNK live-range set once those macros are gone, not a statement-order question this skeleton region can expose. CLASS KILL for this region's spelling space: no reordering of the mfc2/sh/branch skeleton, with or without the commutative-swap axis, moves the score.
- verdict: KILLED
- kill_scope: class
- measured_on: tmp/grind/func_8004A76C/s15/enum_src.c (fresh ENUM-markup of the s7 rejected form's `mid:` block), all 6 v0-v5 variants, --no-swaps and swap-axis both producing the same 6-element space, no FAKE construct in any variant
- predicate_cite: engine/gtemacro.py:82 (the PINNED table) + engine/gtemacro.py:472 (the strip-all-unpinned-macro loop) — the name-pattern-matched strip runs regardless of surrounding C statement order, which is why the enumerator's exhaustive 6-member spelling space of the surviving skeleton cannot reach hunks 1/2/4 no matter which of the 6 it picks; tools/spelling_enum.py's anchor/swap enumeration (which produced the 6-member count) is the exhaustiveness proof for the region itself.

## [s15] An exhaustive statement-order enumeration (tools/spelling_enum.py) of the mfc2/sh/branch skeleton that survives GTE macro stripping (the `mid:` block of rejected/psyq-rtpt-pair-reorg-slot-residual.c) can reach the target's $t0-$t2 register seats / beqz branch sense.
- mechanism: global.c first-fit allocno seating depends on live-range shape at the mfc2 call sites; a statement reorder of the skeleton changes live-range order and could in principle re-seat the allocation.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_8004A76C/s15/enum_src.c --out tmp/grind/func_8004A76C/s15/enum[_swap] [--no-swaps] (6 variants, no-swaps and swap-axis identical); tools/wteng.ps1 main sandbox func_8004A76C --disable all --candidate <each variant>
- result: 6 distinct spellings (8 anchors, 3 assignments, 0 named locals; commutative-swap axis adds nothing). All 6 score 32 -- identical to the un-enumerated form and to s10's independently-authored six structural variants. ENUMERATION: 6 spellings, best 32, 6 at the floor.
- verdict: KILLED
- kill_scope: class
- measured_on: tmp/grind/func_8004A76C/s15/enum_src.c and its 6 v0-v5 variants (both --no-swaps and swap-axis enumerations), no FAKE construct in any variant
- predicate_cite: engine/gtemacro.py:82 (PINNED table) + engine/gtemacro.py:472 (strip-all-unpinned-macro loop) -- the name-pattern strip of hunks 1/2/4's lwc2/c2-rtpt/swc2 blocks runs regardless of the surrounding C statement order, so no member of the enumerator's exhaustive 6-spelling space for this region (tools/spelling_enum.py's anchor/swap enumeration) can reach those hunks; the sweep's zero-hit result is exhaustive over the region, not a sample

## [s16] (mandated `enumerate` modality, repeat dispatch) Widening the spelling_enum region beyond s15's `mid:` block -- to the prologue block (n--/gte_ldv3c/gte_rtpt_nop/v+=3/goto mid), the `loop:` block (gte_rtpt/sz+=3/sz[-3..-1]=z1..z3), and the epilogue block (sz[0..2]=z1..z3) -- opens a new reorderable spelling space the tool has not yet swept.
- mechanism: tools/spelling_enum.py enumerates reorderings only among statements it recognizes as movable (scalar-local assignments feeding its "named locals"/"assignments" axes); everything else (macro calls, pointer/array stores, decrements, goto/labels) is an anchor with a fixed position.
- probe: Built tmp/grind/func_8004A76C/s16/enum_src.c (ENUM-markup around the prologue+loop block) and enum_src2.c (ENUM-markup around the epilogue stores), based on rejected/psyq-rtpt-pair-reorg-slot-residual.c. Ran `python3 tools/spelling_enum.py --candidate <file> --out tmp/grind/func_8004A76C/s16/enum[2][_swap] --list` (and the non-swap variant) on each.
- result: Prologue+loop region: "0 named locals (), 0 assignments, 11 anchors -> 1 distinct spelling" (identical with and without the commutative-swap axis). Epilogue region: "0 named locals (), 0 assignments, 3 anchors -> 1 distinct spelling". Neither region contains a single scalar-local assignment statement -- every statement in them is a macro call, pointer/array store, decrement, or control-flow anchor -- so the enumerator's model has literally nothing to permute: the spelling space size is 1 in both, i.e. there is no sweep to run. The ONLY region in the whole function body with size > 1 is s15's `mid:` block (the three `z1/z2/z3 = gte_mfc2(...)` scalar assignments), already exhaustively swept there (6 spellings, zero hit, class-killed). This session's widening therefore does not open a new spelling axis; it confirms none exists outside the one s15 already covered.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s16/enum_src.c and enum_src2.c (fresh ENUM-markup of the prologue/loop and epilogue blocks of rejected/psyq-rtpt-pair-reorg-slot-residual.c), spelling_enum.py --list output both --no-swaps and swap-axis for each region, no FAKE construct in either file. (Downgraded from an earlier class-kill framing: a degenerate 1-member enumeration is an empty search result on THIS chassis/region pair, not a predicate-cited class proof -- recorded as instance per the driver's validator guidance.)

## [s16] Widening tools/spelling_enum.py's ENUM region beyond s15's `mid:` block to the prologue+loop block and the epilogue block (of rejected/psyq-rtpt-pair-reorg-slot-residual.c) opens a new reorderable spelling space that could move the score on this chassis.
- mechanism: spelling_enum.py enumerates reorderings only among statements it recognizes as movable (scalar-local assignments feeding its named-locals/assignments axes); everything else (macro calls, pointer/array stores, decrements, goto/labels) is a fixed-position anchor.
- probe: Built tmp/grind/func_8004A76C/s16/enum_src.c (prologue+loop ENUM-markup) and enum_src2.c (epilogue ENUM-markup); ran python3 tools/spelling_enum.py --candidate <file> --out tmp/grind/func_8004A76C/s16/enum[2][_swap] --list, both --no-swaps and swap-axis, on each.
- result: Prologue+loop region: 0 named locals, 0 assignments, 11 anchors -> 1 distinct spelling (identical with/without swap axis). Epilogue region: 0 named locals, 0 assignments, 3 anchors -> 1 distinct spelling. Neither region contains a scalar-local assignment statement, so the enumerator's model has nothing to permute -- the spelling space size is 1 in both, i.e. there is no sweep to run on this chassis. The only region with size > 1 remains s15's `mid:` block (already swept there, 6 spellings, zero hit).
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s16/enum_src.c and enum_src2.c (fresh ENUM-markup of the prologue/loop and epilogue blocks of rejected/psyq-rtpt-pair-reorg-slot-residual.c), spelling_enum.py --list output both --no-swaps and swap-axis for each region, no FAKE construct in either file. This is an empty-search result on this chassis/region pair (degenerate 1-member enumeration), not a predicate-cited proof that no chassis could ever expose freedom here, so it is recorded as an instance kill (correcting the prior session's over-broad class-kill framing per the driver's discard guidance).

## [s17] (mandated `enumerate` modality, repeat dispatch; KILL RE-AUDIT per the floor-flat-3-sessions gate) The s13/s14 structural-exhaustion finding and the s9/s10/s13/s14 engine-pin-gap finding both hold unchanged on today's chassis: rejected/psyq-rtpt-pair-reorg-slot-residual.c carries no FAKE construct and still scores 32/37 with 5/5 source-level hunks, identical in shape to every prior re-audit.
- mechanism: `tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate rejected/psyq-rtpt-pair-reorg-slot-residual.c` reports "no FAKE-annotated constructs found; nothing to ablate" (the kill was never FAKE-propped, confirming s9/s14's prior finding). `sandbox --disable all --diff --candidate` on the current INCLUDE_ASM-stub chassis reproduces score 32, target_insns 37, build_insns 16, 5/5 hunks source-level, 0 operand-only, 0 not-scored. Hunks 1/2/4 are the unpinned gte_rtpt/gte_ldv3c/gte_stsxy3c macro-strip gap (engine/gtemacro.py:82 PINNED table + :472 strip-all-unpinned loop); hunk 3's mfc2 seats land at $2(v0)/$3(v1)/$a1 on this re-run rather than the $t0-$t2 wording in older s6/s10 entries — same global.c:1057 first-fit mechanism, different concrete allocno names across cc1 invocations, not a new residual class (the seat identity is masked by the score regardless).
- probe: Ran fake_ablate.py and `sandbox func_8004A76C --disable all --diff --candidate memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c` fresh this session (output saved tmp/grind/func_8004A76C/s17/diff_reaudit.txt). Also confirmed the plain INCLUDE_ASM-stub measurement is still 37 (no_c_body=true).
- result: No drift from s13/s14's banked re-audits. The enumerate-modality search space (s15's `mid:` 6-member sweep, s16's degenerate prologue/loop/epilogue regions) is unchanged and remains the only systematic-spelling axis this chassis exposes; it was already fully swept at s15/s16 with zero hit. No new probe is available under `enumerate` this session beyond this re-audit.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct present (fake_ablate.py confirmed empty this session)

## [s17] The s13/s14 structural-exhaustion finding and the s9/s10/s13/s14 engine-pin-gap finding both hold unchanged on today's chassis: rejected/psyq-rtpt-pair-reorg-slot-residual.c carries no FAKE construct and still scores 32/37 with 5/5 source-level hunks, identical in shape to every prior re-audit.
- mechanism: fake_ablate.py finds nothing to ablate (kill never FAKE-propped); sandbox --diff reproduces the unpinned gte_rtpt/gte_ldv3c/gte_stsxy3c macro-strip (engine/gtemacro.py:82 PINNED table + :472 strip-all-unpinned loop) as hunks 1/2/4, and the global.c:1057 first-fit mfc2-seat mismatch as hunk 3 (seats now named $2/$3/$a1 rather than the older $t0-$t2 wording -- same mechanism, different concrete allocno names across cc1 invocations, not a new residual).
- probe: python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate memory/grind/func_8004A76C/rejected/psyq-rtpt-pair-reorg-slot-residual.c; sandbox func_8004A76C --disable all --diff --candidate <same file> (output saved tmp/grind/func_8004A76C/s17/diff_reaudit.txt)
- result: No drift from s13/s14's banked re-audits. Re-read s15/s16 in full: the spelling_enum search space (mid: block size 6 swept zero-hit; prologue/loop/epilogue degenerate size-1) is still fully mapped with nothing left to widen -- every remaining statement in the rejected form is a macro call, pointer/array store, decrement, or goto/label, all fixed-position anchors in spelling_enum.py's model.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at current HEAD, $a3 vertex-pointer chassis, PsyQ gte_rtpt pair + local ldv3c/stsxy3c asm, no FAKE construct present (fake_ablate.py confirmed empty this session)

## [s18] (synthesis) KILL RE-AUDIT: the two closest banked forms (rejected/psyq-rtpt-pair-reorg-slot-residual.c, rejected/pipelined-goto-wrong-abi-stripped-gte.c) score differently on the current chassis or carried a FAKE carrier that voids their kills.
- probe: python3 tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate <each>; sandbox func_8004A76C --disable all --diff --candidate <each>; sandbox stub.
- result: zero FAKE constructs in either; scores 32 and 30, both 5 source-level / 0 operand-only / 0 not-scored; stub 37. Artifacts tmp/grind/func_8004A76C/s18/ablate_*.txt, sandbox_*.txt. Kills stand.
- verdict: KILLED
- kill_scope: instance
- measured_on: both rejected forms at current HEAD, $a3 vertex-pointer chassis, no FAKE construct present

## [s18] (synthesis) If the $s0 vertex-pointer ABI were granted, GCC 2.7.2 would emit the remaining target bytes from the closest banked body (the ABI is the sole unreachable feature).
- mechanism: diagnostic only -- a file-scope `register SVECTOR *v asm("$16")` (a register pin, completion-bar wall; never shippable) removes the vertex pointer from the arg registers and gives it $16 without a prologue save, isolating the non-ABI residuals.
- probe: tmp/grind/func_8004A76C/s18/g_s0.c (s7 p_f body + global register var, 3-arg signature) through tools/gcc-2.7.2/build/cc1 -O2 -da (cc.sh) and tools/cc1psx_wrapper.sh (psx.sh).
- result: port == cc1psx byte-for-byte. Matches CFG, GTE ops, nop pad, $16 base/advance, beqz slot. Three residuals remain: mfc2 seats $2/$3/$7 (greg 75->2, 76->3, 77->7) vs $t0-$t2; addu a1,12 before the mfc2s vs after; jr slot filled with sh $7 vs nop. Banked rejected/diag-global-register-s0-wall-seats-v0-v1-a3-jrslot.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s18/g_s0.c, global register variable $16 diagnostic (wall-banned pin, no FAKE construct), port cc1 and cc1psx

## [s18] The two closest banked forms (rejected/psyq-rtpt-pair-reorg-slot-residual.c, rejected/pipelined-goto-wrong-abi-stripped-gte.c) score differently on the current chassis or carried a FAKE carrier that voids their kills.
- mechanism: Mandated kill re-audit: a kill measured under a FAKE carrier or older chassis is re-testable.
- probe: tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate <each>; sandbox func_8004A76C --disable all --diff --candidate <each>; sandbox stub.
- result: Zero FAKE constructs in either form. Scores 32 and 30, each 5 source-level / 0 operand-only / 0 not-scored hunks; stub 37. Identical to s5/s7/s17.
- verdict: KILLED
- kill_scope: instance
- measured_on: both rejected forms at current HEAD, $a3 vertex-pointer chassis, no FAKE construct present

## [s18] With the $s0 vertex-pointer ABI granted (diagnostic file-scope register variable in $16), GCC 2.7.2 emits the remaining target bytes from the closest banked body, making the ABI the only residual.
- mechanism: Diagnostic only: a global register variable (wall-banned pin, never shippable) removes the vertex pointer from the arg registers and seats it in $16 without a prologue save, isolating the non-ABI residuals.
- probe: tmp/grind/func_8004A76C/s18/g_s0.c (s7 p_f body + register SVECTOR *v asm("$16"), 3-arg signature) via port cc1 -O2 -da and original cc1psx.
- result: Port == cc1psx byte-for-byte. CFG, GTE ops, entry nop pad, $16 base and advance, j-into-middle, beqz slot all match. Three residuals remain: mfc2 seats $2/$3/$7 (greg 75->2 76->3 77->7) vs target $t0/$t1/$t2; addu a1,12 before the mfc2s vs after; jr slot holds sh $7 vs nop.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_8004A76C/s18/g_s0.c, global register variable $16 diagnostic (wall-banned pin, no FAKE construct), port cc1 and cc1psx
