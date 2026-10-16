# Hypothesis ledger — func_8004A76C

> COMPACTED 2026-10-15 by `grindlib.py compact-ledger` (65,634 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 1188fb1ef:memory/grind/func_8004A76C/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
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
floor 37 at s20; 20 sessions; live frontier (state.json is authoritative):
  - The whole-body canonical-asm grant at inline_asm_canonical.txt:25 is the correct disposition.
  - A deeper inverse search (depth 4+) finds a global.c input vector giving z1-z3 $t0-$t2.
  - Pinning gte_rtpt/ldv3c/stsxy3c in engine/gtemacro.py lets the sandbox score the GTE body.

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

## [s22] (rederive, mandated modality) A fresh decomp.me similarity search against the now-461-scratch corpus (grown from 316 at func_8004A4E0's s12) surfaces a closer match than s8/s11/s12's null result, or m2c emits a materially different skeleton on re-run.
- mechanism: tools/decomp_me_scrape.py search re-shingles asm/funcs/func_8004A76C.s against every cached gcc2.7.2-psx scratch; a larger corpus could in principle contain a GTE-shaped scratch added since s8.
- probe: re-ran `python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8004A76C.s --corpus tmp/decomp_me_corpus --top 10` (tmp/grind/func_8004A76C/s22/decomp_me_search_461.txt) and diffed against s8's cached output. Byte-identical top-10 list (best similarity 0.011, RecordButtonsToDevkit, no GTE-shaped match) — the corpus directory holds exactly 461 scratches in both checks, so no new scratches were added between s8 and now; the growth seen in func_8004A4E0's s12 (316->461) predates this function's own s8 (461 already). Re-read tmp/grind/func_8004A76C/s12/m2c_out.c in full: m2c's output is structurally forced by the instruction set, not a search choice — every lwc2/rtpt/swc2/mfc2 cop2 op becomes M2C_ERROR regardless of corpus or flags (confirmed against tools/m2c/m2c_macros.h: the "valid syntax" unknown-instruction macros exist for arbitrary unhandled instructions in general, but cop2 GTE opcodes have no C semantic to lower to under any m2c flag — this is the same AGENTS.md inline-asm-policy fact, not a tool limitation to route around). Also re-confirmed the sibling func_8004A4E0's candidate.c (re-read in full, unchanged since its 2026-10-15 17:03 write, still comments-only INCLUDE_ASM with nothing to transplant onto this chassis) and re-measured this function's own stub: sandbox --disable all still scores 37 ({"score":37,"no_c_body":true,"build_insns":0}).
- result: no new rederive-axis signal. The decomp.me corpus did not grow between s8 and s22 (both 461), so the search result is not merely unchanged by luck — there was nothing new to find. m2c's cop2 M2C_ERROR output is a structural fact about GTE opcodes having no C lowering, independent of which m2c invocation or corpus state is used.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_8004A76C.s unchanged, decomp.me corpus at 461 scratches (unchanged from s8), no FAKE construct (comments-only stub, nothing to ablate)

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

## [s15] ENUMERATION (mandated `enumerate` modality): systematic spelling sweep of the mfc2/sh/branch skeleton that survives gte_rtpt/ldv3c/stsxy3c macro stripping (the region of hunks 3-5 in the s7 rejected form's --diff) finds zero improvement across every distinct statement-order spelling of that region.
- verdict: KILLED
- kill_scope: class
- predicate_cite: engine/gtemacro.py:82 (the PINNED table) + engine/gtemacro.py:472 (the strip-all-unpinned-macro loop) — the name-pattern-matched strip runs regardless of surro…
- result: ENUMERATION: 6 spellings, best 32, 6 at the floor. All six variants score exactly 32 -- identical to the un-enumerated rejected form and to s10's six independently-authored structural variants. Zero hit, zero movement. This is consistent with (not merely repeating) the s7/s9/s11/s13/s14 finding: the scored residual is dominated by the wholesale-stripped lwc2/c2-rtpt/swc2 macro blocks (hunks 1/2/4…

## [s15] An exhaustive statement-order enumeration (tools/spelling_enum.py) of the mfc2/sh/branch skeleton that survives GTE macro stripping (the `mid:` block of rejected/psyq-rtpt-pair-reorg-slot-residual.c) can reach the target's $t0-$t2 register seats / beqz branch sense.
- verdict: KILLED
- kill_scope: class
- predicate_cite: engine/gtemacro.py:82 (PINNED table) + engine/gtemacro.py:472 (strip-all-unpinned-macro loop) -- the name-pattern strip of hunks 1/2/4's lwc2/c2-rtpt/swc2 bloc…
- result: 6 distinct spellings (8 anchors, 3 assignments, 0 named locals; commutative-swap axis adds nothing). All 6 score 32 -- identical to the un-enumerated form and to s10's independently-authored six structural variants. ENUMERATION: 6 spellings, best 32, 6 at the floor.

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
- verdict: KILLED (instance)

## [s3] A decomp-permuter campaign can be constructed for func_8004A76C to search for a C-level match of the diverging region.
- verdict: KILLED (instance)

## [s4] enumerate — a systematic spelling sweep (`spelling_enum.py` / `sweep_variants.py`) over an `/* ENUM-BEGIN */.../* ENUM-END */` region can search the diverging block's C spelling space for func_8004A76C.
- verdict: KILLED (instance)

## [s4] A systematic spelling sweep (spelling_enum.py / sweep_variants.py over an /* ENUM-BEGIN */.../* ENUM-END */ region) can search func_8004A76C's diverging C spelling space for an exact-match form.
- verdict: KILLED (instance)

## [s5] KILL RE-AUDIT — the s2/s3/s4 instance kills could be void because their chassis or FAKE state changed.
- verdict: KILLED (instance)

## [s5] A natural honest GTE loop (gte.h macros, vertex pointer as 4th arg) approaches the target.
- verdict: KILLED (instance)

## [s5] The sandbox can score a C body of func_8004A76C that contains its rtpt.
- verdict: KILLED (instance)

## [s5] The s2/s3/s4 instance kills are void because their chassis or FAKE state changed since they were measured.
- verdict: KILLED (instance)

## [s5] A natural do/while GTE loop using gte.h macros with the vertex pointer as a 4th argument approaches the target bytes.
- verdict: KILLED (instance)

## [s5] The sandbox scores a C body of func_8004A76C with its gte_rtpt() retained under the 2026-10-15 pinned gtemacro set.
- verdict: KILLED (instance)

## [s6] KILL RE-AUDIT — the closest banked form (rejected/pipelined-goto-wrong-abi-stripped-gte.c) scores differently on the current chassis.
- verdict: KILLED (instance)

## [s6] The sandbox residual of the pipelined form is an RA/SCHED residual the solver models can search.
- verdict: KILLED (instance)

## [s6] Freeing $a3 (vertex pointer off the arg registers) lets GCC place the mfc2 temps in $t0-$t2.
- verdict: KILLED (instance)

## [s6] The closest banked form (rejected/pipelined-goto-wrong-abi-stripped-gte.c) scores differently on the current chassis than its s5 kill recorded.
- verdict: KILLED (instance)

## [s6] The sandbox residual of the pipelined goto form is an RA or scheduler residual the ra_solver/sched_solver models can search.
- verdict: KILLED (instance)

## [s6] Moving the vertex pointer off $a3 (file-scope pointer diagnostic) puts the mfc2 temps in $t0-$t2.
- verdict: KILLED (instance)

## [s7] KILL RE-AUDIT: rejected/pipelined-goto-wrong-abi-stripped-gte.c on the current stub chassis.
- verdict: KILLED (instance)

## [s7] A plain (non-volatile) store tail leaves the `jr ra` delay slot as nop, as in the target (sh; sh; sh; jr ra; nop).
- verdict: KILLED (instance)

## [s7] KILL RE-AUDIT: rejected/pipelined-goto-wrong-abi-stripped-gte.c scores differently on the current stub chassis than its s5/s6 kill recorded.
- verdict: KILLED (instance)

## [s7] A plain (non-volatile) store tail leaves the jr ra delay slot as nop, as in the target's 'sh; sh; sh; jr ra; nop'.
- verdict: KILLED (instance)

## [s8] An independent tool (m2c) run fresh on this function's asm produces a C signature that cannot place the asm's real third input ($s0) as a formal parameter, reproducing the s1 ABI finding via a different method.
- verdict: KILLED (instance)

## [s8] The 461-entry gcc2.7.2-psx decomp.me corpus cached under tmp/decomp_me_corpus/ contains a GTE-pipelined scratch close enough to transplant a structurally different C shape from.
- verdict: KILLED (instance)

## [s8] func_8004A4E0's candidate.c or the completed puts module contains a transplantable code block for this function (mandatory sibling-ledger check).
- verdict: KILLED (instance)

## [s9] A header-exact PsyQ inline GTE macro writes GTE regs 17-19 into $8-$10 or clobbers $2/$3, giving the target's $t0-$t2 mfc2 seats without a pin
- verdict: KILLED (instance)

## [s9] An explicit trailing return after the final sz stores keeps a return label before return_internal and leaves the jr ra slot empty
- verdict: KILLED (instance)

## [s9] An in-loop return (if (n == 0) { stores; return; } inside for(;;)) empties the jr slot while keeping the target layout
- verdict: KILLED (instance)

## [s10] The residuals of the closest banked forms (s7 p_f, s9 q_c) are a divergence of the decompals port cc1 from the original PsyQ cc1psx, so cc1psx would emit the target's $t0-$t2 seats / empty jr slot from the same C.
- verdict: KILLED (instance)

## [s10] PASS-INPUT ENUMERATION for the mfc2 seats: some C-side input shape of the z temporaries (narrow type, declaration order, array, fused asm, sz element type) makes global.c skip $2/$3 and seat the three mfc2 results in $8…
- verdict: KILLED (instance)

## [s10] The residuals of the closest banked forms (s7 p_f, s9 q_c) are a divergence of the decompals port cc1 from original cc1psx, so cc1psx would emit the target $t0-$t2 seats or empty jr slot from the same C.
- verdict: KILLED (instance)

## [s10] One of six C-side input shapes of the z temporaries (s16, u16, reversed declaration, z[3] array, fused three-output mfc2 asm, s32 sz) makes global.c seat the mfc2 results in $8/$9/$10.
- verdict: KILLED (instance)

## [s11] A fresh, independent m2c run on asm/funcs/func_8004A76C.s (distinct process from s8's cached invocation) emits a different two-parameter signature or gives $s0 a formal-parameter slot, opening a new ABI-honest C path.
- verdict: KILLED (instance)

## [s11] The closest banked rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) was killed by a FAKE-propped chassis, so re-measuring it clean on the current chassis would change its verdict.
- verdict: KILLED (instance)

## [s12] (rederive) A third fresh m2c run, plus a freshness re-check of sibling func_8004A4E0's candidate.c (moved to floor 163/23 sessions since s8 last read it at floor ~unspecified), opens a new C/transplant path.
- verdict: KILLED (instance)

## [s12] A third fresh m2c run on asm/funcs/func_8004A76C.s (distinct process from s8/s11) emits a different two-parameter signature or gives $s0 a formal-parameter slot, opening a new ABI-honest C path.
- verdict: KILLED (instance)

## [s12] Sibling func_8004A4E0's candidate.c, re-read at its current state (23 sessions and a floor move to 163 since s8 last checked it), now contains a C body or block worth transplanting onto func_8004A76C's chassis.
- verdict: KILLED (instance)

## [s13] Mandated kill re-audit holds on current chassis: the closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) has zero FAKE constructs and still scores 5/5 source-level hunks, all from the sandbox stripping…
- verdict: KILLED (instance) — fake_ablate: "no FAKE-annotated constructs found ... nothing to ablate" (re-confirms s9/s11). --diff: target 37 insns / ours 16 insns / 5 hunks, 5 source-level…

## [s13] This session's mandated modality is structural (block-local var splits, declaration order, type narrowing, statement re-association). That axis is already exhaustively covered for the one place it could bite -- the z1/z…
- verdict: KILLED (instance) — no untried structural lever remains on the GPR-skeleton side of this chassis; the only scored divergence (all 5 hunks, s13 above) is the unpinned-GTE-macro str…

## [s13] The closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) is free of FAKE constructs and still scores 5/5 source-level hunks on the current chassis, all from the sandbox stripping the unpinned lwc2/c2-rt…
- verdict: KILLED (instance) — fake_ablate: zero FAKE constructs to ablate. --diff: target 37 insns / ours 16 insns / 5 hunks, 5 source-level, 0 operand-only, 0 not-scored -- all five are th…

## [s13] This session's mandated structural axis (block-local var splits, declaration order, type narrowing, statement re-association) has no untried probe left on this chassis: the one place it could move the scored residual --…
- verdict: KILLED (instance) — s13's planned type-narrowing variant is construct-identical to s10's already-killed v1; no other structural lever (decl order, block scoping, re-association) h…

## [s14] Mandated kill re-audit (floor flat 3+ sessions): re-ablate the closest rejected form and re-confirm the structural-axis exhaustion finding of s13 holds unchanged.
- verdict: KILLED (instance) — fake_ablate confirms zero FAKE-annotated constructs in the closest rejected

## [s14] The mandated kill re-audit (floor flat 3+ sessions) on the closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c), re-run this session with fake_ablate and a fresh sandbox --diff attempt on the current ca…
- verdict: KILLED (instance) — fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'. sandbox --diff: error 'func_8004A76C not found in tmp/sandbox/func_8004A76C/main/3AB48…

## [s14] No untried structural lever (block-local var split, declaration order, type narrowing, statement re-association) exists this session beyond s10's six-variant mfc2-seat sweep, which s13 already re-confirmed is construct-…
- verdict: KILLED (instance) — No new structural variant distinguishable from s10's set; the scored gap remains entirely attributable to the unpinned GTE macro strip, confirmed again this se…

## [s16] (mandated `enumerate` modality, repeat dispatch) Widening the spelling_enum region beyond s15's `mid:` block -- to the prologue block (n--/gte_ldv3c/gte_rtpt_nop/v+=3/goto mid), the `loop:` block (gte_rtpt/sz+=3/sz[-3..…
- verdict: KILLED (instance) — Prologue+loop region: "0 named locals (), 0 assignments, 11 anchors -> 1 distinct spelling" (identical with and without the commutative-swap axis). Epilogue re…

## [s16] Widening tools/spelling_enum.py's ENUM region beyond s15's `mid:` block to the prologue+loop block and the epilogue block (of rejected/psyq-rtpt-pair-reorg-slot-residual.c) opens a new reorderable spelling space that co…
- verdict: KILLED (instance) — Prologue+loop region: 0 named locals, 0 assignments, 11 anchors -> 1 distinct spelling (identical with/without swap axis). Epilogue region: 0 named locals, 0 a…

## [s17] (mandated `enumerate` modality, repeat dispatch; KILL RE-AUDIT per the floor-flat-3-sessions gate) The s13/s14 structural-exhaustion finding and the s9/s10/s13/s14 engine-pin-gap finding both hold unchanged on today's c…
- verdict: KILLED (instance) — No drift from s13/s14's banked re-audits. The enumerate-modality search space (s15's `mid:` 6-member sweep, s16's degenerate prologue/loop/epilogue regions) is…

## [s17] The s13/s14 structural-exhaustion finding and the s9/s10/s13/s14 engine-pin-gap finding both hold unchanged on today's chassis: rejected/psyq-rtpt-pair-reorg-slot-residual.c carries no FAKE construct and still scores 32…
- verdict: KILLED (instance) — No drift from s13/s14's banked re-audits. Re-read s15/s16 in full: the spelling_enum search space (mid: block size 6 swept zero-hit; prologue/loop/epilogue deg…

## [s18] (synthesis) KILL RE-AUDIT: the two closest banked forms (rejected/psyq-rtpt-pair-reorg-slot-residual.c, rejected/pipelined-goto-wrong-abi-stripped-gte.c) score differently on the current chassis or carried a FAKE carrie…
- verdict: KILLED (instance) — zero FAKE constructs in either; scores 32 and 30, both 5 source-level / 0 operand-only / 0 not-scored; stub 37. Artifacts tmp/grind/func_8004A76C/s18/ablate_*.…

## [s18] (synthesis) If the $s0 vertex-pointer ABI were granted, GCC 2.7.2 would emit the remaining target bytes from the closest banked body (the ABI is the sole unreachable feature).
- verdict: KILLED (instance) — port == cc1psx byte-for-byte. Matches CFG, GTE ops, nop pad, $16 base/advance, beqz slot. Three residuals remain: mfc2 seats $2/$3/$7 (greg 75->2, 76->3, 77->7…

## [s18] The two closest banked forms (rejected/psyq-rtpt-pair-reorg-slot-residual.c, rejected/pipelined-goto-wrong-abi-stripped-gte.c) score differently on the current chassis or carried a FAKE carrier that voids their kills.
- verdict: KILLED (instance) — Zero FAKE constructs in either form. Scores 32 and 30, each 5 source-level / 0 operand-only / 0 not-scored hunks; stub 37. Identical to s5/s7/s17.

## [s18] With the $s0 vertex-pointer ABI granted (diagnostic file-scope register variable in $16), GCC 2.7.2 emits the remaining target bytes from the closest banked body, making the ABI the only residual.
- verdict: KILLED (instance) — Port == cc1psx byte-for-byte. CFG, GTE ops, entry nop pad, $16 base and advance, j-into-middle, beqz slot all match. Three residuals remain: mfc2 seats $2/$3/$…

## [s19] (solver) KILL RE-AUDIT: rejected/psyq-rtpt-pair-reorg-slot-residual.c drifted on the current chassis or carried a FAKE carrier.
- verdict: KILLED (instance) — score 32, 16 vs 37 insns, 5 hunks all source-level, 0 operand-only, 0 not-scored. Identical to s9/s11/s13/s17/s18. Kill stands.

## [s19] (solver) inverse_compose classify places the first divergence in the RA or SCHED layer.
- verdict: KILLED (instance) — PRE-RA. Ours 16 insns vs target 37; target-only shapes are the lwc2/c2 rtpt/j/addiu 24 the sandbox strips as unpinned gtemacro units. On the stripped object th…

## [s19] (solver) Some perturbation of the global.c inputs (refs, live length, birth order, conflicts, preferences, calls-crossed) gives the mfc2 temps $t0/$t1/$t2 with the other four pseudos at the target disposition.
- verdict: KILLED (instance) — NEGATIVE for both goals over 196 / 207 atoms at depth 3. 28 / 21 preference atoms FORECLOSED: $t0/$t1/$t2/$s0 never appear as hard regs in the pre-RA RTL, so n…

## [s19] Kill re-audit: rejected/psyq-rtpt-pair-reorg-slot-residual.c has drifted on the current chassis or carried a FAKE carrier that voids its kill.
- verdict: KILLED (instance) — Zero FAKE constructs. Score 32, 16 vs 37 insns, 5 source-level / 0 operand-only / 0 not-scored hunks, identical to s9/s11/s13/s17/s18. Kill stands.

## [s19] inverse_compose classify places the first divergence of the closest form in the RA or SCHED layer, so an RA/sched backend search applies to the sandbox stream.
- verdict: KILLED (instance) — PRE-RA: ours 16 insns vs target 37. The target-only shapes are the lwc2/rtpt/j/addiu 24 that the sandbox strips as unpinned gtemacro units, so the sandbox stre…

## [s19] A depth-3 perturbation of the global.c inputs (refs, live length, birth order, conflicts, preferences, calls-crossed) on the $a3 chassis seats the three mfc2 temps in $t0/$t1/$t2 while the other pseudos keep the target…
- verdict: KILLED (instance) — NEGATIVE for both goals over 196/207 atoms at depth 3. 28/21 preference atoms foreclosed because $t0-$t2/$s0 never appear as hard regs pre-RA. Reaching $t0 for…

## [s20] (forensics) Kill re-audit: rejected/psyq-rtpt-pair-reorg-slot-residual.c has drifted on the current chassis or carried a FAKE carrier that voids its kill.
- verdict: KILLED (instance) — Zero FAKE constructs. Score 32, target 37 vs ours 16 insns, 5 hunks all source-level, 0 operand-only, 0 not-scored. Identical to s9/s11/s13/s17/s18/s19. Kill s…

## [s20] (forensics) The mfc2 temps z1-z3 of the closest form are block-local pseudos, so local-alloc's qty/suggestion pass (not global.c first-fit) decides their seats and a local-alloc input shape could reach $t0-$t2.
- verdict: KILLED (instance) — p.i.lreg: pseudos 76/77/78 (z1-z3) are "Registers live at start" of basic blocks 1 and 3 (and 76-78 live into the exit block), each used 3 times across 12/13/1…

## [s20] (forensics) label_census: the closest form's CFG differs from the target in block/predecessor structure.
- verdict: KILLED (instance) — Label count 3 vs 3, predecessor multiset [1,1,2] vs [1,1,2] in identical program order (loop head j-only, mid fallthrough+j, exit beqz-only). CFG matches. Rema…

## [s20] Kill re-audit: rejected/psyq-rtpt-pair-reorg-slot-residual.c has drifted on the current chassis or carried a FAKE carrier that voids its kill.
- verdict: KILLED (instance) — Zero FAKE constructs; score 32, 37 vs 16 insns, 5/5 source-level hunks, identical to s9-s19. Kill stands.

## [s20] The mfc2 temps z1-z3 of the closest form are block-local pseudos, so local-alloc rather than global.c first-fit decides their seats and a local-alloc input shape could reach $t0-$t2.
- verdict: KILLED (instance) — Pseudos 76-78 are live at the start of basic blocks 1 and 3 (global pseudos); greg conflicts are the other pseudos plus $sp only, no preferences; dispositions…

## [s20] The closest form's CFG differs from the target in label/predecessor structure.
- verdict: KILLED (instance) — 3 vs 3 labels, predecessor multiset [1,1,2] vs [1,1,2] in identical program order. Only diffs: $s0 refs 16 vs 0 (ABI) and insn count 39 vs 36 (rtpt .word not c…

## Latest session s21 (verbatim)

## [s21] (forensics) Kill re-audit: rejected/psyq-rtpt-pair-reorg-slot-residual.c has drifted on the current chassis or carried a FAKE carrier that voids its kill.
- mechanism: Mandated re-audit (floor flat 3+ sessions).
- probe: fake_ablate.py (tmp/grind/func_8004A76C/s21/ablate_reaudit.txt); sandbox --disable all --diff --candidate <form> (s21/sandbox_reaudit.txt); canonical (s21/canonical.txt).
- result: Zero FAKE constructs; score 32, target 37 vs ours 16 insns, 5/5 source-level, 0 operand-only, 0 not-scored. canonical reports "whole-function canonical-asm grant in inline_asm_canonical.txt" (line 25). Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at HEAD 1188fb1ef, $a3 vertex-pointer chassis, no FAKE construct

## [s21] (forensics) Raising the mfc2 temps' allocno priority (reg_n_refs lift so z1-z3 rank ahead of the param pseudos) moves their seats off $v0/$v1 toward the target's $t0-$t2.
- mechanism: global.c:615 priority / allocno_compare order; find_reg first-fit.
- probe: closest form spliced into src/main/3AB48.c (restored after; git status clean), tools/nrefs_census.py census + --above 76:72 (s21/nrefs_census.txt).
- result: Census validates (pri(hook)==pri(calc) every row): order 74(a2) 75(a3) 72(a0) 73(a1) 76->$2 77->$3 78->$8. z1 needs only +1 ref to outrank pseudo 72. But order is inert for the seat: z1-z3's conflict set is pseudos + $sp only (s20 greg), and the params carry hard-reg preferences for $4-$7, so whatever rank z1 gets, the lowest unconflicted call-used reg is $2 and it takes it; ranking z1 first would at most push a param pseudo off its preferred reg (more diffs, not fewer). The target's $8/$9/$10 with $2/$3 AND $7 free (v lives in $s0 in the target) is not a first-fit outcome at any priority order.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c spliced into src/main/3AB48.c, instrumented cc1 via nrefs_census, $a3 chassis, no FAKE construct

## [s21] (forensics) loop.c (LICM/strength reduction) participates in the closest form's codegen and could be the lever for the a1-increment position.
- mechanism: loop.c scan_loop needs NOTE_INSN_LOOP_BEG; the form builds its loop from goto.
- probe: tools/loop_movables.py --dumps over the s20 -da dumps staged as s21/dumps20/main/3AB48.* (s21/loop_movables.txt).
- result: "no non-phony loops in this function": the goto loop carries no LOOP_BEG note, so loop.c makes no decision; the a1-increment position is purely source order + sched/reorg (s7 reorg.c:2990 attribution stands). A for/while-spelled loop changes the CFG away from the target (s5 honest-loop form, 33).
- verdict: KILLED
- kill_scope: instance
- measured_on: s20 -da dumps of rejected/psyq-rtpt-pair-reorg-slot-residual.c, $a3 chassis, no FAKE construct

## [s21] Kill re-audit: rejected/psyq-rtpt-pair-reorg-slot-residual.c has drifted on the current chassis or carried a FAKE carrier that voids its kill.
- mechanism: Mandated re-audit (floor flat 3+ sessions).
- probe: tools/fake_ablate.py on the form; sandbox func_8004A76C --disable all --diff --candidate <form>; canonical func_8004A76C.
- result: Zero FAKE constructs; score 32, target 37 vs ours 16 insns, 5/5 hunks source-level, 0 operand-only, 0 not-scored. canonical: whole-function canonical-asm grant. Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at HEAD 1188fb1ef, $a3 vertex-pointer chassis, no FAKE construct

## [s21] Raising the mfc2 temps' allocno priority (reg_n_refs lift so z1-z3 rank ahead of the param pseudos) moves their seats off $v0/$v1 toward the target's $t0-$t2 on the closest form.
- mechanism: global.c:615 priority and allocno_compare order feeding find_reg first-fit.
- probe: Closest form spliced into src/main/3AB48.c (restored after), tools/nrefs_census.py census and --above 76:72.
- result: Census validates (pri hook == calc). Order 74,75,72,73,76,77,78; z1->$2 z2->$3 z3->$8. z1 needs only +1 ref to outrank pseudo 72, but its conflict set is pseudos + $sp only and the params prefer $4-$7, so first-fit gives $2 at any rank. Target uses $8-$10 while $2,$3 and $7 are free, which no priority order produces.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c spliced into src/main/3AB48.c, instrumented cc1 via nrefs_census, $a3 chassis, no FAKE construct

## [s21] loop.c (LICM / strength reduction) makes a decision in the closest form that could move the a1 increment relative to the mfc2s.
- mechanism: loop.c scan_loop only processes loops bracketed by NOTE_INSN_LOOP_BEG; the form's loop is goto-built.
- probe: tools/loop_movables.py --dumps over the s20 -da dumps of the form staged under tmp/grind/func_8004A76C/s21/dumps20.
- result: No non-phony loops in this function; loop.c is inert. The a1-increment position is decided by source order + sched/reorg (s7 attribution stands).
- verdict: KILLED
- kill_scope: instance
- measured_on: s20 -da dumps of rejected/psyq-rtpt-pair-reorg-slot-residual.c, $a3 chassis, no FAKE construct

## [s22] A fresh decomp.me similarity search against the now-461-scratch corpus surfaces a closer match than s8/s11/s12's null result, or m2c emits a materially different skeleton on re-run.
- mechanism: tools/decomp_me_scrape.py search re-shingles asm/funcs/func_8004A76C.s against every cached gcc2.7.2-psx scratch; a larger corpus could in principle contain a GTE-shaped scratch added since s8.
- probe: Re-ran decomp_me_scrape.py search against tmp/decomp_me_corpus (461 scratches) and diffed against s8's cached output; re-read s12's m2c_out.c and tools/m2c/m2c_macros.h; re-read sibling func_8004A4E0/candidate.c in full; re-measured sandbox --disable all.
- result: Byte-identical top-10 decomp.me result (best similarity 0.011, RecordButtonsToDevkit, no GTE match) -- corpus held 461 scratches already at s8, no growth occurred. m2c's cop2 M2C_ERROR output is a structural fact (GTE opcodes have no C lowering under any m2c mode), not a tool-flag or corpus-state artifact. Sibling func_8004A4E0's candidate.c unchanged since 2026-10-15 17:03, still comments-only, nothing to transplant. sandbox re-confirmed score 37, no_c_body true.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_8004A76C.s unchanged, decomp.me corpus at 461 scratches (unchanged from s8), no FAKE construct (comments-only stub, nothing to ablate)

## s23 (escalation)
- H-s23a: Closest pure-C form still sits at 32 with zero FAKE on current chassis. probe: sandbox --diff + fake_ablate. result: 32, 5/5 source-level, no FAKE. verdict: KILLED (instance) — residual not closed by that form.
- H-s23b: Function qualifies for canonical-asm grant path. probe: scan_hand_coded --single. result: STRONG (S7+S11, $s0). verdict: CONFIRMED. Entry filed in docs/grind/decisions.md.

## [s23] The closest pure-C form rejected/psyq-rtpt-pair-reorg-slot-residual.c closes the residual on the current chassis once FAKE constructs are ablated.
- mechanism: Kill re-audit mandated by flat floor: drift or a FAKE carrier could hide a lever.
- probe: sandbox --disable all --diff --candidate on the form; fake_ablate.py on it.
- result: Score 32, 5/5 hunks source-level; fake_ablate reports no FAKE constructs. Unchanged from s20/s21.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/psyq-rtpt-pair-reorg-slot-residual.c at HEAD 07d619ccf, $a3 vertex-pointer chassis, no FAKE construct

## [s23] func_8004A76C meets gate (a) of the endgame-lock check (STRONG hand-coded scan tier).
- mechanism: Caller-passed live-in $s0 mutated without prologue save (S7, S11) is a gate-class not-GCC signal.
- probe: python3 tools/scan_hand_coded.py --single func_8004A76C
- result: tier=STRONG score 2/12, S7 unsaved $s0 and S11 live-in $s0 set.
- verdict: CONFIRMED
