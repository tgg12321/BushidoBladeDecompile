# Hypothesis ledger — func_80052B00

## Session 1 (recon, 2026-08-01) — floor 18 (baseline established)

### H1 — CONFIRMED (structural impossibility)
**Statement.** No C source compiled by this frozen toolchain — pure C, or C
containing canonical GTE inline asm — can produce func_80052B00's target
bytes, because the target's `jr $ra` delay slot holds `ctc2 $t7, $7`.

**Mechanism.** `ctc2` is a cop2 control-register write with no C analog, so it
can only originate from an `__asm__` block. `tools/gcc-2.7.2/reorg.c:730-735`
(`stop_search_p`) returns 1 unconditionally for any INSN whose pattern is
`ASM_INPUT` or for which `asm_noperands(...) >= 0`; `fill_simple_delay_slots`
scans backward from the jump and halts at the first such stop. So GCC 2.7.2
will never move an `__asm__` insn into a `jr` delay slot — there is no C
spelling of the required final instruction placement.

**Probe.** Read `reorg.c:706-740` (saved to
`tmp/grind/func_80052B00/s1/reorg_stop_search_p.txt`); cross-checked against
the honest sandbox disassembly, which emits `jr ra; nop` with the eighth
`ctc2` hoisted above the jump
(`tmp/grind/func_80052B00/s1/sandbox_disasm.txt`).

**Corroboration.** The tree's single regfix rule for this function is
`fill_delay @ 16 <- 15` — build-time paperwork whose entire job is to perform
the delay-slot move GCC refuses to make. The adjacent twin `func_80052B44`
(`src/text1b.c:10995`) was Judge-authorized canonical-asm on 2026-07-27
citing this exact reorg.c mechanism (`inline_asm_canonical.txt:340`).

**Verdict: CONFIRMED.** Note this does NOT by itself dispose of the function —
per the standing 2026-07-27 auto-ruling, declaring exhaustion is the driver's
call, not a session's. It is banked so no future session re-derives it.

### H2 — CONFIRMED (register allocation is pin-only)
**Statement.** The target's `$t0..$t7` allocation and strict ascending
load order are unreachable without `register asm("$N")` pins (forbidden
cheat-asm).

**Mechanism.** In a leaf with no calls, GCC's allocator draws temps from the
caller-save pool in order `$v0,$v1,$a1,$a2,$a3,$t0,$t1,...`; and because `$a0`
is the live base pointer, the `matrix[0]` load is deferred to last so it can
reuse `$a0` as its destination. Both are natural allocator behaviour with no
C-level lever: there is no computation, no control flow, and no call in the
body to bias allocno priority or liveness.

**Probe.** `sandbox --disable all` strips the pins currently at
`src/text1b.c:10970-10977`; the resulting disassembly shows exactly the
`$v0,$v1,$a1..$t1` pool plus the inverted final load
(`tmp/grind/func_80052B00/s1/sandbox_disasm.txt`).

**Verdict: CONFIRMED** as the source of 8 of the 18 gap points. Independent
of H1 — even if the delay slot were somehow reachable, this would remain.

### Standing constraint for all future sessions
The body has zero general-purpose computation: 8 array reads feeding 8 cop2
writes. There is no arithmetic to restructure, no control flow to reshape, no
call to reorder, no type to correct. The entire pure-C lever catalog
(register-alloc-pure-c A-D, cross-jump, scheduling, fold-defeat, width/addressing)
operates on constructs this function does not contain. Do not spend a session
running that catalog here — measure something new or advance the disposition.

## Live frontier (for the next session, highest value first)

1. **Canonical-asm authorization as the disposition** (mechanism: the
   [[gte-wrapper-misroute-park]] GTE-leaf carve-out, which is a no-C-form
   category orthogonal to the scan_hand_coded S1-S8 tiers). Precedent is in
   hand and citable: `func_80052B44` at `src/text1b.c:10995` +
   `inline_asm_canonical.txt:340`, authorized 2026-07-27 for the identical
   construct one function later in the same file; plus `gte_SetRotMatrix`
   (:326), `gte_SetColorMatrix` (:325), `gte_SetTransVector` (:324),
   `func_8007ED6C` (:308). Next probe: the whole-body glabel form is already
   written to `memory/grind/func_80052B00/candidate.c`; a session with the
   authority to touch `inline_asm_canonical.txt` applies it, adds the entry,
   and runs `retire func_80052B00` + `verify-oracle`. Under the endgame-lock
   AND-gates this is the *precedent* gate passing with a real file+line
   citation, not "genre-adjacent" — but the disposition call belongs to the
   driver/owner, not to a grind session in a non-escalation modality.

2. **Confirm the delay-slot impossibility empirically rather than by source
   reading** (mechanism: direct falsification attempt on H1). Next probe:
   compile a minimal TU containing only this body with `cc1 -da` and inspect
   the `.dbr` (delay-branch-scheduling) dump to see `fill_simple_delay_slots`
   bail out at the asm insn, and sweep 3-4 orderings (ctc2 $7 written first /
   last / via a trailing `return`) to show the slot stays `nop` in every form.
   Cheap, and converts a read-the-compiler-source argument into a measured one
   for the owner's audit trail.

3. **Widen the family census** (mechanism: establish that func_80052B00 is the
   last unauthorized member of an already-settled cluster). Next probe:
   grep `asm/funcs/*.s` for bodies consisting solely of `lw`/`ctc2`/`jr` and
   cross-check each against `inline_asm_canonical.txt`; if func_80052B00 is
   the only one still active, that is a clean completeness argument for the
   disposition in frontier item 1.

## [s1] No C source compiled by this frozen toolchain (pure C, or C containing canonical GTE inline asm) can produce func_80052B00's target bytes, because the target's jr $ra delay slot holds ctc2 $t7, $7.
- mechanism: ctc2 is a cop2 control-register write with no C analog, so it can only originate from an __asm__ block. tools/gcc-2.7.2/reorg.c:730-735 (stop_search_p) returns 1 unconditionally for any INSN whose pattern is ASM_INPUT or for which asm_noperands(...) >= 0; fill_simple_delay_slots scans backward from the jump and halts at the first such stop. GCC 2.7.2 therefore never moves an __asm__ insn into a jr delay slot, from any C spelling.
- probe: Read tools/gcc-2.7.2/reorg.c:706-740 (saved to tmp/grind/func_80052B00/s1/reorg_stop_search_p.txt) and cross-checked against the honest cheat-stripped sandbox disassembly (tmp/grind/func_80052B00/s1/sandbox_disasm.txt).
- result: reorg.c:730-735 confirms the unconditional ASM stop. The honest build emits 'jr ra; nop' with the eighth ctc2 hoisted above the jump. Corroborating fingerprint: the tree's single regfix rule for this function is regfix.txt:3411 'func_80052B00: fill_delay @ 16 <- 15' — build-time paperwork whose entire job is the delay-slot move GCC refuses to make.
- verdict: CONFIRMED

## [s1] The target's $t0..$t7 allocation and strict ascending load order are unreachable without register asm("$N") pins (forbidden cheat-asm), accounting for 8 of the 18 gap points independently of the delay-slot blocker.
- mechanism: In a leaf with no calls, GCC's allocator draws temps from the caller-save pool in order $v0,$v1,$a1,$a2,$a3,$t0,$t1; and because $a0 is the live base pointer, the matrix[0] load is deferred to last so it can reuse $a0 as its destination. Both are natural allocator behaviour with no C-level lever available: the body contains no arithmetic, no control flow, and no call to bias allocno priority or liveness.
- probe: sandbox func_80052B00 --disable all strips the 8 register-asm pins at src/text1b.c:10970-10977; disassembled the resulting object.
- result: Honest build allocates v0,v1,a1,a2,a3,t0,t1 plus a0 for the deferred matrix[0] load, loads offsets 4..28 first and offset 0 last, and inserts an extra load-delay nop before the first ctc2. Score 18 = 8 register renames + load-order inversion + 1 extra nop + 1 unfilled delay slot.
- verdict: CONFIRMED

## [s1] The SessionStart near-duplicate lead (func_80052B00 ~= func_8007EEEC, similarity 0.647) points to a usable COMPLETED-C template body.
- mechanism: The find_duplicates.py lens scores structural similarity; a COMPLETED-C right-hand side would normally be transcribable as a starting form.
- probe: Grepped for func_8007EEEC in inline_asm_canonical.txt and src/display.c.
- result: No hits — the lead is flagged STALE by the hook itself and there is no such body to template from. The genuine analogs are the cop2-control-loader family (gte_SetRotMatrix, gte_SetColorMatrix, gte_SetTransVector, func_8007ED6C, func_80052B44), every one of which is canonical-asm rather than COMPLETED-C.
- verdict: KILLED

## Session 2 (structural, 2026-08-01) — floor 18, unmoved

### H3 — KILLED (the structural axis has no gradient here)
**Statement.** Some pure-C structural spelling of func_80052B00's body —
declaration order, load order, walking pointer, extra pointer handle, block
scoping, statement interleaving, aggregate/type reshaping, cop2 write order, or
a trailing return — lowers the honest floor below 18.

**Mechanism.** The mandated structural lever family from the
codegen-technique-index: block-local var splits, declaration order, type
narrowing/widening, statement re-association, and pointer-walk serialization
([[walking-pointer-serializes-parallel-loads]]) all bias GCC's allocno order,
LUID assignment and scheduling, and are the standard levers for register-rename
and load-order residuals — which is exactly what 8 of this function's 18 points
are.

**Probe.** `tmp/grind/func_80052B00/s2/sweep.py` (8 whole-body forms) and
`sweep2.py` (6 ordering / re-association / reorg-perturbing forms, each with the
emitted tail instructions objdumped). Each rewrites the body in `src/text1b.c`,
runs `sandbox func_80052B00 --disable all`, and restores the original in a
`finally` block. No `register asm("$N")` pins in any form; the only inline asm
is the canonical `ctc2` with `%N` placeholders.

**Result.** 14 forms measured. Eight TIE 18 (plain-locals, walking-pointer,
reverse-load-order, base-copy-handle, ctc2-reverse-write, last-write-own-block,
do-while(0) probe, trailing-return); five are strictly worse
(interleaved-pairs-c89 21, direct-operand 25, interleaved-single 25,
struct-block-copy 43, local-array 46); one was void (C89-illegal, redone). NONE
improved on 18. Raw output: `sweep_results.txt`, `sweep2_results.txt`.

**Verdict: KILLED.** The structural modality is exhausted for this function with
measurements, not with an argument. Do not re-run this catalog here.

### H1 — RE-CONFIRMED, now empirically
**Statement (s1).** No C source compiled by this frozen toolchain can produce
the target bytes, because the target's `jr $ra` delay slot holds `ctc2 $t7, $7`
and GCC 2.7.2 never fills a delay slot with an `__asm__` insn.

**New probe (s2).** Instead of reading `reorg.c`, measure it: round 2 of the
sweep objdumps every variant and records the last three instructions.

**Result.** 14/14 forms end `... | jr $ra | nop`, with the eighth `ctc2` hoisted
above the jump. Includes the `do { } while (0)` probe — `NOTE_INSN_LOOP_BEG` is
the single documented lever that perturbs reorg.c's delay-slot/relax_delay_slots
behaviour ([[do-while-zero-exception]]) and it does not move the slot either.
(Run as a falsification probe only; it produced no benefit and is not a proposed
form.)

**Verdict: CONFIRMED** (upgraded from source-read to measured). Per the standing
2026-07-27 auto-ruling this still does NOT dispose of the function — exhaustion
is the driver's call.

### H4 — CONFIRMED (the cop2-control-loader family is settled but for this one)
**Statement.** func_80052B00 is the last unauthorized member of an otherwise
fully-settled cop2-control-loader cluster.

**Mechanism / probe.** `tmp/grind/func_80052B00/s2/census.py` scans every
`asm/funcs/*.s` whose opcode set is a subset of {lw, ctc2, jr, nop} and contains
a `ctc2`, then cross-checks each name against `inline_asm_canonical.txt` and
`engine/queue.json`.

**Result.** Exactly six bodies qualify: func_80052B00 (ACTIVE), func_80052B44
(canonical 2026-07-27), func_8007EEEC / func_8007EF1C / func_8007EF4C (=
gte_SetRotMatrix / gte_SetColorMatrix / gte_SetTransVector per
`named_syms.txt:640-642`, all canonical 2026-06-07), and tslDmaDrawListDelAll
(pure C at `src/display.c:2491` — one `ctc2` of the incoming param, no loads to
allocate, `jr; nop` tail). The script's raw "unauthorized=5" is a
name-resolution artifact: the three `gte_*` bodies ARE authorized under their
named_syms names. True unauthorized count: ONE. The family partitions on exactly
the H1 criterion — the only two members whose TARGET holds a cop2 write in the
`jr $ra` delay slot are func_80052B44 (authorized on precisely that ground) and
func_80052B00.

**Verdict: CONFIRMED.** This is a completeness argument for the disposition in
frontier item 1, not a disposition by itself.

## Live frontier after session 2 (highest value first)

1. **Canonical-asm authorization remains the disposition**, now backed by
   measurement rather than argument: floor 18 flat across two sessions and 14
   measured spellings, H1 empirically confirmed 14/14, and a complete family
   census showing func_80052B00 is the last unauthorized member of a settled
   cluster whose only other delay-slot-filled member was authorized on this
   exact ground (`func_80052B44`, `src/text1b.c:10995`,
   `inline_asm_canonical.txt:340`, 2026-07-27). Next probe: a session with
   authority over `inline_asm_canonical.txt` applies
   `memory/grind/func_80052B00/candidate.c` over `src/text1b.c:10969-10994`,
   adds the entry, runs `retire func_80052B00` (drops `regfix.txt:3411`) and
   `verify-oracle`. The disposition call belongs to the driver/owner.

2. **Register-allocation axis is the only untried measurable one** and is
   already CONFIRMED dead by mechanism in s1's H2 (target wants `$t0..$t7`;
   GCC's leaf caller-save pool gives `$v0,$v1,$a1..$t1`; only a forbidden pin
   reaches it). If a later modality wants a measurement rather than a mechanism,
   the cheap probe is an instrumented-cc1 `.greg` dump
   (`tools/gcc-2.7.2/cc1 -da`, per [[instrumented-cc1-location]]) showing the
   allocno order for the 8 load pseudos — expect no conflicts and pure
   pool-order assignment, i.e. nothing to steer. Low value: even a hypothetical
   fix leaves H1's delay slot, so the floor cannot reach 0.

3. **Permuter is NOT worth a session here.** The body has 17 instructions, no
   control flow and no arithmetic; the entire mutation space the permuter
   explores (statement reordering, temp introduction, type changes) is what
   sweep.py/sweep2.py just enumerated by hand, and the residual is dominated by
   a hard register-set requirement plus a provably unreachable delay slot.
   Recorded so a later session does not spend a fresh-seed campaign on it.

## [s2] Some pure-C structural spelling of the body (declaration order, load order, walking pointer, extra pointer handle, block scoping, statement interleaving, aggregate/type reshaping, cop2 write order, trailing return) lowers the honest floor below 18.
- mechanism: The mandated structural lever family from the codegen-technique-index biases GCC's allocno order, LUID assignment and scheduling — the standard levers for the register-rename and load-order residuals that make up 8 of this function's 18 points.
- probe: tmp/grind/func_80052B00/s2/sweep.py (8 whole-body forms) + sweep2.py (6 ordering/re-association/reorg-perturbing forms with objdumped tails); each rewrites the body in src/text1b.c, runs sandbox --disable all, restores the original in a finally block. No register-asm pins in any form.
- result: 14 forms measured. Eight tie 18 (plain-locals, walking-pointer, reverse-load-order, base-copy-handle, ctc2-reverse-write, last-write-own-block, do-while(0) probe, trailing-return); five are strictly worse (21/25/25/43/46); none improved. Raw: sweep_results.txt, sweep2_results.txt.
- verdict: KILLED

## [s2] H1 re-tested empirically: if any C spelling could get an __asm__ ctc2 into the jr delay slot, a sweep of orderings would find it.
- mechanism: reorg.c:730-735 stop_search_p halts fill_simple_delay_slots at any ASM_INPUT / asm_noperands insn; the prediction is that the slot stays nop under every C spelling, including the one documented reorg-perturbing lever (do-while(0), NOTE_INSN_LOOP_BEG).
- probe: sweep2.py objdumps each variant's sandbox object and records the last three emitted instructions.
- result: 14/14 forms end '... | jr $ra | nop' with the eighth ctc2 hoisted above the jump — including the do-while(0) probe. The s1 source-read argument is now a measurement.
- verdict: CONFIRMED

## [s2] func_80052B00 is the last unauthorized member of an otherwise fully-settled cop2-control-loader cluster.
- mechanism: If every other lw/ctc2/jr-only body in the tree is already authorized or done, the category is settled and this function is a straggler rather than a novel case.
- probe: tmp/grind/func_80052B00/s2/census.py — scans asm/funcs/*.s for opcode sets within {lw,ctc2,jr,nop} containing a ctc2, cross-checks inline_asm_canonical.txt + engine/queue.json.
- result: exactly 6 qualify — func_80052B00 (ACTIVE), func_80052B44 (canonical 2026-07-27), func_8007EEEC/EF1C/EF4C (= gte_SetRotMatrix/SetColorMatrix/SetTransVector, named_syms.txt:640-642, canonical 2026-06-07), tslDmaDrawListDelAll (pure C, src/display.c:2491, single ctc2 of the incoming param). True unauthorized count is ONE. The family partitions on the H1 criterion exactly: only func_80052B44 and func_80052B00 have a cop2 write in the target's jr delay slot, and func_80052B44 was authorized on precisely that ground.
- verdict: CONFIRMED

## [s2] Some pure-C structural spelling of func_80052B00's body (declaration order, load order, walking pointer, extra pointer handle, block scoping, statement interleaving, aggregate/type reshaping, cop2 write order, trailing return) lowers the honest floor below 18.
- mechanism: The mandated structural lever family from the codegen-technique-index biases GCC's allocno ordering, LUID assignment and scheduling; these are the standard levers for register-rename and load-order residuals, which are 8 of this function's 18 gap points.
- probe: tmp/grind/func_80052B00/s2/sweep.py (8 whole-body forms) and sweep2.py (6 ordering / re-association / reorg-perturbing forms, each with the emitted tail instructions objdumped). Each script rewrites the body in src/text1b.c, runs `sandbox func_80052B00 --disable all`, and restores the original body in a finally block. No register-asm pins in any variant; the only inline asm is the canonical ctc2 with %N placeholders and an "r" constraint.
- result: 14 forms measured. Eight tie the floor at exactly 18 (plain-locals, walking-pointer, reverse-load-order, base-copy-handle, ctc2-reverse-write, last-write-own-block, do-while(0) probe, trailing-return); five are strictly worse (interleaved-pairs-c89 21, direct-operand 25, interleaved-single 25, struct-block-copy 43, local-array 46); one round-1 form was void (C89-illegal decls after statements) and was redone correctly in round 2. NONE improved on 18.
- verdict: KILLED

## [s2] H1 re-tested empirically: if any C spelling could get an __asm__ ctc2 into the jr $ra delay slot, a sweep of orderings would find it.
- mechanism: reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots; the prediction is that the slot stays nop under every C spelling, including the one documented reorg-perturbing lever (do { } while (0), which emits NOTE_INSN_LOOP_BEG).
- probe: sweep2.py objdumps each variant's sandbox object and records the last three emitted instructions.
- result: 14/14 forms end '... | jr $ra | nop', with the eighth ctc2 always hoisted above the jump — including the do-while(0) probe. Session 1's compiler-source argument is now corroborated by 14 independent measurements. (The do-while(0) form was run as a falsification probe only; it produced no benefit, is not a proposed form, and its sanctioned-use prerequisites were never in play.)
- verdict: CONFIRMED

## [s2] func_80052B00 is the last unauthorized member of an otherwise fully-settled cop2-control-loader cluster.
- mechanism: If every other lw/ctc2/jr-only body in the tree is already canonical-authorized or done, the category is settled and this function is a straggler rather than a novel case.
- probe: tmp/grind/func_80052B00/s2/census.py — scans every asm/funcs/*.s whose opcode set is a subset of {lw, ctc2, jr, nop} and contains a ctc2, then cross-checks each name against inline_asm_canonical.txt and engine/queue.json; names resolved through named_syms.txt.
- result: Exactly six bodies qualify: func_80052B00 (17 insns, ACTIVE in queue), func_80052B44 (14, canonical-authorized 2026-07-27), func_8007EEEC / func_8007EF1C / func_8007EF4C (= gte_SetRotMatrix / gte_SetColorMatrix / gte_SetTransVector per named_syms.txt:640-642, all canonical-authorized 2026-06-07), and tslDmaDrawListDelAll (4 insns, DONE in pure C at src/display.c:2491 — a single ctc2 of the incoming param, no loads to allocate). The script's raw 'unauthorized=5' is a name-resolution artifact; the true unauthorized count is ONE. The family partitions on exactly the H1 criterion: the only two members whose TARGET holds a cop2 write in the jr $ra delay slot are func_80052B44 (authorized on precisely that ground) and func_80052B00.
- verdict: CONFIRMED
