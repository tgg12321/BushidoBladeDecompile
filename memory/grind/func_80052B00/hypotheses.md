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

## Session 3 (structural, 2026-08-01) — FLOOR 18 -> 17

### H5 — CONFIRMED (asm-statement granularity is a live structural lever)
**Statement.** Fusing all eight cop2 writes into a SINGLE `__asm__` statement
with eight `"r"` inputs lowers the honest floor below 18.

**Mechanism.** With eight separate one-operand asm statements each loaded value
is live only across its own asm insn, so GCC can recycle registers and defer the
`matrix[0]` load into the dying `$a0`. One asm statement with eight inputs makes
all eight values simultaneously live at one program point — the only C-level
construct that can REQUIRE eight distinct hard registers without a
`register asm("$N")` pin. That kills both the load-order inversion and the
load-delay `nop`.

**Probe.** `tmp/grind/func_80052B00/s3/sweep3.py` — 11 forms varying asm
granularity (1 / 2 / 4 / 7+1 / 1+7 / 8 writes per statement), operand order,
direct-rvalue operands, unsigned temporaries, walking pointer and reversed load
order. Each rewrites the body in `src/text1b.c`, runs
`sandbox func_80052B00 --disable all`, objdumps the sandbox object, and restores
the original in a `finally` block.

**Result.** Every fused form scores **17** (build 18 insns vs target 17), down
from the two-session floor of 18. Full emitted bodies in `s3/form_disasm.txt`:
the fused form is instruction-for-instruction isomorphic to the target — loads
in strict ascending offset order 0..0x1C, cop2 writes in CR order, no load-delay
nop. Re-verified from the banked artifact (`s3/verify_banked.txt`).

**Verdict: CONFIRMED.** The structural axis was NOT dead — session 2's KILL was
correct for the statement level it swept, but the asm-granularity dimension was
untested and was worth one point plus a complete structural cleanup of the body.

### H6 — CONFIRMED (the residual is now purely register naming + the delay slot)
**Statement.** After the fused form, the entire 17-point residual is 16
instructions naming the wrong GPRs plus the one unfilled `jr $ra` delay slot.

**Probe / result.** `s3/dump_forms.py` dumps target and build side by side.
Build: `lw v0,0(a0) .. lw t2,28(a0)` / `ctc2 v0,$0 .. ctc2 t2,$7` / `jr ra` /
`nop`. Target: `lw $t0..$t7` / `ctc2 $t0..$t6,$0..$6` / `jr $ra` /
`ctc2 $t7,$7`. Opcodes, order, offsets and operand roles all match; only the
GPR names differ, on all 16 instructions, plus the delay slot.

**Verdict: CONFIRMED.** If the register set could be steered, the honest floor
would be 1 — and that last point is H1, which is a proven impossibility. So the
pure-C floor for this function is exactly 1 in the limit, and 17 in practice.

### H7 — KILLED (no legal C spelling moves the allocator's starting register)
**Statement.** Some parameter type, declaration order, constraint letter,
operand-list shape or copy structure moves GCC off `$v0` as the first allocated
hard register, letting the eight temporaries land on `$t0..$t7`.

**Mechanism (compiler source, new this session).**
`tools/gcc-2.7.2/config/mips/mips.h` defines **no `REG_ALLOC_ORDER`**. Without
it, local-alloc's `find_free_reg` walks hard registers in plain ascending number
order and takes the first non-conflicting one: `$2, $3, (skip $4 — the live base
pointer), $5, $6, $7, $8, $9, $10`, i.e. exactly the observed
`v0,v1,a1,a2,a3,t0,t1,t2`. The only documented ways to move that start point are
a conflict across a call and a copy suggestion to a named hard register; a
call-free leaf with no computation has neither.

**Probe.** `tmp/grind/func_80052B00/s3/sweep4.py` — `const s32 *` parameter,
`s32 matrix[8]` parameter, `void *` parameter with an in-body cast, reversed
declaration order, `"d"` instead of `"r"` constraints, a ninth `"r"(matrix)`
operand keeping the base pointer live through the asm, and a copy chain through
a second set of eight locals.

**Result.** 7/7 score 17 with the ctc2 register sequence
`v0,v1,a1,a2,a3,t0,t1,t2` — identical to the round-3 forms. Across all 18
session-3 spellings the register SET never changed. The only constructs that
would change it (a clobber list naming `$2/$3/$5-$7`, or `register T x asm("$N")`
pins) are register pins by another spelling, forbidden and score-inert under the
cheat-invisible sandbox.

**Verdict: KILLED.** 16 of the remaining 17 points are unreachable in pure C.

### PROBE ONLY — volatile pointer (not a proposed form)
`volatile s32 *vp = matrix;` feeding the fused asm scores 15. Volatile coercion
of a non-hardware access is a cheat under [[inline-asm-policy]] /
[[legitimate-volatile-interrupt-touched]]. Recorded solely to measure what the
load scheduling is worth (2 points). It must not be proposed as a form, and it
does not fill the delay slot either.

## Live frontier after session 3 (highest value first)

1. **Canonical-asm authorization remains the disposition**, and session 3
   strengthens rather than weakens it: the honest pure-C body is now
   instruction-for-instruction isomorphic to the target, which shows the C form
   is *right* and the residual is entirely register naming (mechanically
   unreachable, H7) plus the delay slot (provably unreachable, H1). Precedent
   unchanged and citable: `func_80052B44`, `src/text1b.c:10995`,
   `inline_asm_canonical.txt:340`, authorized 2026-07-27 on the identical
   construct one function later in the same file. Next probe: a session or
   operator with authority over `inline_asm_canonical.txt` applies
   `memory/grind/func_80052B00/candidate.c` over `src/text1b.c:10969-10994`,
   adds the entry, runs `retire func_80052B00` (drops `regfix.txt:3411`) and
   `verify-oracle`. The disposition call belongs to the driver/owner.

2. **If the canonical disposition is refused, `best_pure_c_fused8_floor17.c` is
   the body to ship**, not the HEAD body. It is pin-free, scores 17 honestly,
   and would need only `regfix.txt:3411` (the delay-slot move) plus register
   paperwork rather than eight `register asm("$N")` pins plus that rule — i.e.
   it strictly reduces the cheat surface even in the fallback outcome. Next
   probe: measure what rule set an operator would actually need to make the
   fused body byte-match with rules ENABLED (a `sandbox` run without
   `--disable all`, and a full `verify-oracle` after splicing) — a surface a
   grind session may not touch, so this is an operator step.

3. **The remaining sanctioned modalities still have no new axis.** Session 3
   found the one untested structural dimension; forensics / rederive / synthesis
   operate on constructs this body does not contain (no arithmetic, no control
   flow, no calls, no data layout). Permuter remains not worth a session (s2):
   its mutation space is statement reordering / temp introduction / type
   changes, all now enumerated by hand across 32 forms, and the residual is a
   hard register-set requirement plus a provably unreachable delay slot.

## [s3] Fusing all eight cop2 writes into a SINGLE __asm__ statement with eight "r" inputs lowers the honest floor below 18.
- mechanism: With eight separate one-operand asm statements each loaded value is live only across its own asm insn, so GCC recycles registers and defers the matrix[0] load into the dying $a0. One asm statement with eight inputs makes all eight values simultaneously live at a single program point — the only C-level construct that can REQUIRE eight distinct hard registers without a register asm("$N") pin — which kills both the load-order inversion and the extra load-delay nop.
- probe: tmp/grind/func_80052B00/s3/sweep3.py — 11 forms varying asm granularity (1/2/4/7+1/1+7/8 cop2 writes per statement), operand order, direct-rvalue operands, unsigned temporaries, walking pointer, reversed load order. Each rewrites the body in src/text1b.c, runs sandbox --disable all, objdumps the object, restores the original in a finally block. Re-verified from the banked artifact by s3/verify_banked.py.
- result: every fused form scores 17 (build 18 insns vs target 17), down from the flat 18 of sessions 1 and 2. The emitted body becomes instruction-for-instruction isomorphic to the target: loads in strict ascending offset order 0..0x1C, cop2 writes in CR order, no load-delay nop. Session 2's structural KILL was correct for the statement level it swept but did not cover the asm-granularity dimension.
- verdict: CONFIRMED

## [s3] After the fused form the entire 17-point residual is 16 instructions naming the wrong GPRs plus the one unfilled jr $ra delay slot.
- mechanism: if the only surviving differences are operand names and the delay slot, then the C form itself is correct and every remaining point sits on the register-allocation axis (H2/H7) or the delay-slot axis (H1).
- probe: tmp/grind/func_80052B00/s3/dump_forms.py dumps the target body and the emitted body side by side for four forms (s3/form_disasm.txt).
- result: build emits lw v0,0(a0) .. lw t2,28(a0) / ctc2 v0,$0 .. ctc2 t2,$7 / jr ra / nop; target has lw $t0..$t7, 0..0x1C($a0) / ctc2 $t0..$t6,$0..$6 / jr $ra / ctc2 $t7,$7. Opcodes, ordering, memory offsets and operand roles all match; 16 instructions differ only in GPR name, plus the delay slot. The pure-C floor is therefore exactly 1 in the limit and 17 in practice.
- verdict: CONFIRMED

## [s3] Some parameter type, declaration order, constraint letter, operand-list shape or copy structure moves GCC off $v0 as the first allocated hard register, letting the eight temporaries land on $t0..$t7.
- mechanism: tools/gcc-2.7.2/config/mips/mips.h defines NO REG_ALLOC_ORDER, so local-alloc's find_free_reg walks hard registers in plain ascending number order and takes the first non-conflicting one — $2,$3,(skip $4, the live base pointer),$5,$6,$7,$8,$9,$10, exactly the observed v0,v1,a1,a2,a3,t0,t1,t2. The only documented ways to move that start point are a conflict across a call and a copy suggestion to a named hard register, and a call-free leaf with no computation has neither.
- probe: tmp/grind/func_80052B00/s3/sweep4.py — const s32 * parameter, s32 matrix[8] parameter, void * parameter with an in-body cast, reversed declaration order, "d" instead of "r" constraints, a ninth "r"(matrix) operand keeping the base pointer live through the asm, and a copy chain through a second set of eight locals.
- result: 7/7 score 17 with ctc2 register sequence v0,v1,a1,a2,a3,t0,t1,t2, identical to the round-3 forms. Across all 18 session-3 spellings the register SET never changed. The only constructs that would change it (a clobber list naming $2/$3/$5-$7, or register asm("$N") pins) are register pins by another spelling — forbidden, and score-inert under the cheat-invisible sandbox.
- verdict: KILLED

## [s3] PROBE ONLY (not a proposed form): a volatile-qualified pointer feeding the fused asm reaches 15.
- mechanism: volatile forces strict in-order, non-deferred loads, isolating how much of the residual the load scheduling is worth.
- probe: the volatile-param-PROBE variant in sweep3.py.
- result: score 15 (build 18 insns), delay slot still nop, register set reversed but unchanged as a set. Volatile coercion of a non-hardware access is a cheat under inline-asm-policy / legitimate-volatile-interrupt-touched, so this is banked purely as a 2-point measurement of the scheduling component and must never be proposed as a form.
- verdict: KILLED (as a form; retained as a measurement)

## [s3] Fusing all eight cop2 writes into a SINGLE __asm__ statement with eight "r" inputs lowers the honest floor below 18.
- mechanism: With eight separate one-operand asm statements each loaded value is live only across its own asm insn, so GCC recycles registers and defers the matrix[0] load into the dying $a0. One asm statement with eight inputs makes all eight values simultaneously live at a single program point - the only C-level construct that can REQUIRE eight distinct hard registers without a register asm("$N") pin - which kills both the load-order inversion and the extra load-delay nop. Sessions 1-2 swept the STATEMENT level only and never varied asm-statement granularity.
- probe: tmp/grind/func_80052B00/s3/sweep3.py: 11 forms varying asm granularity (1/2/4/7+1/1+7/8 cop2 writes per statement), operand order, direct-rvalue operands, unsigned temporaries, walking pointer, reversed load order. Each rewrites the body in src/text1b.c, runs `sandbox func_80052B00 --disable all`, objdumps the sandbox object, restores the original in a finally block. Re-verified end-to-end from the banked artifact by s3/verify_banked.py.
- result: Every fused form scores 17 (build 18 insns vs target 17), down from the flat 18 of sessions 1 and 2. The emitted body becomes instruction-for-instruction isomorphic to the target: loads in strict ascending offset order 0..0x1C, cop2 writes in CR order, no load-delay nop. Session 2's structural KILL was correct for the statement level it swept but did not cover this dimension.
- verdict: CONFIRMED

## [s3] After the fused form, the entire 17-point residual is 16 instructions naming the wrong GPRs plus the one unfilled jr $ra delay slot.
- mechanism: If the only surviving differences are operand names and the delay slot, the C form itself is correct and every remaining point sits on the register-allocation axis (H2/H7) or the delay-slot axis (H1).
- probe: tmp/grind/func_80052B00/s3/dump_forms.py dumps the target body and the emitted body side by side for four forms (s3/form_disasm.txt).
- result: Build emits lw v0,0(a0) .. lw t2,28(a0) / ctc2 v0,$0 .. ctc2 t2,$7 / jr ra / nop; target has lw $t0..$t7, 0..0x1C($a0) / ctc2 $t0..$t6,$0..$6 / jr $ra / ctc2 $t7,$7. Opcodes, ordering, memory offsets and operand roles all match; 16 instructions differ only in GPR name, plus the delay slot. The pure-C floor is therefore exactly 1 in the limit and 17 in practice.
- verdict: CONFIRMED

## [s3] Some parameter type, declaration order, constraint letter, operand-list shape or copy structure moves GCC off $v0 as the first allocated hard register, letting the eight temporaries land on $t0..$t7.
- mechanism: tools/gcc-2.7.2/config/mips/mips.h defines NO REG_ALLOC_ORDER, so local-alloc's find_free_reg walks hard registers in plain ascending number order and takes the first non-conflicting one - $2,$3,(skip $4, the live base pointer),$5,$6,$7,$8,$9,$10, exactly the observed v0,v1,a1,a2,a3,t0,t1,t2. The only documented ways to move that start point are a conflict across a call and a copy suggestion to a named hard register; a call-free leaf with no computation has neither.
- probe: tmp/grind/func_80052B00/s3/sweep4.py: const s32 * parameter, s32 matrix[8] parameter, void * parameter with an in-body cast, reversed declaration order, "d" instead of "r" constraints, a ninth "r"(matrix) operand keeping the base pointer live through the asm, and a copy chain through a second set of eight locals.
- result: 7/7 score 17 with the ctc2 register sequence v0,v1,a1,a2,a3,t0,t1,t2, identical to the round-3 forms. Across all 18 session-3 spellings the register SET never changed. The only constructs that would change it (a clobber list naming $2/$3/$5-$7, or register asm("$N") pins) are register pins by another spelling - forbidden, and score-inert under the cheat-invisible sandbox.
- verdict: KILLED

## [s3] PROBE ONLY, not a proposed form: a volatile-qualified pointer feeding the fused asm reaches 15.
- mechanism: volatile forces strict in-order, non-deferred loads, isolating how much of the residual the load scheduling is worth.
- probe: the volatile-param-PROBE variant in tmp/grind/func_80052B00/s3/sweep3.py.
- result: Score 15 (build 18 insns); delay slot still nop; register set reversed in order but unchanged as a set. Volatile coercion of a non-hardware access is a cheat under inline-asm-policy / legitimate-volatile-interrupt-touched, so this is banked purely as a 2-point measurement of the scheduling component and must never be proposed as a form.
- verdict: KILLED

## [s3] H1 (re-test, third session): if any C spelling could get an __asm__ ctc2 into the jr $ra delay slot, a sweep of asm granularities and operand shapes would find it.
- mechanism: reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots at any asm insn; ctc2 has no C analog so the target's delay-slot instruction can only come from an __asm__ block.
- probe: All 18 session-3 forms had their emitted tails objdumped (sweep3_results.txt, sweep4_results.txt, form_disasm.txt).
- result: 18/18 end 'jr $ra | nop'. Running total across sessions 1-3: 32/32 measured C spellings leave the delay slot unfilled.
- verdict: CONFIRMED

## Session 4 (permuter, 2026-08-01) — floor 17, unmoved

### H8 — KILLED (the permuter axis is dead, now by measurement rather than by argument)
**Statement.** A decomp-permuter campaign on func_80052B00 finds a C form scoring
below the honest floor of 17 (permuter base score 140).

**Mechanism (why it was worth measuring anyway).** Sessions 2 and 3 both asserted
the permuter was not worth a session because its mutation space (statement
reordering, temp introduction, type changes) had been hand-enumerated across 32
forms. That is an argument, not a measurement, and the driver correctly mandated
the modality. The falsifiable prediction: the permuter has no mutation that names
a hard register and none that can defeat `reorg.c`'s unconditional ASM stop, so
it cannot touch EITHER component of the residual (16 wrong-GPR instructions +
1 unfilled delay slot), and should therefore return nothing below 140.

**Probe.** Two telemetry-tracked campaigns via `tools/permuter_campaign.py`
(owner directive 2026-07-07), each waited on in-turn and harvested with `--stop`:
1. `fused8-r-constraints` — base = `best_pure_c_fused8_floor17.c` verbatim.
2. `struct-fields-direct-rvalue-fused8` — reseeded per the fresh-seed rule with a
   structurally different chassis: `GteCtl` struct field references fed directly
   as the fused asm's eight operands, zero named temporaries.
Workspace built by `tmp/grind/func_80052B00/s4/mkws.sh` running the real
cc1 → prologue_fix → maspsx → multu_pad chain, function-region extraction, and
`as -march=r3000 -no-pad-sections -O1 -G0`; validated against
`asm/funcs/func_80052B00.s`.

**Result.** Campaign 1: 31,871 iterations / 1,139 s / 8 workers — **zero finds**,
not even a score tie. Campaign 2: 85,443 iterations / 1,141 s — **one find, at
score 140 (a tie), at 12 s**, consisting of an appended dead
`int new_var; if (new_var) { new_var = 1; }` that changes no emitted instruction
and is an uninitialized dead-local cheat shape; banked as
`rejected/permuter-tie-dead-if-noise.c`, not proposed. 117,314 iterations across
two chassis, nothing below 140.

**Verdict: KILLED.** The permuter modality is closed for this function. The
zero-tie result on chassis 1 additionally shows the search space has no interior:
a 17-instruction body with no arithmetic, no control flow and no calls admits
almost no neutral mutation, so there is no basin for sampling to descend.

### H9 — CONFIRMED (this function's codegen is TU-context-independent)
**Statement.** func_80052B00 compiled in a minimal standalone TU emits exactly the
body it emits inside `src/text1b.c`.

**Mechanism.** Leaf, no calls, no globals, no statics, no data references — none
of the context-sensitive cc1 state (GP-relative decisions, prior-declaration
ordering, the scheduler-state effects noted in [[cc1-first-pass-scheduler-bug]])
has any input here.

**Probe.** `mkws.sh` compiles a TU containing only `typedef signed int s32;` plus
the s3 fused body and objdumps it; compared against the s3 in-tree sandbox dumps
(`s3/form_disasm.txt`).

**Result.** Identical: `lw v0,0(a0) … lw t2,28(a0) / ctc2 v0,$0 … ctc2 t2,$7 /
jr ra / nop`. Standalone-TU experiments on this function are faithful and cost a
fraction of an in-tree sandbox cycle — useful for any future session that wants a
cheap experiment vehicle here.

**Verdict: CONFIRMED.**

### H7 — re-CONFIRMED on a 19th spelling
The struct-field / no-temporaries chassis (never swept in s1-s3) emits the same
`{v0,v1,a1,a2,a3,t0,t1,t2}` register set. Removing the named temporaries entirely
does not move the allocator's starting hard register, consistent with the
"no `REG_ALLOC_ORDER` in `mips.h`, ascending-scan `find_free_reg`" mechanism.

### H1 — re-CONFIRMED across ~117k machine-generated spellings
Neither campaign produced any output with a filled delay slot (campaign 1
produced no outputs; campaign 2's single output is byte-identical in emitted
code to base, which ends `jr ra ; nop`). Running total: 32 hand-measured
spellings plus 117,314 permuter iterations, zero delay-slot fills.

## Live frontier after session 4 (highest value first)

1. **Canonical-asm authorization remains the disposition — unchanged by this
   session, and the evidence base is now one modality wider.** The honest pure-C
   body is instruction-for-instruction isomorphic to the target; the residual is
   register naming (H7, mechanically unreachable) plus the delay slot (H1,
   provably unreachable); the structural axis is dead by measurement (s2/s3) and
   now the permuter axis is too (s4). Precedent unchanged and citable:
   `func_80052B44`, `src/text1b.c:10995`, `inline_asm_canonical.txt:340`,
   authorized 2026-07-27 for the identical construct one function later in the
   same file. Next probe: a session or operator with authority over
   `inline_asm_canonical.txt` applies `memory/grind/func_80052B00/candidate.c`
   over `src/text1b.c:10969-10994`, adds the entry, runs
   `retire func_80052B00` (drops `regfix.txt:3411`) and `verify-oracle`. The
   disposition call belongs to the driver/owner, not to a grind session in a
   non-escalation modality.

2. **If the canonical disposition is refused,
   `memory/grind/func_80052B00/best_pure_c_fused8_floor17.c` is still the body to
   ship** — pin-free, honest 17, strictly less cheat surface than the pinned HEAD
   body (which needs eight `register asm("$N")` pins PLUS `regfix.txt:3411`).
   Next probe (operator only): splice it in, run `sandbox func_80052B00` WITHOUT
   `--disable all` to see what the enabled rule set achieves, find the minimal
   rule set that byte-matches, and `verify-oracle`.

3. **Modality ladder status.** recon (s1), structural (s2, s3) and permuter (s4)
   are all measured dead or exhausted. Forensics / rederive / synthesis remain
   formally untried but operate on constructs this body does not contain (no
   arithmetic, no control flow, no calls, no data layout, no library-provenance
   surface beyond the already-identified `SetRotMatrix`+`SetTransMatrix` fusion).
   The one remaining CHEAP measurement anybody could still want is an
   instrumented-cc1 `-da` `.greg`/`.lreg` dump (per [[instrumented-cc1-location]]:
   `tools/gcc-2.7.2/cc1`, NOT `build/cc1`) showing plain ascending hard-register
   assignment for the eight load pseudos — a corroboration of H7's mechanism, not
   a new axis, and it cannot reach 0 because H1 still stands.

## [s4] A decomp-permuter campaign on func_80052B00 finds a C form scoring below the honest floor of 17 (permuter base score 140).
- mechanism: Sessions 2 and 3 argued the permuter was futile here because its mutation space (statement reordering, temp introduction, type changes) had been hand-enumerated across 32 forms; that is an argument, not a measurement. The falsifiable prediction is that the permuter has no mutation that names a hard register and none that can defeat reorg.c's unconditional ASM stop in stop_search_p, so it cannot touch either component of the residual (16 wrong-GPR instructions + 1 unfilled jr $ra delay slot) and must return nothing below 140.
- probe: Two telemetry-tracked campaigns via tools/permuter_campaign.py (owner directive 2026-07-07), each waited on in-turn with `wait` and harvested with `--stop`. Chassis 1 `fused8-r-constraints` = memory/grind/func_80052B00/best_pure_c_fused8_floor17.c verbatim. Chassis 2 `struct-fields-direct-rvalue-fused8` = a structurally different reseed per the fresh-seed rule: GteCtl struct field references fed directly as the fused asm's eight operands with zero named temporaries. Workspace built by tmp/grind/func_80052B00/s4/mkws.sh running the real cc1 -> prologue_fix -> maspsx -> multu_pad chain with function-region extraction, validated against asm/funcs/func_80052B00.s.
- result: Campaign 1: 31,871 iterations / 1,139 s / 8 workers - ZERO finds, not even a score tie. Campaign 2: 85,443 iterations / 1,141 s - exactly one find, at score 140 (a tie with base), at 12 s in, consisting of an appended dead `int new_var; if (new_var) { new_var = 1; }` that changes no emitted instruction and is an uninitialized dead-local cheat shape (banked as rejected/permuter-tie-dead-if-noise.c, not proposed). 117,314 iterations across two chassis, nothing below 140. Campaign 1's zero-tie result additionally shows the space has no interior: a 17-instruction body with no arithmetic, control flow or calls admits almost no neutral mutation, so there is no basin to descend.
- verdict: KILLED

## [s4] func_80052B00's codegen is independent of its TU context, so a minimal standalone TU is a faithful experiment vehicle.
- mechanism: The function is a leaf with no calls, no globals, no statics and no data references, so none of the context-sensitive cc1 state (GP-relative decisions, prior-declaration ordering, the scheduler-state effects noted in cc1-first-pass-scheduler-bug) has any input here.
- probe: tmp/grind/func_80052B00/s4/mkws.sh compiles a TU containing only `typedef signed int s32;` plus the session-3 fused body through the real stage chain and objdumps it; compared against the in-tree sandbox dumps in tmp/grind/func_80052B00/s3/form_disasm.txt.
- result: Identical body - lw v0,0(a0) .. lw t2,28(a0) / ctc2 v0,$0 .. ctc2 t2,$7 / jr ra / nop. Standalone-TU experiments on this function are faithful and cost a fraction of an in-tree sandbox cycle.
- verdict: CONFIRMED

## [s4] H7 re-tested on a 19th spelling: removing the named temporaries entirely (struct field references fed directly as asm operands) moves the allocator off $v0.
- mechanism: If the allocator's starting hard register were sensitive to the presence or shape of the C-level temporaries, a chassis with no temporaries at all would differ from the eight-locals chassis.
- probe: Compiled the struct-fields-direct-rvalue-fused8 chassis (tmp/grind/func_80052B00/s4/ws2/base.c) through the real stage chain and objdumped it.
- result: Identical register set {v0,v1,a1,a2,a3,t0,t1,t2} and identical 18-instruction body. Consistent with the mechanism from session 3 (no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so find_free_reg scans hard registers in ascending order). H7 stands at 19 spellings.
- verdict: KILLED

## [s4] H1 re-tested across machine-generated spellings: if any C spelling could get an __asm__ ctc2 into the jr $ra delay slot, ~117k permuter samples would find it.
- mechanism: reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots at any asm insn.
- probe: Both s4 campaigns (117,314 iterations total) with the emitted objects scored against target.o.
- result: No output with a filled delay slot. Campaign 1 produced no outputs at all; campaign 2's single output is code-identical to base, which ends `jr ra ; nop`. Running total: 32 hand-measured spellings plus 117,314 permuter iterations, zero delay-slot fills.
- verdict: CONFIRMED

## [s4] A decomp-permuter campaign on func_80052B00 finds a C form scoring below the honest floor of 17 (permuter base score 140).
- mechanism: Sessions 2 and 3 argued the permuter was futile here because its mutation space (statement reordering, temp introduction, type changes) had been hand-enumerated across 32 forms - but that is an argument, not a measurement, and the driver correctly mandated the modality. The falsifiable prediction: the permuter has no mutation that names a hard register, and none that can defeat reorg.c:730-735 stop_search_p's unconditional ASM stop, so it cannot touch EITHER component of the residual (16 wrong-GPR instructions + 1 unfilled jr $ra delay slot) and must return nothing below 140.
- probe: Two telemetry-tracked campaigns via tools/permuter_campaign.py (owner directive 2026-07-07), each launched with -j 8, waited on IN-TURN with `wait`, and harvested with `--stop`. Workspace built by tmp/grind/func_80052B00/s4/mkws.sh running the real stage chain (tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -> tools/prologue_fix.py -> tools/maspsx/maspsx.py --aspsx-version=2.34 with every Makefile gate list -> tools/multu_pad.py), extracting the .ent/.end func_80052B00 region and assembling with mipsel-linux-gnu-as -march=r3000 -no-pad-sections -O1 -G0; target.o = decomp-permuter prelude (gp=64 stripped) + asm/funcs/func_80052B00.s. Chassis 1 `fused8-r-constraints` = memory/grind/func_80052B00/best_pure_c_fused8_floor17.c verbatim. Chassis 2 `struct-fields-direct-rvalue-fused8` = a fresh-seed reseed with a materially different chassis (GteCtl struct field references fed DIRECTLY as the fused asm's eight operands, zero named temporaries - a pycparser surface never swept in s1-s3).
- result: Campaign 1: 31,871 iterations / 1,139 s / 8 workers - ZERO finds, not even a score tie. Campaign 2: 85,443 iterations / 1,141 s - exactly ONE find, at score 140 (a TIE with base), 12 s after launch, consisting of an appended dead `int new_var; if (new_var) { new_var = 1; }` that changes no emitted instruction and is an uninitialized dead-local cheat shape; banked as memory/grind/func_80052B00/rejected/permuter-tie-dead-if-noise.c, not proposed. 117,314 iterations across two chassis, nothing below 140. Campaign 1's zero-TIE result is itself informative: a 17-instruction body with no arithmetic, no control flow and no calls admits almost no neutral mutation, so the search space has no interior and there is no basin for sampling to descend.
- verdict: KILLED

## [s4] func_80052B00's codegen is independent of its TU context, so a minimal standalone TU is a faithful (and far cheaper) experiment vehicle than an in-tree sandbox cycle.
- mechanism: The function is a leaf with no calls, no globals, no statics and no data references, so none of the context-sensitive cc1 state (GP-relative decisions, prior-declaration ordering, the scheduler-state effects noted in cc1-first-pass-scheduler-bug) has any input here.
- probe: mkws.sh compiles a TU containing only `typedef signed int s32;` plus the session-3 fused body through the real stage chain and objdumps the result; compared against the in-tree sandbox dumps in tmp/grind/func_80052B00/s3/form_disasm.txt.
- result: Identical body: lw v0,0(a0) .. lw t2,28(a0) / ctc2 v0,$0 .. ctc2 t2,$7 / jr ra / nop. src/text1b.c's ~125k tokens of preceding declarations contribute nothing to this function's codegen.
- verdict: CONFIRMED

## [s4] H7 re-test on a 19th spelling: removing the named temporaries entirely (struct field references fed directly as the asm operands) moves the allocator off $v0 as its first hard register.
- mechanism: If the allocator's starting hard register were sensitive to the presence or shape of C-level temporaries, a chassis with no temporaries at all would differ from the eight-locals chassis. Session 3's mechanism predicts it will not: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc's find_free_reg scans hard registers in plain ascending number order and takes the first non-conflicting one.
- probe: Compiled tmp/grind/func_80052B00/s4/ws2/base.c (the struct-fields-direct-rvalue-fused8 chassis) through the real stage chain and objdumped it.
- result: Identical register set {v0,v1,a1,a2,a3,t0,t1,t2} and an identical 18-instruction body. H7 now holds across 19 distinct spellings.
- verdict: KILLED

## [s4] H1 re-test across machine-generated spellings: if any C spelling could get an __asm__ ctc2 into the jr $ra delay slot, ~117k permuter samples would find it.
- mechanism: reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots at the first asm insn scanning back from the jump; ctc2 has no C analog so the target's delay-slot instruction can only come from an __asm__ block.
- probe: Both session-4 campaigns (117,314 iterations total), with every emitted object scored against target.o built from asm/funcs/func_80052B00.s.
- result: No output with a filled delay slot. Campaign 1 produced no outputs at all; campaign 2's single output is code-identical to its base, which ends `jr ra ; nop`. Running total across sessions 1-4: 32 hand-measured spellings plus 117,314 permuter iterations, zero delay-slot fills.
- verdict: CONFIRMED

## Session 5 (permuter — DIRECTED, 2026-08-01) — floor 17, unmoved

### H10 — KILLED (the DIRECTED permuter sub-modality is dead, and load order only ever hurts)
**Statement.** Some ordering of the eight load statements, crossed with some
asm-operand shape of the fused cop2 write, produces a form below the honest
floor of 17 (permuter base score 140).

**Mechanism (why this was worth measuring even after s4).** Session 4 killed the
UNDIRECTED permuter axis with 117,314 iterations of random mutation. Random
mutation and PERM_*-directed enumeration are different searches: the former
samples an unstructured neighbourhood of one base, the latter makes
decomp-permuter walk a cross-product the author names. The s1-s3 hand sweeps
covered exactly TWO of the 40,320 possible load orderings (ascending and
descending), and the four fused asm shapes had each been measured at 17 only
under ascending loads. So there was a genuinely uncovered region: shape × order.
H7's mechanism predicts it is empty — with no `REG_ALLOC_ORDER` in
`tools/gcc-2.7.2/config/mips/mips.h`, local-alloc's `find_free_reg` scans hard
registers ascending from `$2`, so reordering the loads can only permute the
ASSIGNMENT within a fixed register set, never change the set — but a prediction
is not a measurement.

**Probe.** `tmp/grind/func_80052B00/s5/ws3/base.c`, a hand-authored directed
chassis per `.claude/rules/permuter-directives.md` (`tools/permuter_annotate.py`
has no applicable hint — its four slugs all need a pin, multiple returns, a
rotate or a loop, none of which this body contains). `PERM_LINESWAP` over the
eight `tN = matrix[N];` statements (8! = 40,320 orderings) × `PERM_GENERAL` over
four asm shapes (eight `"r"` inputs / eight `"d"` inputs / eight `"r"` plus a
ninth `"r"(matrix)` / a 7+1 split with the CR7 write in its own trailing
`__asm__`) = 161,280 combinations, which the permuter enumerates
(`Will run for 161280 iterations`). Workspace = session 4's validated harness
with only `base.c` replaced. Launched with `tools/permuter_campaign.py launch
-j 8` (label `directed-lineswap8-x-asmshape4`), waited on IN-TURN with `wait`,
harvested with `--stop`.

**Result.** 46,653 iterations / 1,869 s / 8 workers — **zero outputs**. The
score distribution over the 46,658 logged samples is strictly BIMODAL: 140
(= base = the honest floor of 17) in 35,111 samples, 180 (= base + 40 = eight
additional register-name mismatches at the permuter's 5-points-per-register
weight) in 11,547, nothing in between and nothing below 140. Load order is
therefore the ONE lever in this space that moves the score at all — and it moves
it only upward, by permuting which pseudo lands on which ascending hard
register, never moving the set off `{v0,v1,a1,a2,a3,t0,t1,t2}`.

**Verdict: KILLED.** Both permuter sub-modalities are now closed by measurement:
undirected (s4, 117,314 iterations, two chassis) and directed (s5, 46,653
iterations over an exhaustively-defined 161,280-point cross-product). Do not run
a third permuter campaign on func_80052B00.

### H7 — re-CONFIRMED (as a KILL) with a much larger sample
Session 3 measured the register-set invariant across 18 spellings and session 4
across 19. Session 5 adds ~46.6k machine-generated spellings whose scores never
drop below base, and whose only upward excursion (+40 = 8 registers) is exactly
the signature of a permuted assignment within an unchanged set. The
allocator-start-point argument is now measured, not just read out of `mips.h`.

### H1 — re-CONFIRMED
No form in the directed space filled the `jr $ra` delay slot; a fill would have
scored below 140 and been saved, and zero outputs were produced. Running total
across sessions 1-5: 32 hand-measured spellings plus 163,967 permuter
iterations, zero delay-slot fills.

## Live frontier after session 5 (highest value first)

1. **Canonical-asm authorization remains the disposition; the evidence base is
   now complete on every automated axis.** The honest pure-C body is
   instruction-for-instruction isomorphic to the target; the residual is
   register naming (H7 — mechanically unreachable, now measured across 19 hand
   spellings plus ~46.6k directed machine spellings) plus the delay slot (H1 —
   provably unreachable, `reorg.c:730-735`). Structural is dead (s2/s3),
   undirected permuter is dead (s4), directed permuter is dead (s5). Precedent
   unchanged and citable: `func_80052B44`, `src/text1b.c:10995`,
   `inline_asm_canonical.txt:340`, authorized 2026-07-27 for the identical
   construct one function later in the same file. Next probe: a session or
   operator with authority over `inline_asm_canonical.txt` applies
   `memory/grind/func_80052B00/candidate.c` over `src/text1b.c:10969-10994`,
   adds the entry, runs `retire func_80052B00` (drops `regfix.txt:3411`) and
   `verify-oracle`. The disposition call belongs to the driver/owner, not to a
   grind session in a non-escalation modality.

2. **If the canonical disposition is refused,
   `memory/grind/func_80052B00/best_pure_c_fused8_floor17.c` is still the body
   to ship** — pin-free, honest 17, strictly less cheat surface than the pinned
   HEAD body (eight `register asm("$N")` pins PLUS `regfix.txt:3411`). Next
   probe (operator only, a surface a grind session may not touch): splice it in,
   run `sandbox func_80052B00` WITHOUT `--disable all` to see what the enabled
   rule set achieves, find the minimal rule set that byte-matches, and
   `verify-oracle`.

3. **No automated or structural search axis remains untried.** recon (s1),
   structural (s2, s3), permuter-undirected (s4) and permuter-directed (s5) are
   all measured dead or exhausted. Forensics / rederive / synthesis operate on
   constructs this body does not contain (no arithmetic, no control flow, no
   calls, no data layout, no library-provenance surface beyond the
   already-identified `SetRotMatrix`+`SetTransMatrix` fusion). The only cheap
   step anyone could still want is a CORROBORATION, not a new axis: an
   instrumented-cc1 `-da` `.lreg`/`.greg` dump (`tools/gcc-2.7.2/cc1`, NOT
   `build/cc1`, per [[instrumented-cc1-location]]) showing plain ascending
   hard-register assignment for the eight load pseudos. It cannot reach 0
   because H1 still stands.

## [s5] Some ordering of the eight load statements, crossed with some asm-operand shape of the fused cop2 write, produces a form below the honest floor of 17 (permuter base score 140).
- mechanism: Session 4 killed the UNDIRECTED permuter axis with 117,314 iterations of random mutation, but random mutation and PERM_*-directed enumeration are different searches. The s1-s3 hand sweeps covered exactly TWO of the 40,320 possible load orderings (ascending, descending), and the four fused asm shapes had each been measured at 17 only under ascending loads, so shape x order was a genuinely uncovered region. H7's mechanism predicts it is empty (no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so find_free_reg scans hard registers ascending from $2 and load order can only permute the assignment within a fixed set) - but a prediction is not a measurement.
- probe: Hand-authored directed chassis tmp/grind/func_80052B00/s5/ws3/base.c per .claude/rules/permuter-directives.md (tools/permuter_annotate.py has no applicable hint: its four slugs need a register pin, multiple returns, a rotate or a loop). PERM_LINESWAP over the eight `tN = matrix[N];` statements (8! = 40,320 orderings) x PERM_GENERAL over four asm shapes (eight "r" inputs, eight "d" inputs, eight "r" plus a ninth "r"(matrix) keeping the base pointer live, and a 7+1 split with the CR7 write in its own trailing __asm__) = 161,280 combinations, which decomp-permuter enumerates ("Will run for 161280 iterations", base score 140). Workspace = session 4's validated harness (compile.sh / target.o / settings.toml / prelude) with only base.c replaced. Launched via tools/permuter_campaign.py launch -j 8 (label directed-lineswap8-x-asmshape4), waited on IN-TURN with `wait`, harvested with --stop under the fresh-seed rule.
- result: 46,653 iterations / 1,869 s / 8 workers - ZERO outputs. The score distribution over the 46,658 logged samples is strictly BIMODAL: 140 (= base = honest floor 17) in 35,111 samples and 180 (= base + 40 = eight additional register-name mismatches at 5 points per register) in 11,547, with nothing in between and nothing below 140. Load order is the one lever in this space that moves the score at all, and it moves it only upward, by permuting which pseudo lands on which ascending hard register; it never moves the set off {v0,v1,a1,a2,a3,t0,t1,t2}. Both permuter sub-modalities are now closed by measurement: undirected (s4, 117,314 iterations, two chassis) and directed (s5, 46,653 over a 161,280-point cross-product).
- verdict: KILLED

## [s5] H7 re-tested at scale: if the allocator's register SET were sensitive to the order in which the eight loads are written, an enumeration of all 8! orderings would find an ordering that reaches {t0..t7}.
- mechanism: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc's find_free_reg scans hard registers in plain ascending number order from $2 and takes the first non-conflicting one. Reordering the loads changes which pseudo is allocated first, i.e. the ASSIGNMENT within the set, but nothing about the order in which hard registers are offered.
- probe: The session-5 directed campaign's PERM_LINESWAP dimension (40,320 orderings, ~46.6k sampled combinations across four asm shapes).
- result: Exactly two score values ever observed - 140 (base) and 180 (base + 40 = eight more register mismatches). No ordering lowered the score; the +40 excursion is precisely the signature of a permuted assignment inside an unchanged register set. H7 now holds across 19 hand spellings plus ~46.6k machine-generated ones.
- verdict: KILLED

## [s5] H1 re-tested across the directed space: if any load ordering or asm-operand shape could get an __asm__ ctc2 into the jr $ra delay slot, an enumeration of 161,280 shape-x-order combinations would find it.
- mechanism: reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots at the first asm insn scanning back from the jump; ctc2 has no C analog, so the target's delay-slot instruction can only come from an __asm__ block.
- probe: The session-5 directed campaign (46,653 iterations of the 161,280-point cross-product); a filled delay slot would have scored below 140 and been saved as an output.
- result: Zero outputs. Running total across sessions 1-5: 32 hand-measured spellings plus 163,967 permuter iterations, zero delay-slot fills.
- verdict: CONFIRMED

## [s5] Some ordering of the eight load statements, crossed with some asm-operand shape of the fused cop2 write, produces a C form below the honest floor of 17 (permuter base score 140).
- mechanism: Session 4 killed the UNDIRECTED permuter axis with 117,314 iterations of random mutation, but random mutation and PERM_*-directed enumeration are different searches: the former samples an unstructured neighbourhood of one base, the latter makes decomp-permuter walk a cross-product the author names. The s1-s3 hand sweeps covered exactly TWO of the 40,320 possible load orderings (ascending and descending), and the four fused asm shapes had each been measured at 17 only under ascending loads, so shape x order was a genuinely uncovered region. H7's mechanism predicts it is empty - tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc's find_free_reg scans hard registers in plain ascending number order from $2 and load order can only permute the assignment within a fixed set - but a prediction is not a measurement, which is exactly the gap this session was mandated to close.
- probe: Hand-authored directed chassis tmp/grind/func_80052B00/s5/ws3/base.c per .claude/rules/permuter-directives.md (tools/permuter_annotate.py has no applicable hint: its four slugs - register-asm-pins, shared-end-label, loop-rotation-two-shift, loop-counter-fills-load-delay - each require a register pin, multiple return paths, a rotate or a loop, none of which this body contains). PERM_LINESWAP over the eight `tN = matrix[N];` statements (8! = 40,320 orderings) crossed with PERM_GENERAL over four asm-operand shapes: eight "r" inputs (the s3/s4 best pure-C body), eight "d" inputs, eight "r" inputs plus a ninth "r"(matrix) keeping the base pointer live through the asm, and a 7+1 split with the CR7 write in its own trailing __asm__. That is 161,280 combinations and decomp-permuter confirmed it enumerates rather than samples ('Will run for 161280 iterations', '[func_80052B00] base score = 140'). Workspace = session 4's validated harness (compile.sh running the real cc1 -> prologue_fix -> maspsx -> multu_pad chain, target.o, settings.toml, prelude_r3k.inc) with only base.c replaced. Launched via tools/permuter_campaign.py launch -j 8 (label directed-lineswap8-x-asmshape4), waited on IN-TURN with `wait`, harvested with --stop under the fresh-seed rule.
- result: 46,653 iterations / 1,869 s / 8 workers - ZERO outputs, not one form at or below base was ever saved. The score distribution over the 46,658 logged samples is strictly BIMODAL: 140 (= base = the honest floor of 17) in 35,111 samples and 180 (= base + 40 = eight additional register-name mismatches at the permuter's 5-points-per-register weight) in 11,547, with nothing in between and nothing below 140. Load ORDER is therefore the one lever in this entire space that moves the score at all - a genuinely new fact, since s1-s3 had only two data points on it - but it moves the score only UPWARD, by permuting which pseudo lands on which ascending hard register, and it never moves the register SET off {v0,v1,a1,a2,a3,t0,t1,t2}.
- verdict: KILLED

## [s5] H7 re-test at scale: if the allocator's register SET were sensitive to the order in which the eight loads are written, an enumeration of all 8! orderings would find one that reaches the target's {t0..t7}.
- mechanism: No REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so local-alloc's find_free_reg offers hard registers in plain ascending number order from $2 and takes the first non-conflicting one. Reordering the loads changes which pseudo is allocated first (the assignment within the set) but nothing about the order in which hard registers are offered.
- probe: The PERM_LINESWAP dimension of the session-5 directed campaign: 40,320 orderings, ~46.6k sampled combinations spread across all four asm shapes.
- result: Exactly two score values ever observed - 140 and 180. No ordering lowered the score, and the +40 excursion is precisely the signature of a permuted assignment inside an unchanged register set (eight registers x 5 points). H7 now holds across 19 hand spellings plus ~46.6k machine-generated ones; the allocator-start-point argument is measured, not merely read out of the machine description.
- verdict: KILLED

## [s5] H1 re-test across the directed space: if any load ordering or asm-operand shape could get an __asm__ ctc2 into the jr $ra delay slot, an enumeration of 161,280 shape-x-order combinations would find it.
- mechanism: tools/gcc-2.7.2/reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots at the first asm insn scanning back from the jump; ctc2 has no C analog, so the target's delay-slot instruction can only originate in an __asm__ block.
- probe: The session-5 directed campaign (46,653 iterations); a filled delay slot would have scored below 140 and been saved as an output.
- result: Zero outputs. Running total across sessions 1-5: 32 hand-measured spellings plus 163,967 permuter iterations, zero delay-slot fills.
- verdict: CONFIRMED

## Session 6 (forensics, 2026-08-01) — floor 17, unmoved; two mechanisms RENAMED and one residual re-closed by arithmetic

Vehicle: the honest floor-17 fused body (`best_pure_c_fused8_floor17.c`) compiled
in a minimal standalone TU — faithful per H9 (s4) — with the instrumented cc1
(`tools/gcc-2.7.2/cc1`, NOT `build/cc1`, per [[instrumented-cc1-location]]) and
`-da`, producing the full RTL pass dump set (.rtl .jump .cse .loop .cse2 .flow
.combine .sched .lreg .greg .sched2 .dbr) in
`tmp/grind/func_80052B00/s6/dumps/`. Four differential controls (ctlA-ctlG) were
compiled the same way; all raw output is banked in the s6 scratch directory.

### H12 — CONFIRMED, and it CORRECTS the H7 mechanism named in sessions 3-5
**Statement.** The eight loads never reach the register allocator at all: GCC's
`combine` pass substitutes each `(mem)` directly into the fused asm's input
operands, so the register names in the emitted body are chosen by RELOAD, not by
local-alloc's `find_free_reg`.

**Probe.** Count surviving `(set (reg ...))` insns pass by pass in the dumps.

**Result.** `.rtl`/`.jump`/`.cse`/`.loop`/`.flow` each have 9; `.combine` has 0.
The `.lreg` (local-alloc) dump for this function contains exactly ONE insn — the
`asm_operands` — whose eight operands are `(mem:SI (reg:SI 4 a0))`,
`(mem/s:SI (plus (reg:SI 4 a0) (const_int 4)))`, … `(const_int 28)`. There are no
pseudos left to allocate. The `.greg` dump then prints
`;; Need 8 regs of class GR_REGS (for insn 34)` followed by
`Spilling reg 2. / 3. / 5. / 6. / 7. / 8. / 9. / 10.` and
`;; Hard regs used: 2 3 4 5 6 7 8 9 10`. The eight `lw` instructions in the final
asm are reload-generated reloads of those MEMs into spill registers.

**Verdict: CONFIRMED.** Sessions 3-5 attributed the register set to
"local-alloc's `find_free_reg` walking hard registers in ascending order". The
observable set is the same, but the pass is wrong: with combine having eaten the
loads, local-alloc has no work here. Any future attempt to steer the registers
must aim at reload, or at defeating combine first (see H14).

### H13 — CONFIRMED (the exact reload decision, with source line)
**Statement.** The observed spill order {2,3,5,6,7,8,9,10} is
`order_regs_for_reload()` at `tools/gcc-2.7.2/reload1.c:3606`, `#else` branch.

**Mechanism (compiler source).** With no `REG_ALLOC_ORDER` in
`tools/gcc-2.7.2/config/mips/mips.h`, that function fills `potential_reload_regs`
in two ascending-regno passes: first every hard reg with `uses == 0` that is
`call_used_regs`, then every hard reg with `uses == 0` that is not. Regs that are
already used are appended afterwards, sorted by use count. `reload1.c:486` copies
`regs_ever_live` into `regs_explicitly_used`, and `:3651` gives any explicitly-used
reg `large + 1` uses AND sets `bad_spill_regs` — which is precisely why `$4` (the
live incoming base pointer) is skipped in the middle of the ascending run. So the
first eight spill choices in a computation-free leaf are forced to be
$v0,$v1,$a1,$a2,$a3,$t0,$t1,$t2.

**Verdict: CONFIRMED.**

### H11 — CONFIRMED: the target's $t0..$t7 IS reachable in pure C, at a cost the target cannot pay
**Statement.** If $2,$3,$5,$6,$7 are occupied by other live pseudos across the
asm, reload's first eight choices become $8..$15 — the target's exact register
set.

**Probe.** `tmp/grind/func_80052B00/s6/ctlE.sh` — the fused body plus five
additional locals loaded from `matrix[8..12]` and stored back after the asm.
Control `ctlD.sh` is the same idea with a second pointer parameter.

**Result.** ctlE emits `lw $8,0($4) … lw $15,28($4)` / `ctc2 $8,$0 … ctc2 $15,$7`
— identical to the target in opcode, order, offset AND register name; the spill
list is `8,9,10,11,…,15`. ctlD, whose second parameter occupies $5, lands one
slot further along at {9,10,11,12,13,14,15,24}, confirming the choice is purely
positional in the ascending scan. Cost: ctlE emits 5 extra `lw` + 5 extra `sw`,
i.e. 27 instructions where the target has 17.

**Verdict: CONFIRMED as a mechanism, DEAD as a form.** Banked at
`rejected/regocc5-reproduces-t0t7-costs-10-extra-insns.c`. This upgrades the
register residual from "no spelling found in 19 hand forms + 163k permuter
iterations" (an absence of evidence) to a closed arithmetic argument: the set is
reachable only from a register-pressure state that adds >= 10 instructions to a
body that must not grow by one.

### H14 — KILLED (there is no zero-instruction way to occupy $2,$3,$5,$6,$7)
**Statement.** Declaring extra unused register parameters marks $a1-$a3 as
explicitly used, pushing them into `bad_spill_regs` at no instruction cost, and
moves reload off them.

**Mechanism.** `regs_explicitly_used` is a copy of `regs_ever_live`; if
`assign_parms` marked incoming argument registers live even for unused
parameters, the deprioritisation at `reload1.c:3651` would fire for free.

**Probe.** `tmp/grind/func_80052B00/s6/ctlF.sh` — `ctlF(s32 *matrix, s32 p1,
s32 p2, s32 p3)` and `ctlG(s32 *matrix, s32 p1)`, both with the body untouched.

**Result.** Both emit the unchanged `lw $2,0($4) … lw $10,28($4)` /
`ctc2 $2,$0 … ctc2 $10,$7` and both `.greg` dumps print the unchanged spill list
`2,3,5,6,7,8,9,10`. Unused register parameters do not set `regs_ever_live`.

**Verdict: KILLED.** With this, H11's cost is not merely the cost of one
implementation — it is the cost of the only implementation.

### H15 — CONFIRMED: combine can be defeated, and it does not help
**Statement.** Forcing each loaded value to have two uses stops combine
substituting the MEM into the asm, sending the eight loads through local-alloc
instead of reload — a genuinely different allocation path.

**Probe.** `ctlC` in `tmp/grind/func_80052B00/s6/controls.sh`: a second, EMPTY
`__asm__` consuming the same eight values. PROBE ONLY, never a proposed form — an
empty asm with register inputs is a scheduling barrier and a cheat under the
standing policy.

**Result.** `.combine` retains all 9 `(set (reg ...))`; the `.greg` dump shows no
`Spilling` lines at all (nothing needed reloading) and `Hard regs used: 2 3 4 5 6
7 8 9 10`. Local-alloc assigns the eight pseudos DESCENDING —
`lw $10,0($4) / lw $9,4($4) / … / lw $2,28($4)`, `ctc2 $10,$0 … ctc2 $2,$7`. Same
SET, reverse order, still no $11-$15. (This also explains session 3's
volatile-pointer PROBE scoring 15: volatile MEMs are non-substitutable, so that
probe was measuring the local-alloc path, not a volatile magic.)

**Verdict: CONFIRMED.** Both allocation paths draw from the same candidate pool
{2,3,5,6,7,8,9,10}; neither can reach $t3..$t7 without the H11 occupancy.

### H1 — upgraded a third time: now a pass-level DIFFERENTIAL, not just a source read or an output observation
**Statement (unchanged).** GCC 2.7.2 will not put the `__asm__` `ctc2` in the
`jr $ra` delay slot.

**New probe (s6).** Read the compiler's own reorg counters, and run a control
that differs from the honest body ONLY in that the last insn before the return is
a plain C store rather than an asm.

**Result.** Honest body, `dumps/fused8.c.dbr` header:
`;; Reorg pass #1: … 3 insns needing delay slots / 3 got 0 delays`, and the
return survives as a bare
`(jump_insn 68 41 69 (parallel[ (return) (use (reg:SI 31 ra)) ]) 305
{return_internal})` — never wrapped in a `(sequence)`. cc1's assembly output is
`j $31` with no `.set noreorder` block at all, i.e. the slot is handed downstream
unfilled. Control ctlA (identical body + `matrix[0] = 0;` after the asm) flips the
counter to `2 got 0 delays, 1 got 1 delays`, produces a delay-slot `(sequence)`,
and emits `.set noreorder / j $31 / sw $0,0($4)`. Control ctlB (no inline asm
anywhere) likewise fills. Control ctlC (asm present, combine defeated, loads as
real pseudos) is back to `3 got 0 delays`.

**Verdict: CONFIRMED.** The delay slot of THIS function shape is fillable by
reorg — the sole blocker is that the only candidate insn is an ASM, exactly as
`reorg.c:730-735` `stop_search_p` specifies. Running total across sessions 1-6:
32 hand spellings, 163,967 permuter iterations, and now a compiler-internal
counter with a matched control, all agreeing.

## Live frontier after session 6 (highest value first)

1. **Canonical-asm authorization remains the disposition, and forensics adds an
   affirmative provenance argument rather than one more null result.** Session 6
   shows GCC CAN name $t0..$t7 for these eight values, but only from a state that
   costs >= 10 extra instructions (H11 + H14), and CAN fill this jr's delay slot,
   but only when the candidate insn is not an asm (H1 differential). The target
   does both at once, in 17 instructions, with an instruction (`ctc2`) that has no
   C analog. No compiler state exists in this toolchain that produces the target
   bytes from C. Precedent unchanged and citable: `func_80052B44`,
   `src/text1b.c:10995`, `inline_asm_canonical.txt:340`, authorized 2026-07-27 for
   the identical construct one function later in the same file. Next probe: a
   session or operator with authority over `inline_asm_canonical.txt` applies
   `memory/grind/func_80052B00/candidate.c` over `src/text1b.c:10969-10994`, adds
   the entry, runs `retire func_80052B00` (drops `regfix.txt:3411`) and
   `verify-oracle`. The disposition call belongs to the driver/owner.

2. **If the canonical disposition is refused,
   `memory/grind/func_80052B00/best_pure_c_fused8_floor17.c` is still the body to
   ship** — pin-free, honest 17, strictly less cheat surface than the pinned HEAD
   body (eight `register asm("$N")` pins PLUS `regfix.txt:3411`). Operator-only
   next probe: splice it in, run `sandbox func_80052B00` WITHOUT `--disable all`,
   find the minimal rule set that byte-matches, `verify-oracle`.

3. **Do not re-open the register axis on the spelling dimension.** Six sessions
   have now measured it three independent ways (19 hand spellings, ~164k permuter
   iterations, and the compiler's own reload/local-alloc dumps) and the s6
   forensics identifies the ONLY lever that moves it — five-register occupancy —
   together with the arithmetic that excludes it. The one thing s6 did NOT
   exhaustively enumerate is zero-cost occupancy sources: unused parameters are
   killed (H14), and the remaining candidates (a value returned in $v0, a DImode
   return occupying $v0+$v1, a `longjmp`/`setjmp` frame, an alloca) all either
   emit instructions or are absent from a 17-instruction leaf. A future session
   wanting one last measurement here should test those four in a single ctl sweep
   rather than sweeping C spellings again.

## [s6] The eight loads never reach the register allocator: combine substitutes each (mem) directly into the fused asm's input operands, so the emitted register names are chosen by RELOAD, not by local-alloc's find_free_reg as sessions 3-5 recorded.
- mechanism: GCC 2.7.2's combine pass (try_combine) substitutes a single-use pseudo's defining set into its user. Each load is used exactly once, in the asm, so all eight are folded and deleted; local-alloc then has nothing to allocate and the eight lw instructions in the final output are reload-generated reloads of the folded MEMs into spill registers.
- probe: Compiled the honest floor-17 fused body in a minimal standalone TU (faithful per H9) with the instrumented cc1 tools/gcc-2.7.2/cc1 -da, and counted surviving (set (reg ...)) insns pass by pass across .rtl/.jump/.cse/.loop/.flow/.combine, then read .lreg and .greg. Artifacts in tmp/grind/func_80052B00/s6/dumps/.
- result: 9 sets survive through .flow; .combine has 0. The .lreg dump contains exactly one insn - the asm_operands - with eight (mem:SI (plus (reg:SI 4 a0) (const_int N))) operands. .greg prints ';; Need 8 regs of class GR_REGS (for insn 34)' then 'Spilling reg 2. 3. 5. 6. 7. 8. 9. 10.' and ';; Hard regs used: 2 3 4 5 6 7 8 9 10'. The pass named in the s3-s5 ledger (local-alloc find_free_reg) does no work on this function.
- verdict: CONFIRMED

- ## [s6] The observed spill order {2,3,5,6,7,8,9,10} is exactly order_regs_for_reload() at tools/gcc-2.7.2/reload1.c:3606, #else branch (no REG_ALLOC_ORDER in mips.h).
- mechanism: order_regs_for_reload fills potential_reload_regs in two ascending-regno passes - first every hard reg with uses == 0 that is call_used_regs, then every hard reg with uses == 0 that is not - and appends already-used regs afterwards sorted by use count. reload1.c:486 copies regs_ever_live into regs_explicitly_used and :3651 gives any explicitly-used reg large+1 uses plus a bad_spill_regs bit, which is why $4 (the live incoming base pointer) is skipped in the middle of the ascending run.
- probe: Read tools/gcc-2.7.2/reload1.c:3590-3712 and matched it against the .greg spill list for the honest body and for four controls.
- result: Source and dumps agree exactly. In a computation-free leaf the first eight spill choices are forced to $v0,$v1,$a1,$a2,$a3,$t0,$t1,$t2.
- verdict: CONFIRMED

- ## [s6] The target's $t0..$t7 register set IS reachable from pure C - it requires occupying $2,$3,$5,$6,$7 with five other live pseudos across the asm.
- mechanism: order_regs_for_reload skips hard regs that are live or explicitly used, so occupying exactly the five call-used regs below $8 makes reload's first eight choices $8..$15 = $t0..$t7.
- probe: tmp/grind/func_80052B00/s6/ctlE.sh - the fused body plus five extra locals loaded from matrix[8..12] and stored back after the asm; control ctlD.sh is the same idea with a second pointer parameter.
- result: ctlE emits lw $8,0($4) .. lw $15,28($4) / ctc2 $8,$0 .. ctc2 $15,$7 - identical to the target in opcode, order, offset AND register name - with spill list 8,9,10,11,12,13,14,15. ctlD, whose second parameter occupies $5, lands one slot further at {9,10,11,12,13,14,15,24}, showing the choice is purely positional in the ascending scan. Cost: 5 extra lw + 5 extra sw, 27 instructions against the target's 17. Banked as rejected/regocc5-reproduces-t0t7-costs-10-extra-insns.c. The register residual is now closed by arithmetic rather than by absence of evidence.
- verdict: CONFIRMED

- ## [s6] Extra unused register parameters mark $a1-$a3 explicitly used and move reload off them at zero instruction cost.
- mechanism: regs_explicitly_used is a copy of regs_ever_live taken at reload1.c:486; if assign_parms marked incoming argument registers live even for unused parameters, the bad_spill_regs deprioritisation at reload1.c:3651 would fire for free and give the H11 occupancy without emitting anything.
- probe: tmp/grind/func_80052B00/s6/ctlF.sh - ctlF(s32 *matrix, s32 p1, s32 p2, s32 p3) and ctlG(s32 *matrix, s32 p1), body untouched, both compiled with cc1 -da.
- result: Both emit the unchanged lw $2,0($4) .. lw $10,28($4) / ctc2 $2,$0 .. ctc2 $10,$7, and both .greg dumps print the unchanged spill list 2,3,5,6,7,8,9,10. Unused register parameters do not set regs_ever_live. H11's 10-instruction cost is therefore the cost of the only implementation, not of one implementation.
- verdict: KILLED

- ## [s6] Defeating combine (giving each loaded value a second use) routes the eight loads through local-alloc instead of reload and changes the register naming.
- mechanism: combine only substitutes a pseudo that dies at its single use; a second use blocks the substitution, so the loads survive as real insns and are allocated by local-alloc rather than materialised by reload. PROBE ONLY - the second use used here is an empty __asm__ with eight "r" inputs, which is a scheduling barrier and a cheat under the standing policy, and is never proposed as a form.
- probe: ctlC in tmp/grind/func_80052B00/s6/controls.sh.
- result: .combine retains all 9 (set (reg ...)); .greg has no Spilling lines at all; local-alloc assigns the eight pseudos DESCENDING - lw $10,0($4) / lw $9,4($4) / ... / lw $2,28($4), ctc2 $10,$0 .. ctc2 $2,$7. Same SET, reverse order, still nothing in $11-$15. This also explains session 3's volatile-pointer PROBE scoring 15: volatile MEMs are non-substitutable, so that probe was measuring the local-alloc path rather than any property of volatile. Both allocation paths draw from the same candidate pool and neither reaches $t3..$t7 without the H11 occupancy.
- verdict: CONFIRMED

- ## [s6] H1, third upgrade: the delay slot of this exact function shape is fillable by reorg, and the sole blocker is that the only candidate insn is an ASM.
- mechanism: reorg.c:730-735 stop_search_p returns 1 unconditionally for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots' backward scan; the falsifiable prediction is that swapping the last insn from an asm to a plain C store flips the compiler's own reorg counter from '0 delays' to '1 delays' with nothing else changed.
- probe: Read the ;; Reorg pass counters in the .dbr dumps of the honest body and of three controls (ctlA = same body plus matrix[0] = 0 after the asm; ctlB = no inline asm anywhere; ctlC = asm present, combine defeated), plus the RTL shape of the return insn and cc1's own assembly output.
- result: Honest body - '3 insns needing delay slots / 3 got 0 delays'; the return survives as a bare (jump_insn ... (parallel[(return) (use (reg:SI 31 ra))]) 305 {return_internal}), never a (sequence), and cc1 emits 'j $31' with no .set noreorder block, handing the slot downstream unfilled. ctlA - '2 got 0 delays, 1 got 1 delays', a real delay-slot (sequence), and '.set noreorder / j $31 / sw $0,0($4)'. ctlB fills likewise. ctlC is back to '3 got 0 delays'. Running total across sessions 1-6: 32 hand spellings, 163,967 permuter iterations, and now a compiler-internal counter with a matched control, all agreeing.
- verdict: CONFIRMED

## [s6] The eight loads never reach the register allocator: GCC's combine pass substitutes each (mem) directly into the fused asm's input operands, so the emitted register names are chosen by RELOAD, not by local-alloc's find_free_reg as sessions 3-5 recorded.
- mechanism: combine (try_combine) substitutes a single-use pseudo's defining set into its user. Each load is used exactly once - in the asm - so all eight are folded and deleted; local-alloc then has no pseudos to allocate, and the eight lw in the final output are reload-generated reloads of the folded MEMs into spill registers.
- probe: Compiled the honest floor-17 fused body in a minimal standalone TU (faithful per s4's H9) with the instrumented cc1 at tools/gcc-2.7.2/cc1 (NOT build/cc1) using -da, then counted surviving (set (reg ...)) insns pass by pass across .rtl/.jump/.cse/.loop/.flow/.combine and read .lreg and .greg. Scripts and dumps in tmp/grind/func_80052B00/s6/.
- result: 9 sets survive through .flow; .combine has 0. The .lreg dump contains exactly one insn - the asm_operands - whose eight operands are (mem:SI (reg:SI 4 a0)) and (mem/s:SI (plus (reg:SI 4 a0) (const_int 4..28))). .greg then prints ';; Need 8 regs of class GR_REGS (for insn 34)' followed by 'Spilling reg 2. 3. 5. 6. 7. 8. 9. 10.' and ';; Hard regs used: 2 3 4 5 6 7 8 9 10'. The observable conclusion carried since s3 was right; the pass named for it was wrong.
- verdict: CONFIRMED

## [s6] The observed spill order {2,3,5,6,7,8,9,10} is exactly order_regs_for_reload() at tools/gcc-2.7.2/reload1.c:3606, #else branch (mips.h defines no REG_ALLOC_ORDER).
- mechanism: order_regs_for_reload fills potential_reload_regs in two ascending-regno passes - zero-use call_used_regs first, then zero-use call-saved - and appends already-used regs afterwards sorted by use count. reload1.c:486 copies regs_ever_live into regs_explicitly_used, and :3651 gives any explicitly-used reg large+1 uses plus a bad_spill_regs bit, which is why $4 (the live incoming base pointer) is skipped in the middle of the ascending run.
- probe: Read tools/gcc-2.7.2/reload1.c:3590-3712 and matched the source against the .greg spill lists of the honest body and of five controls (ctlA, ctlC, ctlD, ctlE, ctlF/ctlG).
- result: Source and dumps agree exactly, including the positional shift under controls: with a second pointer parameter occupying $5 (ctlD) the eight choices become {9,10,11,12,13,14,15,24}. In a computation-free leaf the first eight spill choices are forced to $v0,$v1,$a1,$a2,$a3,$t0,$t1,$t2.
- verdict: CONFIRMED

## [s6] The target's $t0..$t7 register set is reachable from pure C - it requires occupying $2,$3,$5,$6,$7 with five other live pseudos across the asm.
- mechanism: order_regs_for_reload skips hard regs that are live or explicitly used, so occupying exactly the five call-used regs below $8 makes reload's first eight choices $8..$15, which is $t0..$t7.
- probe: tmp/grind/func_80052B00/s6/ctlE.sh - the fused body plus five extra locals loaded from matrix[8..12] and stored back after the asm, compiled with cc1 -da; control ctlD.sh is the same construction with a second pointer parameter.
- result: ctlE emits lw $8,0($4) .. lw $15,28($4) / ctc2 $8,$0 .. ctc2 $15,$7 - identical to the target in opcode, order, memory offset AND register name - with spill list 8..15. Cost: 5 extra lw + 5 extra sw, i.e. 27 instructions against the target's 17. CONFIRMED as a mechanism, DEAD as a form; banked at memory/grind/func_80052B00/rejected/regocc5-reproduces-t0t7-costs-10-extra-insns.c. This upgrades the register residual from an absence of evidence (19 hand spellings + ~164k permuter iterations found nothing) to a closed arithmetic argument.
- verdict: CONFIRMED

## [s6] Extra unused register parameters mark $a1-$a3 explicitly used, pushing them into bad_spill_regs at zero instruction cost and giving the H11 occupancy for free.
- mechanism: regs_explicitly_used is a copy of regs_ever_live taken at reload1.c:486; if assign_parms marked incoming argument registers live even for unused parameters, the deprioritisation at reload1.c:3651 would fire without emitting anything.
- probe: tmp/grind/func_80052B00/s6/ctlF.sh - ctlF(s32 *matrix, s32 p1, s32 p2, s32 p3) and ctlG(s32 *matrix, s32 p1), bodies untouched, both compiled with cc1 -da.
- result: Both emit the unchanged lw $2,0($4) .. lw $10,28($4) / ctc2 $2,$0 .. ctc2 $10,$7 and both .greg dumps print the unchanged spill list 2,3,5,6,7,8,9,10. Unused register parameters do not set regs_ever_live. The 10-instruction price of the register set is therefore the price of the only implementation, not of one implementation.
- verdict: KILLED

## [s6] Defeating combine (giving each loaded value a second use) routes the eight loads through local-alloc instead of reload and changes the register naming.
- mechanism: combine only substitutes a pseudo that dies at its single use, so a second use blocks the substitution and the loads survive as real insns for local-alloc. PROBE ONLY - the second use here is an empty __asm__ with eight r inputs, a scheduling barrier and a cheat under the standing policy, never proposed as a form.
- probe: ctlC in tmp/grind/func_80052B00/s6/controls.sh, compiled with cc1 -da.
- result: All 9 (set (reg ...)) survive .combine; .greg has no Spilling lines at all; local-alloc assigns the eight pseudos DESCENDING - lw $10,0($4) / lw $9,4($4) / ... / lw $2,28($4), ctc2 $10,$0 .. ctc2 $2,$7. Same candidate pool, reverse order, still nothing in $11-$15. This also retroactively explains session 3's volatile-pointer probe scoring 15: volatile MEMs are non-substitutable, so that probe was measuring the local-alloc path rather than any property of volatile.
- verdict: CONFIRMED

## [s6] H1, third upgrade: the jr $ra delay slot of this exact function shape is fillable by reorg, and the sole blocker is that the only candidate insn is an ASM.
- mechanism: reorg.c:730-735 stop_search_p returns 1 unconditionally for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots' backward scan. Falsifiable prediction: swapping the last insn from an asm to a plain C store, changing nothing else, flips the compiler's own reorg counter from 0 delays to 1 delay.
- probe: Read the ';; Reorg pass' counters in the .dbr dumps of the honest body and three controls - ctlA (same body plus matrix[0] = 0 after the asm), ctlB (no inline asm anywhere), ctlC (asm present, combine defeated) - plus the RTL shape of the return insn and cc1's own assembly output.
- result: Honest body: '3 insns needing delay slots / 3 got 0 delays'; the return survives as a bare (jump_insn 68 41 69 (parallel[(return) (use (reg:SI 31 ra))]) 305 {return_internal}), never wrapped in a (sequence); cc1 emits 'j $31' with no .set noreorder block, handing the slot downstream unfilled. ctlA: '2 got 0 delays, 1 got 1 delays', a real delay-slot (sequence), and '.set noreorder / j $31 / sw $0,0($4)'. ctlB fills likewise. ctlC is back to '3 got 0 delays'. Running total across sessions 1-6: 32 hand spellings, 163,967 permuter iterations, and now a compiler-internal counter with a matched control, all agreeing.
- verdict: CONFIRMED

## Session 7 (forensics, 2026-08-01) — floor 17, unmoved; the register axis is
## now PRICED rather than "unreachable", and the target is shown not to be a
## fixed point of this compiler configuration.

### H16 — CONFIRMED (the target's register set is self-inconsistent under GCC 2.7.2 reload)
**Statement.** No C source — legal-semantics or otherwise — can make this
toolchain emit func_80052B00's exact 17 instructions, because the register
allocation those 17 instructions use is unreachable *from those same 17
instructions*. The target is not a fixed point of GCC 2.7.2 + PsyQ flags.

**Mechanism (compiler source, verified verbatim this session).**
`tools/gcc-2.7.2/reload1.c:486` does
`bcopy (regs_ever_live, regs_explicitly_used, sizeof regs_ever_live)`, and
`order_regs_for_reload()` at `reload1.c:3606` (dump saved to
`tmp/grind/func_80052B00/s7/reload1_order_regs_for_reload.txt`) contains exactly
two ways for a hard register to enter `bad_spill_regs`:

```
      if (fixed_regs[i])
        { hard_reg_n_uses[i].uses += 2 * large + 2;
          SET_HARD_REG_BIT (bad_spill_regs, i); }
      else if (regs_explicitly_used[i])
        { hard_reg_n_uses[i].uses += large + 1;
          SET_HARD_REG_BIT (bad_spill_regs, i); }
```

plus `hard_reg_n_uses[]` accumulated from pseudos that local-alloc/global already
assigned to a hard register. On MIPS with these flags $2,$3,$5,$6,$7 are not
`fixed_regs`, and per the s6 CORRECTION `combine` folds all eight loads into the
single fused asm so there are no allocated pseudos to contribute uses (the .lreg
dump holds one insn). Therefore the ONLY route to `bad_spill_regs` for
$2,$3,$5,$6,$7 is `regs_ever_live[]` — i.e. those registers must appear
explicitly somewhere in this function's own RTL.

**The contradiction.** The target's 17 instructions mention exactly three things:
$a0 (= $4, the base pointer of all eight `lw`), $t0..$t7 (= $8..$15, the eight
loaded values, each used once by an `lw` and once by a `ctc2`), and $ra (= $31,
by the `jr`). It mentions $2, $3, $5, $6 and $7 **nowhere**. So a hypothetical C
source whose emitted code equals the target would have `regs_ever_live` =
{4, 8..15, 31}, `bad_spill_regs` would not contain $2,$3,$5,$6,$7, and
`order_regs_for_reload` would hand the eight reloads $2,$3,$5,$6,$7,$8,$9,$10 —
not $8..$15. The target's register naming requires occupancy that the target's
own instruction stream does not supply.

**Probe.** `tmp/grind/func_80052B00/s7/ctl_occupancy.sh` (six differential
controls compiled with the instrumented `tools/gcc-2.7.2/cc1 -da`, per
[[instrumented-cc1-location]], in the standalone TU vehicle validated as faithful
by H9) plus `tmp/grind/func_80052B00/s7/sweep_price.py` (the same six shapes
spliced into `src/text1b.c` and scored with
`sandbox func_80052B00 --disable all`). Raw output:
`s7/ctl_occupancy_out.txt`, `s7/sweep_price_results.txt`, `s7/dumps/`.

**Result.** Every construct that moves the register window emits at least one
instruction that the target does not contain, and the two constructs that reach
$t0..$t7 exactly do so by making five registers explicitly used:

| form | score | build insns | ctc2 register sequence |
|---|---|---|---|
| base-fused8 (the honest floor) | 17 | 18 | v0,v1,a1,a2,a3,t0,t1,t2 |
| ret-s32-zero | 17 | 18 | v1,a1,a2,a3,t0,t1,t2,t3 |
| ret-dimode-zero | 19 | 20 | a1,a2,a3,t0,t1,t2,t3,t4 |
| occ3-param-store | 20 | 21 | v0,v1,t0,t1,t2,t3,t4,t5 |
| occ5-param-store-dimode | **7** | 23 | **t0,t1,t2,t3,t4,t5,t6,t7** |
| occ5-locals-ctlE (the s6 shape, now scored) | 11 | 27 | t0,t1,t2,t3,t4,t5,t6,t7 |

**Verdict: CONFIRMED.** This is a second, independent impossibility proof
alongside H1. H1 says the delay-slot `ctc2` cannot come from C because reorg.c
stops at asm insns; H16 says the register NAMES cannot come from C either,
because reload's own occupancy requirement is not satisfiable by a 17-instruction
body that never mentions $2,$3,$5,$6,$7. Together they say the shipped bytes are
not compiler output from this configuration at all — which is affirmative
provenance evidence for hand-written asm, and is exactly why the HEAD body needs
eight `register asm("$N")` pins plus `regfix.txt:3411` to reproduce them.

**Acknowledged bound on the proof.** `regs_ever_live` is latched before reload
runs, so in principle an insn that exists at reload entry and is deleted
afterwards (flow2 / jump2 dead-code removal) could set occupancy for free. No C
construct that does this was found, and none is known; the loophole is recorded
rather than claimed closed. Every construct actually measured (19 hand spellings
in s1-s3, ~164k permuter iterations in s4-s5, six cc1 -da controls in s6, six
more in s7) either emits the instruction or fails to set occupancy.

### H17 — KILLED (the four constructs the s6 frontier flagged as untested)
**Statement.** One of a $v0 return value, a DImode return, setjmp, or alloca
occupies $2,$3,$5,$6,$7 at zero instruction cost, letting the eight reloads land
on $t0..$t7 for free.

**Probe.** `tmp/grind/func_80052B00/s7/ctl_occupancy.sh` — ctlH (s32 return),
ctlI (`long long` return), ctlJ (varargs, added to the list because
[[fake-varargs-explicit-homing]] is the one sanctioned family that touches
$5,$6,$7), ctlK (DImode return + varargs), ctlL (setjmp), ctlM (alloca), plus
ctlBASE. Each compiled with `tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet
-mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -da`; `.greg` spill list and
emitted body recorded per control.

**Result.**
- **ctlH (s32 return 0) is the one zero-cost construct** — it occupies $2 and
  nothing else, and its `move $2,$0` is absorbed by the previously empty `jr $ra`
  delay slot, so total emitted length is unchanged at 18. It shifts the register
  window by exactly one position ($3,$5,$6,$7,$8,$9,$10,$11) and the sandbox
  score is unchanged at 17. One register is not five.
- **ctlI (DImode return)** occupies $2 and $3 but costs +2: the two `move`s do
  not both fit the slot (`move $2,$0 / move $3,$0 / j $31`). Score 19.
- **ctlJ (varargs) is dead as an occupancy lever.** The register set came back
  UNCHANGED at $2,$3,$5..$10 despite the emitted `sw $5,4($sp) / sw $6,8($sp) /
  sw $7,12($sp)` homing stores — `setup_incoming_varargs` does not put those
  registers into `regs_ever_live` in time for `order_regs_for_reload`. It costs
  +4 and buys nothing. ctlK confirms varargs adds nothing on top of a DImode
  return. This closes the sanctioned-family idea for this function.
- **ctlL (setjmp): 38 instructions.** The call destroys the leaf; all eight
  values are spilled to the stack across it and reloaded.
- **ctlM (alloca): 45 instructions.** Forces a frame pointer and pushes the eight
  values onto callee-saved $16..$23 with a full save/restore sequence.

**Verdict: KILLED.** The last theoretical gap in the register argument is closed.
Zero-cost occupancy exists for exactly ONE of the five registers required, and
every construct that occupies the other four emits instructions. Banked:
`rejected/ret-s32-zero-only-zero-cost-occupancy-buys-one-register.c`.

### H18 — CONFIRMED, and it CORRECTS six sessions of ledger wording
**Statement.** The register residual is not "unreachable"; it is a PRICED axis,
and the price is score-POSITIVE — paying instructions to buy the target's
register names lowers the honest distance well below the standing floor of 17.

**Mechanism.** The engine's distance is a differing-instruction count. Sixteen of
the eighteen emitted instructions currently differ only in GPR name. Buying the
right names fixes sixteen points; the occupancy constructs cost one point per
extra emitted instruction. Six extra instructions therefore net ten points.

**Probe.** `sweep_price.py` — the six shapes above spliced into `src/text1b.c`
and scored with `sandbox func_80052B00 --disable all` (the same splice/restore
harness used by s3's sweep3.py, original body restored in a `finally` block).

**Result.** `occ5-param-store-dimode` scores **7** (build 23 insns) and
`occ5-locals-ctlE` — the s6 shape whose SCORE s6 never measured — scores **11**
(build 27). Both reproduce `ctc2 t0..t7` exactly. Sessions 1-6 all recorded the
register axis as mechanically unreachable; the accurate statement is that it is
reachable only by constructs that change the function's contract.

**Verdict: CONFIRMED as a measurement; both forms REJECTED as forms, and the
honest floor stays 17.** `occ5-param-store-dimode` adds three parameters no
caller passes and writes their garbage contents to `matrix[8..10]` (live memory
corruption past the eight words the real function reads) and changes the return
type from `void` to `long long`. `occ5-locals-ctlE` reads and writes
`matrix[8..12]`, memory the real function never touches. Both exist solely to
make five hard registers appear in the RTL so reload will not spill to them —
a register pin expressed through the function signature, the same intent as
`register T x asm("$N")` in one more spelling, and squarely inside the
cheats-by-any-spelling policy. Neither can reach 0 regardless: the extra
instructions have no home in the target's 17 and the delay slot is still `nop`
(H1). Banked: `rejected/occ5-param-store-dimode-score7-breaks-abi.c`.

## Live frontier after session 7 (highest value first)

1. **Canonical-asm authorization remains the disposition, and s7 upgrades the
   argument from "no C spelling found" to "the target is not a fixed point of
   this compiler".** H16 shows the shipped register allocation is unreachable
   from the shipped instruction stream under `reload1.c:3606`, independently of
   H1's delay-slot proof. Two orthogonal impossibility results plus an
   already-authorized identical-construct precedent one function later in the
   same file (`func_80052B44`, `src/text1b.c:10995`,
   `inline_asm_canonical.txt:340`, 2026-07-27). Next probe (operator / a session
   with authority over `inline_asm_canonical.txt`): apply
   `memory/grind/func_80052B00/candidate.c` over `src/text1b.c:10969-10994`, add
   the canonical entry, run `retire func_80052B00` (drops `regfix.txt:3411`) and
   `verify-oracle`.

2. **If the canonical disposition is refused,
   `memory/grind/func_80052B00/best_pure_c_fused8_floor17.c` is still the body to
   ship** — pin-free, honest 17, strictly less cheat surface than the pinned HEAD
   body. s7 changes nothing here except to confirm no cheaper honest form exists.
   Next probe (operator only): splice it in, run `sandbox func_80052B00` WITHOUT
   `--disable all`, find the minimal rule set that byte-matches, `verify-oracle`.

3. **The one loophole left in H16, recorded so nobody re-derives it and nobody
   overclaims the proof.** `regs_ever_live` is latched before reload, so an insn
   present at reload entry but deleted by a post-reload pass (flow2 / jump2)
   would set occupancy at zero emitted cost. No C construct doing this is known
   and none was found across the 19 hand spellings, ~164k permuter iterations and
   twelve cc1 -da controls measured in sessions 1-7. Next probe if anyone wants
   it: grep the post-reload passes for their deletion conditions and try to
   manufacture a C source hitting one — low expected value, and even a hit leaves
   H1's delay slot, so the floor still could not reach 0.

## [s7] H16 - No C source can make this toolchain emit func_80052B00's exact 17 target instructions, because the register allocation those instructions use is unreachable from those same instructions. The target is not a fixed point of GCC 2.7.2 + PsyQ flags.
- mechanism: tools/gcc-2.7.2/reload1.c:486 does bcopy(regs_ever_live, regs_explicitly_used, ...), and order_regs_for_reload() at reload1.c:3606 admits a hard register to bad_spill_regs by exactly two routes: fixed_regs[i], or regs_explicitly_used[i] (which gets `large + 1` uses and SET_HARD_REG_BIT(bad_spill_regs, i)). The only other input is hard_reg_n_uses[] accumulated from pseudos that local-alloc/global already assigned to hard registers - and per the s6 correction there are none here, because combine folds all eight loads into the single fused asm so .lreg holds one insn. On MIPS with these flags $2,$3,$5,$6,$7 are not fixed_regs. Therefore the ONLY route to bad_spill_regs for those five is regs_ever_live, i.e. they must appear explicitly in this function's own RTL. But the target's 17 instructions mention exactly $4 (base of all eight lw), $8..$15 (the eight values), and $31 (the jr) - $2,$3,$5,$6,$7 appear nowhere. A C source emitting the target's stream would leave all five spillable and order_regs_for_reload would hand the eight reloads $2,$3,$5,$6,$7,$8,$9,$10, which is precisely the honest output measured in every session.
- probe: Read and saved order_regs_for_reload() verbatim (tmp/grind/func_80052B00/s7/reload1_order_regs_for_reload.txt, reload1.c:3596-3670) and cross-checked against six cc1 -da differential controls (tmp/grind/func_80052B00/s7/ctl_occupancy.sh, dumps in s7/dumps/) plus the same six shapes scored in-tree with `sandbox func_80052B00 --disable all` (s7/sweep_price.py, s7/sweep_price_results.txt).
- result: Confirmed on both the source and the measurements. ctlBASE reproduces the .greg line 'Spilling reg 2. 3. 5. 6. 7. 8. ...' and emits regs $2,$3,$5..$10. Every construct measured that moves the register window does so by making additional hard registers explicitly used, and every such construct emits at least one instruction the 17-instruction target has no room for. The two constructs that reach $t0..$t7 exactly (occ5-param-store-dimode, occ5-locals-ctlE) do so by occupying exactly $2,$3,$5,$6,$7, at +6 and +10 instructions respectively. Acknowledged bound on the proof, recorded rather than claimed closed: regs_ever_live is latched before reload, so an insn present at reload entry but deleted by a post-reload pass (flow2/jump2) could in principle set occupancy for free; no C construct doing this is known and none appeared across 19 hand spellings, ~164k permuter iterations and twelve cc1 -da controls in sessions 1-7.
- verdict: CONFIRMED

## [s7] H17 - One of the four constructs the s6 frontier named as untested (a $v0 return value, a DImode return, setjmp, alloca) occupies $2,$3,$5,$6,$7 at zero instruction cost, letting the eight reloads land on $t0..$t7 for free.
- mechanism: If regs_ever_live can be set for a hard register by a construct that GCC then emits no instruction for, reload's bad_spill_regs would exclude it at no cost to the instruction count. s6 measured that extra UNUSED parameters do not set regs_ever_live (ctlF/ctlG); the four named constructs were the remaining candidates, and varargs was added to the list because fake-varargs-explicit-homing is the one sanctioned technique family that touches $5,$6,$7.
- probe: tmp/grind/func_80052B00/s7/ctl_occupancy.sh - seven controls (ctlH s32 return, ctlI long long return, ctlJ varargs, ctlK DImode return + varargs, ctlL setjmp, ctlM alloca, ctlBASE) compiled with the instrumented tools/gcc-2.7.2/cc1 at -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -da in the standalone TU vehicle validated as faithful by H9; .greg spill list and emitted body recorded per control. Raw log: s7/ctl_occupancy_out.txt.
- result: ctlH (s32 return 0) is the ONE zero-cost construct: 18 cc1 insns, regs $3,$5,$6,$7,$8,$9,$10,$11; it occupies $2 only, and its `move $2,$0` is absorbed by the previously empty jr $ra delay slot so total emitted length is unchanged - but one register is not five, and the sandbox score stays 17. ctlI (long long) occupies $2 and $3 for +2 insns (move $2,$0 / move $3,$0 / j $31; reorg filled the slot with neither), score 19. ctlJ (varargs) is DEAD as an occupancy lever: the register set came back UNCHANGED at $2,$3,$5..$10 despite emitting sw $5,4($sp) / sw $6,8($sp) / sw $7,12($sp) - setup_incoming_varargs homing does not reach regs_ever_live in time for order_regs_for_reload - and it costs +4. ctlK confirms varargs adds nothing on top of a DImode return. ctlL (setjmp) 38 insns: the call destroys the leaf and all eight values round-trip through the stack. ctlM (alloca) 45 insns: frame pointer plus callee-saved $16..$23 with full save/restore.
- verdict: KILLED

## [s7] H18 - The register residual is not 'unreachable' as sessions 1-6 all worded it; it is a PRICED axis and the price is score-POSITIVE, i.e. paying instructions to buy the target's register names lowers the honest distance well below the standing floor of 17.
- mechanism: The engine's distance is a differing-instruction count. Sixteen of the eighteen emitted instructions currently differ only in GPR name, so buying the right names is worth 16 points, while each extra emitted instruction costs 1. Six extra instructions therefore net ten points. Sessions 1-6 measured instruction cost but never scored the occupancy shapes (s6 recorded ctlE's +10 instructions and no score at all).
- probe: tmp/grind/func_80052B00/s7/sweep_price.py - six shapes spliced into src/text1b.c and scored with `sandbox func_80052B00 --disable all` using the s3 splice/restore harness (original body restored in a finally block; git status confirms src/text1b.c clean at session end).
- result: base-fused8 score=17 build=18 ctc2=v0,v1,a1,a2,a3,t0,t1,t2 | ret-s32-zero score=17 build=18 ctc2=v1,a1,a2,a3,t0,t1,t2,t3 | ret-dimode-zero score=19 build=20 | occ3-param-store score=20 build=21 | occ5-param-store-dimode score=7 build=23 ctc2=t0,t1,t2,t3,t4,t5,t6,t7 | occ5-locals-ctlE (the s6 shape, scored for the first time) score=11 build=27 ctc2=t0..t7. CONFIRMED as a measurement; both sub-17 forms REJECTED as forms, and the honest floor therefore stays 17. occ5-param-store-dimode adds three parameters no caller passes and writes their garbage contents to matrix[8..10] (live memory corruption past the eight words the real function reads) and changes the return type from void to long long; occ5-locals-ctlE reads and writes matrix[8..12]. Both exist solely to make five hard registers appear in the RTL so reload will not spill to them - a register pin expressed through the function signature, the same intent as register asm("$N") in one more spelling, squarely inside the cheats-by-any-spelling policy. Neither can reach 0 regardless: the extra instructions have no home in the target's 17 and the delay slot is still nop (H1).
- verdict: CONFIRMED

## Session 8 (rederive, 2026-08-01)

### H19 — CONFIRMED. The delay-slot `nop` in our build is emitted by maspsx, not by cc1.
Mechanism: cc1 emits a bare `j $31` with no delay-slot instruction and no
`.set noreorder` wrapper, deferring the slot to the assembler;
`tools/maspsx/maspsx/__init__.py`:1192-1195 appends `nop  # DEBUG: branch/jump`
after any branch/jump while `is_reorder` is true, and :945-948 forces
`.set noreorder` after every `.ent`, which also disables GNU `as`'s own
reorder-mode swap.
Probe: standalone stage dump, `tmp/grind/func_80052B00/s8/stages.sh` →
`fused8.cc1.s` vs `fused8.maspsx.s`.
Consequence: sessions 1-7's attribution of the nop solely to reorg.c was
incomplete. This does not weaken H1 (see H20) but it means H1 was never the
whole story of where the nop comes from.

### H20 — KILLED (the hypothesis, not the disposition). "The original assembler filled the delay slot, so the residual is a maspsx fidelity gap rather than a property of the C."
This was the live alternative H19 opened, and the only route by which the
delay-slot point could have stopped being evidence about the source. A
reordering assembler fills `j $31` slots; the `fill_delay` regfix action
emulates exactly that; four of the tree's six `fill_delay` rules are
"previous insn into the jr slot".
Probe: whole-binary census of every function ending in `jr $ra`
(`s8/slotcensus.py`, 1,369 functions). Result: 249 filled, 16 nop with a
hazardous predecessor, and **1,104 nop with a benign trivially-swappable
predecessor — 881 of them `addiu $sp,$sp,N`**.
Verdict: KILLED. ASPSX 2.34 did not fill branch delay slots; maspsx's nop is
faithful. H1's conclusion therefore stands on a complete enumeration of origins
rather than on an unexamined assumption: cc1's reorg cannot fill past an asm
insn, the assembler never filled at all, and a human is the only remaining
author of `ctc2 $t7, $7` in that slot.

### H21 — CONFIRMED. The cop2-tail population partitions by ORIGIN, and the hand-asm side is one contiguous address block.
Mechanism: functions whose delay slot our toolchain CAN reproduce (`jr; nop`)
versus those it cannot should separate by who wrote them, and they do.
Probe: `s8/cop2_partition.txt`. 17 cop2-tail functions leave the slot nop — 16
of them inside the single PsyQ libgte block 0x8007E1AC-0x8007F1A8, plus
tslDmaDrawListDelAll (closed in pure C). 5 hold the cop2 op in the slot, and all
five occupy one contiguous run of BB2's own code, 0x80052A80-0x80052BDC:
game_2d_CheckLifeGaugeNoDisp, func_80052A88, func_80052B00, func_80052B44
(already Judge-authorized canonical-asm), func_80052B7C. The four unauthorized
members each carry exactly one regfix rule, a `fill_delay` pulling the
immediately preceding instruction — four of only six such rules tree-wide.
Verdict: CONFIRMED, and it is the first AFFIRMATIVE provenance evidence on this
function; every prior session's result was a null ("no C spelling found"). The
0x80052A80-0x80052BDC block is a single authoring unit of hand-written GTE asm,
and func_80052B00 sits in its middle next to an already-authorized twin.

### H22 — KILLED. "Moving the eight loads INSIDE the asm block (the real PsyQ libgte macro shape) is a legal pure-C-plus-canonical-GTE form."
Mechanism: no form in sessions 1-7 had ever moved an instruction across the
asm boundary; the library macros put their `lw`s inside the asm with the matrix
pointer as %0, making this the natural library-transplant answer for the
rederive modality.
Probe: four forms, `s8/sweep8.py` — asmloads8, asmloads8-ret-s32,
asmloads8-mem-operand (sandbox 16, build_insns **2**) and asmloads-split
(16, build_insns **10**). The 16s are the score of a deleted body, not an
improvement; `engine.inlineasm.strip_cheat_asm_file()` on the asmloads8 body
returns `stripped_count=1` and an empty function. The block is classified
cheat-asm because `lw` is its first template instruction (`lw` ∈ CHEAT_ASM_OPS;
and `split_template` splitting on literal `\n` only means a `"\n\t"`-joined
template's later instructions read as `\tctc2` and default to cheat, so a block
is effectively classified by its first instruction).
Verdict: KILLED twice over — score-inert by construction under the honest
sandbox, and further from the target anyway (standalone it emits
`lw $9,0($4) … lw $2,28($4)`, neither $t0..$t7 nor ascending). The classifier's
verdict is substantively right: an asm block doing its own general-purpose loads
emits bytes from template text rather than from compilation.

## [s8] The delay-slot `nop` in our build is emitted by cc1's reorg pass.
- mechanism: Sessions 1-7 read tools/gcc-2.7.2/reorg.c:730-735 (stop_search_p halts fill_simple_delay_slots at any ASM_INPUT / asm_noperands insn) and attributed the unfilled slot entirely to that pass, never inspecting which pipeline stage actually writes the nop into the assembly text.
- probe: Standalone-TU stage dump of the banked floor-17 fused body (tmp/grind/func_80052B00/s8/stages.sh -> fused8.cc1.s vs fused8.maspsx.s), comparing cc1 output with post-maspsx output instruction by instruction.
- result: FALSE as stated. cc1's tail is `ctc2 $9,$6 / ctc2 $10,$7 / #NO_APP / j $31 / .end` — a BARE `j $31` with no delay-slot instruction, no nop, and no `.set noreorder` wrapper (contrast the s6 ctlA control, which emitted `.set noreorder / j $31 / sw`). cc1 defers the slot to the assembler. maspsx appends `nop  # DEBUG: branch/jump` (tools/maspsx/maspsx/__init__.py:1192-1195, only while is_reorder) and forces `.set noreorder` immediately after every `.ent` (:945-948), which additionally denies GNU as its own reorder-mode swap. The nop is a maspsx product.
- verdict: CONFIRMED

## [s8] H20 — The ORIGINAL assembler (ASPSX 2.34) filled `j $31` delay slots in reorder mode, so the target's delay-slot ctc2 needs no C explanation and the residual is a maspsx fidelity gap rather than a property of the source.
- mechanism: This is the live alternative H19 opens, and the only route by which the delay-slot point could stop being evidence about the C. A reordering assembler swaps the preceding instruction into an empty branch slot; the regfix `fill_delay` action emulates exactly that; four of the tree's six fill_delay rules are 'previous insn into the jr slot'. If true, the disposition for this function collapses from 'no C form exists' to 'our ASPSX stand-in is lossy'.
- probe: Whole-binary delay-slot census over every asm/funcs/*.s (tmp/grind/func_80052B00/s8/slotcensus.py, raw slotcensus.txt): classify all 1,369 functions ending in `jr $ra` by whether the slot is filled and, when it is `nop`, whether the immediately preceding instruction was benign and trivially swappable.
- result: KILLED. 249 filled, 16 nop with a hazardous/absent predecessor, and 1,104 nop with a benign trivially-swappable predecessor — 881 of those a plain `addiu $sp,$sp,N` stack restore. No reorder-mode assembler leaves 1,104 stack restores sitting outside the slot; the binary was assembled without delay-slot filling, so maspsx's nop is FAITHFUL, not a gap. Consequence: the origin space for `ctc2 $t7,$7` in that slot is now exhaustively enumerated — cc1's reorg (provably halts at asm insns; 32/32 hand spellings and ~164k permuter iterations leave it nop), the assembler (measured never to fill), or a human. Only the third survives. H1's conclusion now rests on a complete enumeration rather than on an unexamined assumption.
- verdict: KILLED

## [s8] H21 — The cop2-tail population partitions by ORIGIN: functions whose delay slot our toolchain can reproduce versus those it cannot should separate by who wrote them.
- mechanism: If the filled-slot cases are hand-written asm and the nop cases are compiled/library code, the split should be visible as an address-space clustering and should be corroborated by the regfix rule population, independently of any argument about C spellings.
- probe: tmp/grind/func_80052B00/s8/cop2_partition.txt — every function whose last instruction before `jr $ra` is a cop2 op, cross-checked against inline_asm_canonical.txt, engine/queue.json and the six fill_delay rules in regfix.txt.
- result: CONFIRMED, and it is the first affirmative provenance evidence on this function (every prior session produced nulls). 17 leave the slot nop — 16 of them inside the single PsyQ libgte block 0x8007E1AC-0x8007F1A8, plus tslDmaDrawListDelAll (closed in pure C at src/display.c:2491); our toolchain reproduces that shape exactly. FIVE hold the cop2 op in the slot and all five occupy one contiguous run of BB2's own code, 0x80052A80-0x80052BDC: game_2d_CheckLifeGaugeNoDisp (jr @80052A80), func_80052A88 (@80052AF8), func_80052B00 (@80052B3C), func_80052B44 (@80052B74, already Judge-authorized canonical-asm), func_80052B7C (@80052BDC). The four unauthorized members each carry exactly ONE regfix rule and it is a fill_delay pulling the immediately preceding instruction — four of only six such rules tree-wide; the other two (func_8003B9D0 @49<-52, cpu_side_move_dir_3 @66<-68) pull from AFTER the jump, a jal-argument hoist and an unrelated phenomenon.
- verdict: CONFIRMED

## [s8] H22 — Moving the eight loads INSIDE the __asm__ block (the real PsyQ libgte macro shape) is a legal pure-C-plus-canonical-GTE form and a new register-allocation problem.
- mechanism: All 32 hand spellings and ~164k permuter iterations of sessions 1-7 kept the same division of labour — GCC emits the eight lw, the asm holds only ctc2. Nothing had ever moved an instruction across the asm boundary, and the library macros put their lw inside the asm with the matrix pointer as %0, making this the natural library-transplant answer for the rederive modality. It also turns the eight values into asm OUTPUTS (early-clobber) rather than inputs, a different reload problem.
- probe: Four forms measured with sandbox --disable all (tmp/grind/func_80052B00/s8/sweep8.py, engine JSON per form in engout_*.txt), all with %N placeholders and =&r/r constraints, no pins, no hardcoded GPRs, no fake clobbers; plus a direct call to engine.inlineasm.strip_cheat_asm_file() on the body and a standalone cc1 compile.
- result: KILLED twice over. asmloads8 / asmloads8-ret-s32 / asmloads8-mem-operand report score 16 with build_insns 2, and asmloads-split reports 16 with build_insns 10 — the 16s are the score of a DELETED body, not an improvement: strip_cheat_asm_file() returns stripped_count=1 and an empty function. Mechanism: classify_inline_asm puts `lw` in CHEAT_ASM_OPS and `ctc2` in CANONICAL_ASM_OPS, and _block_category keeps a block if ANY split template instruction is canonical — but split_template splits on the literal \n only, so in a "\n\t"-joined template every later instruction keeps a literal \t prefix, reads as first-token \tctc2, matches neither table and defaults to cheat; in practice a block is classified by its FIRST instruction, and here that is `lw`. The verdict is substantively correct regardless of the parsing quirk (an asm block doing its own general-purpose loads emits bytes from template text rather than from compilation — the inline-asm-injection family). It would not have matched anyway: standalone, cc1 accepts it and emits lw $9,0($4) / lw $8,4($4) / lw $7,8($4) / lw $6,12($4) / lw $5,16($4) / lw $10,20($4) / lw $3,24($4) / lw $2,28($4) — neither $t0..$t7 nor ascending, strictly further from the target than the s3 fused8 body.
- verdict: KILLED

## [s9] H23 � The target's register allocation ({$t0..$t7}) and/or its filled `jr $ra` delay slot are reachable under the ORIGINAL shipped PsyQ compiler (cc1psx.exe, GCC 2.7.2.SN.1) where our decompals fork fails; i.e. the whole 17-point residual is a compiler-divergence artifact of the reference toolchain rather than a property of the C source.
- mechanism: Every impossibility argument built in sessions 1-8 (H1 reorg.c:730-735 stop_search_p; H16 reload1.c:3606 order_regs_for_reload) was read out of, and measured against, `tools/gcc-2.7.2` � the decompals/mips-gcc-2.7.2 open-source port, which is a KMC-tailored fork, NOT the SN Systems cc1psx that actually built SLUS-00663. That gap is the single largest unexamined assumption underneath the whole disposition: if the shipped compiler's reload ordered registers differently (no `bad_spill_regs` route via `regs_explicitly_used`, or a `REG_ALLOC_ORDER` its port defines) or if its reorg admitted asm insns as delay-slot candidates, the target bytes would be ordinary compiler output from ordinary C and eight sessions of impossibility reasoning would be reasoning about the wrong compiler. Session 8 closed the assembler (H20) as a non-human origin but left the compiler-identity question untouched; no session 1-8 ever ran cc1psx on this function. The project's own standing rule ([[cc1psx-calibration-only]], user directive 2026-05-29) is that cc1psx is available precisely to settle "is it a compiler divergence?" empirically instead of asserting it.
- probe: tmp/grind/func_80052B00/s9/dualcc.sh compiles a standalone TU with BOTH compilers under the identical canonical flag set (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w`; the wrapper drops -mno-abicalls/-fno-builtin, which cc1psx does not accept, and passes the rest verbatim) and diffs the instruction streams. Two structurally distinct chassis were run � s9/fused8.c (the banked floor-17 form: eight C loads + ONE fused eight-input `__asm__`) and s9/sep8.c (the session-1/2 shape: eight C loads + EIGHT separate one-operand `__asm__` statements). Standalone TUs are faithful to in-tree codegen for this function per H9 (s4). Artifacts: s9/fused8.ours.s, s9/fused8.psx.s, s9/sep8.ours.s, s9/sep8.psx.s.
- result: KILLED, decisively and on both chassis. The two compilers emit INSTRUCTION-IDENTICAL bodies; the only diff is header/directive noise (`.file "T.C"` vs the real path, the `GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation` banner vs `GNU C 2.7.2 ... GNU MIPS/ELF`, cc1psx's extra `__gnu_compiled_c:` label and its absent `.version`/`.type`/`.size`/`.ident`). fused8 under BOTH: `lw $2,0($4) / lw $3,4($4) / lw $5,8($4) / lw $6,12($4) / lw $7,16($4) / lw $8,20($4) / lw $9,24($4) / lw $10,28($4)` then `ctc2 $2,$0 ... ctc2 $10,$7` then a bare `j $31` with no delay-slot instruction and no `.set noreorder` wrapper. That is the exact `{$2,$3,$5,$6,$7,$8,$9,$10}` set H16 predicts from `order_regs_for_reload`'s ascending zero-use scan with $4 excluded as an explicitly-used bad spill reg � reproduced by the compiler that actually shipped the game. sep8 under BOTH: the identical eight-load stream INCLUDING the idiosyncratic deferred self-clobbering `lw $4,0($4)` last and the resulting `ctc2 $4,$0` first � the two forks agree even on that quirk. So cc1psx is not a different allocator here, and the shipped compiler cannot produce the shipped function's registers from this C any more than our fork can.
- verdict: KILLED

## [s9] H24 � cc1psx's `reorg` differs from our fork's in whether an `__asm__` insn may be pulled into a `jr $ra` delay slot, so H1's impossibility proof (read from tools/gcc-2.7.2/reorg.c:730-735 `stop_search_p`) does not necessarily hold for the compiler that built the game.
- mechanism: H1 is the load-bearing half of the disposition and it is a SOURCE-READING result about our fork plus 32/32 empirical spellings compiled with our fork. `stop_search_p`'s unconditional `return 1` for ASM_INPUT / `asm_noperands >= 0` is one line of one file; SN Systems patched cc1 (the banner says 2.7.2.SN.1) and delay-slot policy is exactly the sort of PS1-targeted thing a vendor fork touches. If cc1psx's reorg were willing to schedule across asm, the target's delay-slot `ctc2 $t7,$7` would be ordinary compiler output and the disposition would collapse.
- probe: A differential, not just a re-run � the H23 dual-compile of s9/fused8.c (last insn needing a slot IS the asm) against control s9/ctlB.c, which is byte-for-byte the same body plus one trailing plain C store `matrix[0] = t7;` so the last insn needing a slot is NOT an asm. If both compilers fill in ctlB and neither fills in fused8, the asm insn is the only thing stopping the fill in both, and stop_search_p's behaviour is shared. Artifacts: s9/ctlB.c, s9/ctlB.ours.s, s9/ctlB.psx.s.
- result: KILLED. ctlB under BOTH compilers ends `.set noreorder / .set nomacro / j $31 / sw $2,0($4) / .set macro / .set reorder` � the store IS pulled into the delay slot, and both emit the identical `.set noreorder` wrapper to protect it. fused8 under BOTH ends with a bare `j $31`, no wrapper, no slot instruction. Same body, same flags, one non-asm trailing insn is the entire difference, and the two independent 2.7.2 forks behave identically on both sides of it. reorg in the SHIPPED compiler is equally willing and equally blocked: it fills this `jr` when it can and refuses when the only candidate is an asm insn. H1 is therefore a property of GCC 2.7.2 as a family, verified against the actual build compiler, not an artifact of the decompals port.
- verdict: KILLED

## [s9] The target's register allocation ({$t0..$t7}) and/or its filled `jr $ra` delay slot are reachable under the ORIGINAL shipped PsyQ compiler (cc1psx.exe, GCC 2.7.2.SN.1) where our decompals fork fails; i.e. the whole 17-point residual is a compiler-divergence artifact of the reference toolchain rather than a property of the C source.
- mechanism: Every impossibility argument built in sessions 1-8 (H1 from reorg.c:730-735 stop_search_p; H16 from reload1.c:3606 order_regs_for_reload) was read out of, and measured against, tools/gcc-2.7.2 — the decompals/mips-gcc-2.7.2 open-source port, a KMC-tailored fork, NOT the SN Systems cc1psx that actually built SLUS-00663. That is the single largest unexamined assumption underneath the disposition: if the shipped compiler's reload ordered hard registers differently (no bad_spill_regs route via regs_explicitly_used, or a REG_ALLOC_ORDER its port defines), or if its reorg admitted asm insns as delay-slot candidates, the target bytes would be ordinary compiler output from ordinary C and eight sessions would have been reasoning about the wrong compiler. Session 8 closed the ASSEMBLER as a non-human origin (H20) but left compiler identity untouched; no session 1-8 ever ran cc1psx on this function. The project's standing rule (cc1psx-calibration-only, user directive 2026-05-29) exists precisely to settle 'is it a compiler divergence?' by measurement rather than assertion.
- probe: tmp/grind/func_80052B00/s9/dualcc.sh compiles a standalone TU (faithful to in-tree codegen for this function per H9/s4) with BOTH compilers under the identical canonical flag set (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w; the wrapper drops -mno-abicalls/-fno-builtin, which cc1psx does not accept, and passes the rest verbatim) and diffs the instruction streams. Two structurally distinct chassis: s9/fused8.c (banked floor-17 form — eight C loads + ONE fused eight-input __asm__) and s9/sep8.c (session-1/2 form — eight C loads + EIGHT separate one-operand __asm__ statements).
- result: KILLED, decisively and on both chassis. The two compilers emit INSTRUCTION-IDENTICAL bodies; the only diffs are header/directive noise (.file "T.C" vs the real path; the 'GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation' banner vs 'GNU C 2.7.2 ... GNU MIPS/ELF'; cc1psx's extra __gnu_compiled_c: label; its absent .version/.type/.size/.ident). fused8 under BOTH: lw $2,0($4) / lw $3,4($4) / lw $5,8($4) / lw $6,12($4) / lw $7,16($4) / lw $8,20($4) / lw $9,24($4) / lw $10,28($4), then ctc2 $2,$0 ... ctc2 $10,$7, then a bare j $31 with no delay-slot instruction and no .set noreorder wrapper — the exact {$2,$3,$5,$6,$7,$8,$9,$10} set H16 predicts from order_regs_for_reload's ascending zero-use scan with $4 excluded as an explicitly-used bad spill reg, now reproduced by the compiler that shipped the game. sep8 under BOTH: the identical eight-load stream INCLUDING the idiosyncratic deferred self-clobbering lw $4,0($4) scheduled last and the resulting ctc2 $4,$0 first — the two forks agree even on that quirk.
- verdict: KILLED

## [s9] cc1psx's reorg differs from our fork's in whether an __asm__ insn may be pulled into a `jr $ra` delay slot, so H1's impossibility proof (read from tools/gcc-2.7.2/reorg.c:730-735 stop_search_p) does not necessarily hold for the compiler that built the game.
- mechanism: H1 is the load-bearing half of the disposition and it is a source-reading result about our fork plus 32/32 empirical spellings compiled with our fork. stop_search_p's unconditional return 1 for ASM_INPUT / asm_noperands >= 0 is one line of one file; SN Systems patched cc1 (the banner reads 2.7.2.SN.1) and delay-slot policy is exactly the sort of PS1-targeted behaviour a vendor fork touches. If cc1psx's reorg were willing to schedule across asm, the target's delay-slot ctc2 $t7,$7 would be ordinary compiler output and the disposition would collapse.
- probe: A differential rather than a re-run: the H23 dual-compile of s9/fused8.c (last insn needing a slot IS the asm) against control s9/ctlB.c, byte-for-byte the same body plus one trailing plain C store `matrix[0] = t7;` so the last insn needing a slot is NOT an asm. If both compilers fill in ctlB and neither fills in fused8, the asm insn is the sole blocker in both and stop_search_p's behaviour is shared.
- result: KILLED. ctlB under BOTH compilers ends `.set noreorder / .set nomacro / j $31 / sw $2,0($4) / .set macro / .set reorder` — the store IS pulled into the delay slot, and both emit the identical protective wrapper. fused8 under BOTH ends with a bare j $31, no wrapper, no slot instruction. Same body, same flags, one non-asm trailing insn is the entire difference, and the two independent 2.7.2 forks behave identically on both sides of it. reorg in the SHIPPED compiler is equally willing and equally blocked. H1 is therefore a property of GCC 2.7.2 as a family, verified against the actual build compiler.
- verdict: KILLED
