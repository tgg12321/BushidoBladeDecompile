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
