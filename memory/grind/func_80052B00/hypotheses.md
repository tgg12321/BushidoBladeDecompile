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
