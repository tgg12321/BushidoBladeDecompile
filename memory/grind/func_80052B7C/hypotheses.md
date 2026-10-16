# Hypothesis ledger — func_80052B7C

## s1 (recon)

### H1 — Four of five chunks land as direct PsyQ macro calls; chunk 2 is the open question

Statement: `func_80052B7C(MATRIX *rot, SVECTOR *bias, VECTOR *v0, VECTOR *out)` { `#include
<psxsdk/inline_o.h>` in this TU; `gte_SetRotMatrix(rot); <chunk-2 for bias>; gte_ldv0(v0);
gte_rt(); gte_stlvl(out); }` reproduces the 24-insn target, where chunk 2 is either (a) a
small written-out `__asm__` island (3x `lh` + 3x `ctc2` into CR5-7) citing the
`inline_asm_canonical.txt:340` grant directly (no macro exists to call), or (b) resolved by
an owner `ruling-request` on whether "no vendored macro exists for this op" is itself grounds
for the written-out island exception that item 3 of the 2026-10-15 GTE-macro-calls section
otherwise forbids.
Mechanism: not a codegen/GCC-internals question at all — this is a byte-for-byte,
instruction-for-instruction reproduction of an already-authorized whole-function asm leaf;
the only live question is which LANDING FORM (call vs. written island) the current policy
text permits for the one macro-less chunk.
Probe (for the next, construct-modality session): write the draft above into
src/main/3AB48.c (removing the `INCLUDE_ASM` line), add `#include <psxsdk/inline_o.h>`,
run `sandbox func_80052B7C --disable all` and `--diff`. If distance is not 0, the risk is
INSTRUCTION ORDER: GCC may not schedule the macro calls (each is its own set of `volatile`
asm statements) in the exact original sequence (loads-then-ctc2-batched vs. the macros'
interleaved load/store pattern) — read `.sched`/`.combine` dumps if so, per the PASS
ATTRIBUTION rule.
Result: UNMEASURED this session (recon only; no src edit made — construction is the next
session's job per the modality ladder).
Verdict: — (not yet probed)

### Frontier for the next (construct) session

1. hypothesis: chunks 1,3,4,5 compile byte-identically as `gte_SetRotMatrix(rot)`,
   `gte_ldv0(v0)`, `gte_rt()`, `gte_stlvl(out)` calls, in that order, immediately after a
   written chunk-2 island — mechanism: each vendored macro's own body is already the exact
   byte sequence `objdump` shows for that chunk (verified by hand in evidence.md s1); next
   probe: land the draft, `sandbox --disable all --diff`, check hunk class per-chunk.
2. hypothesis: if distance != 0 after (1), the failure is ORDERING between the 5 chunks’
   `volatile` asm blocks (GCC is free to interleave/reschedule across separate `__asm__`
   statements only in narrow cases, but cop2 latency `nop`s may land differently) — mechanism:
   `.sched`/`-da` dump diff at the macro-call boundaries; next probe: `dump.ps1
   func_80052B7C`, read `.sched`.
3. hypothesis: chunk 2 (the macro-less TR-from-halfwords load) needs a `ruling-request` on
   whether it may be a small written `__asm__` island under the existing
   `inline_asm_canonical.txt:340` grant (rather than a macro call, since none exists) — next
   probe: ask the precise question only if (1)/(2) leave chunk 2 as the sole blocker.

## s2 (structural)

### H1 result — CONFIRMED in part, KILLED in part

Built the s1 draft for real (applied to src/main/3AB48.c, measured, reverted per
asm-until-matched rule 1). `inline_o.h`: score 22 (build_insns 27). Switching to
`inline_c.h` (same macro names, pointer used directly as `%0`, no staging `move`):
score 16, build_insns == target_insns == 24. All 5 remaining hunks source-level per
`--diff`.

CONFIRMED: chunks 3 (`gte_ldv0`) and 4 (`gte_rt`) compile byte-identical to target when
called via `inline_c.h` (not `inline_o.h` — that header choice matters and was not
distinguished in s1's text-only read).

KILLED (instance): chunk 1 as a plain `gte_SetRotMatrix(rot)` call does NOT reproduce the
target's instruction order with EITHER vendored header — both interleave load/ctc2 where
the target batches all loads then all ctc2s, and no finer-grained vendored macro exists to
compose the batched order. kill_scope: instance (this chassis: inline_c.h macro call,
no FAKE construct present, measured via sandbox --diff this session on src/main/3AB48.c
with gte_SetRotMatrix(rot) as the sole chunk-1 statement). This was previously assumed
TRUE (an unmeasured claim) by s1's evidence.md; s2 measured it and found it false.

KILLED (instance): chunk 5 as a plain `gte_stlvl(out)` call does NOT reproduce the target's
delay-slot placement of the final `swc2` — measured via the same sandbox --diff run,
chassis: inline_c.h macro call, no FAKE construct present. Mechanism (reasoned, NOT
dump-confirmed — dump.ps1 produced no .rtl this session): gte_stlvl's 3-instruction
asm template is one opaque block to reorg.c's delay-slot filler.

### Frontier for the next session

1. hypothesis: a `ruling-request` covering BOTH chunk 1 and chunk 5 (not just chunk 2) is
   needed — the question is now "may a written-out single-instruction-per-statement form of
   an EXISTING vendored macro be used, under the inline_asm_canonical.txt:340 grant, when the
   macro's own bundled body cannot reproduce the target's measured instruction order or
   delay-slot placement" — mechanism: policy interpretation, not codegen; next probe: ask
   this precise question, citing this session's two measured kills as the evidence that a
   macro CALL is insufficient for chunks 1 and 5 specifically (chunks 3/4 are fine as calls).
2. hypothesis: before asking, confirm finding 2's mechanism with a real dump — `dump.ps1`
   found no `.rtl` this session (check whether it needs the sandboxed .o pre-built at a
   specific tmp path, or a different invocation form) — next probe: get `.sched` reading
   on the chunk-5 region to convert the reasoned inference into a confirmed one before it's
   cited in a ruling-request.
3. hypothesis: re-check whether `gtemac.h` (the composite macros, not yet read line-by-line
   for CR0-4-only or stlvl-only variants in s1/s2) has anything closer to the batched order
   for chunk 1 — next probe: grep `gtemac.h` for any macro touching only CR0-CR4 or only
   DR9-11, not yet done (s1 only enumerated CR5-7 sites for chunk 2).

## [s2] func_80052B7C(MATRIX *rot, SVECTOR *tr, VECTOR *v0, VECTOR *out) using gte_SetRotMatrix/gte_ldv0/gte_rt/gte_stlvl from psxsdk/inline_c.h plus a written __asm__ island for the macro-less TR-halfword chunk reaches sandbox distance 0.
- mechanism: byte-for-byte reproduction of an already owner-authorized whole-function asm leaf via vendored macro calls; inline_c.h's macros bind the pointer argument directly as the asm base register (no staging move), unlike inline_o.h.
- probe: Applied to src/main/3AB48.c, ran sandbox func_80052B7C --disable all --diff.
- result: Score dropped from floor 24 to 16; build_insns now equals target_insns (24). Confirmed for chunks 3 (gte_ldv0) and 4 (gte_rt), which match target byte-for-byte with inline_c.h. Did NOT reach distance 0: chunks 1 and 5 still diverge in instruction order / delay-slot placement. Reverted src to INCLUDE_ASM per asm-until-matched rule 1; full form banked in candidate.c.
- verdict: CONFIRMED

## [s2] gte_SetRotMatrix(rot), called as a vendored macro (either header), reproduces target chunk 1's exact instruction sequence.
- mechanism: s1 claimed (by manual text comparison only, never built) that this macro's body is byte-identical to the target's chunk 1.
- probe: Built the form with both inline_o.h and inline_c.h, read sandbox --diff hunk 1 against each.
- result: False with both headers: both macros interleave 2-lw+2-ctc2 then 3-lw+3-ctc2; target does all 5 lw first, then all 5 ctc2. No finer-grained vendored macro (checked inline_c.h for a standalone CR0-4 ctc2-only primitive) exists to compose the batched order. This corrects s1's evidence.md, which asserted an exact match that was never actually measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c, gte_SetRotMatrix(rot) called via psxsdk/inline_c.h, no FAKE construct present, sandbox func_80052B7C --disable all --diff this session

## [s2] gte_stlvl(out), called as a vendored macro, reproduces target chunk 5's delay-slot placement of the final swc2 in the jr $ra slot.
- mechanism: gte_stlvl's asm template bundles all 3 swc2 instructions into one volatile asm statement; GCC's delay-slot filler (reorg.c) fills a jr's delay slot from a single trailing ordinary insn, not by reaching inside a compound multi-instruction inline-asm block -- this mechanism is a reasoned inference from the measured instruction sequence, NOT yet confirmed by a .sched/.rtl dump (dump.ps1 produced no dump file this session).
- probe: Read sandbox --diff hunk 4/5 for the gte_stlvl(out) call with inline_c.h.
- result: False: our build emits swc2 $11,8(a3) before jr $ra with an explicit nop filling the delay slot, instead of target's swc2-in-delay-slot placement. No vendored macro stores only the final DR word alone to let it stand as a separate trailing statement.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c, gte_stlvl(out) called via psxsdk/inline_c.h, no FAKE construct present, sandbox func_80052B7C --disable all --diff this session

## s3 (structural)

### H1 — chassis discontinuity is spurious; floor 16 reproduces exactly

Statement: Re-applying s2's candidate.c body (gte_SetRotMatrix/gte_ldv0/gte_rt/gte_stlvl via
inline_c.h + written chunk-2 island) to src/main/3AB48.c this session reproduces sandbox score
16, build_insns 24 == target_insns 24, same 5 source-level hunks, byte-for-byte the same
measurement s2 banked.
Mechanism: not a codegen question -- the dispatch-time "measured 24" was simply the top-of-
session INCLUDE_ASM floor (asm-until-matched rule 1 means src/ is never left with draft C
between sessions), not evidence the s2 floor was wrong.
Probe: sandbox func_80052B7C --disable all --diff with the candidate body in place; output
saved to tmp/grind/func_80052B7C/s3/sandbox_recheck.txt.
Result: Reproduced exactly. Floor 16 stands, no re-derivation needed.
Verdict: CONFIRMED

### H2 — no finer-grained vendored GTE primitive exists in gtemac.h or inline_c.h for chunk 1 or chunk 5

Statement: Neither `include/psxsdk/gtemac.h` (composite macros) nor `include/psxsdk/inline_c.h`
(leaf macros) defines anything touching only CR0-CR4 (chunk 1's batched-order target) or only
DR9-11's last word alone (chunk 5's single-store target) as a strict subset of an existing
macro's body.
Mechanism: not a codegen question -- a full `grep -n "define gte_"` enumeration of both headers
(48 gtemac.h composites, ~140 inline_c.h leaves) shows every CR0-4-touching macro is
`gte_SetRotMatrix`/`gte_SetRotMatrixClearTrans`-shaped (same batching as the target's load-side,
but still interleaved on the ctc2 side per s2's finding) and every DR9-11-touching store macro
is `gte_stlvl`/`gte_stlvnl`-shaped (always 2-3 stores bundled in one asm_operands, never a lone
final word).
Probe: grep both headers for every `define gte_` site this session (s1 only enumerated the
CR5-7-touching subset for chunk 2; this session covers the full file for both chunks 1 and 5).
Result: No finer primitive found in either header.
Verdict: KILLED
Kill_scope: instance (this chassis: inline_c.h + gtemac.h as vendored at this commit, no FAKE
construct present, measured by direct grep enumeration this session)
Measured_on: include/psxsdk/gtemac.h and include/psxsdk/inline_c.h, full `define gte_`
enumeration, no src/ edit involved in this specific check.

### H3 — chunk 5's delay-slot gap is reorg.c/dbr treating the bundled asm_operands as opaque (dump-confirmed)

Statement: `tmp/grind/func_80052B7C/dumps/main/3AB48.dbr` shows insn 23 (`gte_stlvl`'s full
3-swc2 body) as a single RTL `asm_operands` node, followed (after only `NOTE_INSN_DELETED`
notes) by `jump_insn 32 (return)`; no ordinary single-instruction RTL node sits between them for
reorg.c's `fill_simple_delay_slots` to steal into the `jr $ra` delay slot.
Mechanism: GCC's delay-slot filler cannot select or split a multi-instruction `asm_operands`
RTL insn for delay-slot placement -- it is one opaque node regardless of how many MIPS
instructions its text expands to; confirmed by reading the actual `.dbr` dump rather than
inferred from instruction-count reasoning (s2's version of this kill).
Probe: tools/grinder/dump.ps1 func_80052B7C (fixed this session -- see artifacts), read
3AB48.dbr lines 112-148 for the func_80052B7C region.
Result: Confirmed via direct RTL read. Supersedes s2's "reasoned, not dump-confirmed" caveat on
the same kill; the kill itself (gte_stlvl as a plain macro call does not reach delay-slot
placement) is unchanged, now dump-backed.
Verdict: KILLED
Kill_scope: instance
Measured_on: src/main/3AB48.c, gte_stlvl(out) called via psxsdk/inline_c.h, no FAKE construct
present, .dbr RTL dump this session (tmp/grind/func_80052B7C/dumps/main/3AB48.dbr)

### Frontier for the next session

1. hypothesis: a `ruling-request` is still needed (unchanged from s2) -- chunks 1 and 5 each
   have a vendored macro whose bundled body structurally cannot reach the target's measured
   instruction order (chunk 1) or delay-slot placement (chunk 5), and this session's gtemac.h
   enumeration (H2) closes off "a finer macro exists" as an escape; the written-single-
   instruction-island question is now the ONLY live lever for these two chunks and is a policy
   question (inline-asm-policy.md item 3's "Calls, not copies"), not a codegen one -- mechanism:
   policy interpretation; next probe: file the ruling-request citing s2's two kills + this
   session's H2/H3 confirmations as the complete evidence set (no codegen work remains to do
   before asking).
2. hypothesis: (RETIRED by this session's H2) gtemac.h composites might offer a finer primitive
   -- checked exhaustively this session, none found; do not re-check without a header update.

## s3 continued (structural) -- ruling landed, frontier closed

### H4 -- the written-island ruling-request (s2/s3 frontier item 1) resolved FAIL; written islands
for chunks 1, 2 and 5 are a CLASS kill, not an open policy question

Statement: docs/grind/decisions.md 2026-10-15 20:38 (func_80052B7C ruling) answers s2/s3's
exact frontier question and answers it NO: "a written-out single-instruction-per-statement form
of an existing vendored macro" is NOT admissible under the inline_asm_canonical.txt:340 grant,
even when the macro's own bundled body cannot reach the target bytes ("'The macro can't reach
the bytes' is not an exception anywhere in the rule"). The ruling also confirms chunk 2's
pre-existing no-macro-exists question resolves the same way (no carve-out for "no macro exists
either").
Mechanism: inline-asm-policy.md:55-57 ("Calls, not copies... No `__asm__` spelling of a header
macro (written-out bodies, joined statements, edited clobbers, hand command words)") plus
inline-asm-policy.md:70-72 ("The written-out island routes... admit nothing new... until
`completion.REFUSE_WRITTEN_OUT_UNITS` is turned on"). This is a rule-text class predicate, not
chassis-relative: it bans the CONSTRUCT (a written __asm__ reproducing/splitting a macro body)
regardless of which chunk or which macro, for every function under this grant.
Probe: read the ruling text in full (docs/grind/decisions.md:~1364) and cross-checked against
the current inline-asm-policy.md text (quoted above, lines 55-57 and 70-72) -- the ruling's
citations match the live rule, so it is not stale.
Result: CONFIRMED (the ruling stands and matches current rule text). The s2 candidate.c body
(gte_SetRotMatrix/gte_ldv0/gte_rt/gte_stlvl via inline_c.h + written chunk-2 lh/ctc2 island) is
INVALID as a landing candidate; moved to rejected/.
Verdict: KILLED
Kill_scope: class
Measured_on: src/main/3AB48.c (not re-applied this session -- the ruling itself is the
measurement; no FAKE construct would be present in the rejected form either way, it is simply
a forbidden construct, not an unlabelled FAKE)
Predicate_cite: .claude/rules/inline-asm-policy.md:55 (also :70-72)

### H5 -- no gtemac.h composite macro offers an alternate expansion for chunks 1 or 5 (full visual
confirmation beyond s3's grep)

Statement: Read every `#define gte_` composite in gtemac.h (48 entries, lines 7-360). None
names or expands to `gte_SetRotMatrix`/`gte_SetRotMatrixClearTrans` (chunk 1's macro) or
`gte_stlvl` (chunk 5's macro) in a batched/partial form; the closest shape, `gte_RotTrans`
(line 23: `gte_ldv0(r1); gte_rt(); gte_stlvnl(r2); gte_stflg(r3);`), is built from the SAME leaf
macros already measured in s2/s3 (and uses `gte_stlvnl`, a different store leaf, not `gte_stlvl`)
-- composites are pure textual sugar over leaves already tried, so they cannot change either
chunk's internal instruction order.
Mechanism: gtemac.h composites expand to sequences of the SAME inline_c.h/inline_o.h leaf
macros (each already volatile asm, so GCC preserves each leaf's internal order regardless of
which composite macro calls it) -- no composite can alter a leaf's own bundled instruction
order, so this was never a live avenue once H2 (s3) confirmed no finer leaf exists.
Probe: Read include/psxsdk/gtemac.h in full (all 48 `#define gte_` sites and their expansions).
Result: Confirms s3's H2 with a full visual read (not just grep for name substrings): no
composite macro reaches chunk 1's batched load-then-ctc2 order or chunk 5's lone trailing
store. This closes the "composite might help" sub-question definitively; retired from frontier.
Verdict: KILLED
Kill_scope: instance (this chassis: gtemac.h as vendored at this commit; would become class if
gtemac.h is ever revendored/updated, which is out of grind-session scope)
Measured_on: include/psxsdk/gtemac.h, full-file read this session, no src/ edit involved.

### Frontier for the next session

With H4 and H5, every structural/codegen lever for func_80052B7C is now measured dead:
- Chunks 3, 4 land as plain macro calls (CONFIRMED, s2).
- Chunks 1, 2, 5 have no legal pure-C landing form: the macros that exist don't reach the
  target bytes (chunks 1, 5 -- confirmed s2/H3), no macro exists at all (chunk 2 -- s1), no
  finer leaf or composite exists for any of the three (H2, H5), and written islands closing the
  gap are now a ruled CLASS refusal (H4).
- The only remaining levers are NOT C-source levers and are out of grind-session scope:
  (a) an `engine:`-commit addition to the vendored macro set / a new DMPSX word covering
  chunk 2's TR-from-halfwords load, needing its own owner ruling (inline-asm-policy.md item 1);
  (b) raising func_80052B7C's `scan_hand_coded` tier from LOW 0/12 to STRONG so the existing
  whole-function `inline_asm_canonical.txt:340` grant holds as-is (the func_8004A76C 2026-10-15
  entry in docs/grind/decisions.md is the precedent for this path, but getting there needs an
  engine/tools change to the scanner's signal set, not a C edit).
1. hypothesis: (a) above -- a new vendored macro/DMPSX word for chunk 2 would let all 5 chunks
   land as calls (chunks 1/5 still fail on order/delay-slot though -- H4 means even THIS doesn't
   fully close it without also revisiting (b)). mechanism: policy/engine, not codegen. next
   probe: this needs an owner ruling + engine commit; not actionable in a grind session.
2. hypothesis: (b) above -- scan_hand_coded needs a new STRONG-tier signal this function
   actually exhibits (re-run `scan_hand_coded.py --single func_80052B7C` fresh to confirm it is
   still LOW 0/12, not stale) before concluding the grant-path is blocked on tooling alone.
   mechanism: engine/scan_hand_coded.py signal coverage. next probe: re-run the scanner this
   session's successor to get a current tier reading, then decide whether a tooling change (not
   a grind action) is the only gap.

## [s3] A written __asm__ island reproducing or splitting an existing (or nonexistent) vendored GTE macro's body is admissible under the inline_asm_canonical.txt:340 grant when the macro itself cannot reach the target bytes (chunks 1 and 5) or no macro exists at all (chunk 2).
- mechanism: policy interpretation of inline-asm-policy.md's 'Calls, not copies' (PsyQ GTE macro calls, item 3) -- not a codegen/GCC-internals question.
- probe: Filed as the s2/s3 ruling-request; answered by docs/grind/decisions.md 2026-10-15 20:38 func_80052B7C ruling, cross-checked this session against the live inline-asm-policy.md text (lines 55-57, 70-72).
- result: FAIL: 'the macro can't reach the bytes' is not an exception anywhere in the rule; the written chunk-2 island is itself a new written island and not precedent; the grant is not a licence for islands inside a C body.
- verdict: KILLED
- kill_scope: class
- measured_on: src/main/3AB48.c (the ruling itself is the measurement; no src/ re-application needed -- the construct is a forbidden spelling, not an unlabelled FAKE, so chassis state does not bear on it)
- predicate_cite: .claude/rules/inline-asm-policy.md:55

## [s3] A gtemac.h composite macro (not yet read in full, only grepped by name in s1/s3) offers a batched-order expansion for chunk 1 (gte_SetRotMatrix) or a lone-trailing-store expansion for chunk 5 (gte_stlvl).
- mechanism: gtemac.h composites are textual sugar over the same inline_c.h/inline_o.h leaf macros already measured; each leaf is its own volatile asm block whose internal order GCC preserves regardless of which composite invokes it.
- probe: Read include/psxsdk/gtemac.h in full (48 #define gte_ sites, lines 7-360) rather than grepping for substring matches.
- result: No composite names or expands to gte_SetRotMatrix/gte_SetRotMatrixClearTrans or gte_stlvl; the closest shape (gte_RotTrans, line 23) uses gte_ldv0/gte_rt/gte_stlvnl/gte_stflg -- the same already-tried leaves, and a different store leaf (stlvnl, not stlvl) besides.
- verdict: KILLED
- kill_scope: instance
- measured_on: include/psxsdk/gtemac.h as vendored at this commit, full-file read this session, no src/ edit involved

## s4 (permuter)

### H6 -- decomp-permuter's standard import pipeline cannot build a target.o for this function at all

Statement: `tools/decomp-permuter/import.py --keep src/main/3AB48.c asm/funcs/func_80052B7C.s`,
run against a temporary (never committed) chassis `#include <psxsdk/inline_c.h>; void
func_80052B7C(MATRIX *rot, SVECTOR *tr, VECTOR *v0, VECTOR *out) { gte_SetRotMatrix(rot);
gte_ldv0(v0); gte_rt(); gte_stlvl(out); }`, exits 1 and produces no target.o. import.py's
default `Assembler:` is plain `mips-linux-gnu-as -march=vr4300 -mabi=32` (not this project's
aspsx-2.34-via-maspsx pipeline); an earlier identical attempt (same chassis, not saved to file)
showed the concrete cause: `target.s:105: Error: unrecognized opcode 'mvmva 1,0,0,0,0'` -- GNU
binutils as has no cop2 GTE opcodes, and `asm/funcs/func_80052B7C.s:22` is exactly that `mvmva`
instruction. No `base.c`/`compile.sh` built this session routes around it: the permuter needs a
byte-correct `target.o` to score against, and the only pipeline that assembles PsyQ GTE pseudo-
ops is the project's own `maspsx`/`aspsx` chain (as used by `sandbox`/the real build), which
import.py does not invoke.
Mechanism: not a codegen question -- plain tooling incompatibility between decomp-permuter's
generic MIPS assembler and this function's cop2 instruction content.
Probe: `tmp/grind/func_80052B7C/s4/run_import.sh` (committed to artifacts), output in
`tmp/grind/func_80052B7C/s4/permuter_import_attempt.txt`. Chassis applied to src/main/3AB48.c
for the run only, reverted immediately after (asm-until-matched rule 1); no src/ change banked.
Result: `IMPORT_EXIT=1`, `nonmatchings/func_80052B7C/` holds only `base.c`/`compile.sh`/
`settings.toml`/`target.s` -- no `target.o`. No permuter campaign could be launched (no scorable
workspace exists to launch one against).
Verdict: KILLED
Kill_scope: instance (this chassis + this import invocation; a hand-built workspace routing
target.s through the project's own maspsx/aspsx pipeline, like `tools/mar_perm_workspace.sh`
does for `marionation_Exec`, is UNTESTED and would be a different probe, not covered by this
kill)
Measured_on: src/main/3AB48.c, temporary macro-call chassis (gte_SetRotMatrix/gte_ldv0/gte_rt/
gte_stlvl via inline_c.h, chunk-2 omitted), no FAKE construct present, reverted after measurement;
tmp/grind/func_80052B7C/s4/permuter_import_attempt.txt

### Why even a working permuter workspace would not help here (reasoned, ties to s2/s3's dump-confirmed kills)

This chassis has zero free C-level variables for the permuter to mutate: four pointer params
used exactly once each, four sequential statement-level macro calls, no loops, no scalars, no
branches. The three unresolved chunks' divergence (chunk 1's load/store interleave order, chunk
5's delay-slot placement, confirmed via `.dbr` RTL read in s3/H3) sits INSIDE each macro's own
single `asm_operands` RTL node -- a C-level statement permutation around the call sites cannot
reach into it, and splitting the macro's internal asm would itself be the already-class-killed
written-island construct (s3/H4). PERM_LINESWAP/PERM_GENERAL reorder STATEMENTS; there is no
statement-level reordering of `gte_SetRotMatrix(rot); gte_ldv0(v0); gte_rt(); gte_stlvl(out);`
that is both semantically valid (gte_rt needs both loads to have happened) and capable of
changing either macro's own internal instruction emission. This is not a new measurement (no
tool run backs it beyond the dump already cited in s3) -- recorded as reasoning context for why
H6's infrastructure failure does not leave useful permuter work undone.

## [s4] decomp-permuter's standard import pipeline (tools/decomp-permuter/import.py) can build a scorable target.o for func_80052B7C from asm/funcs/func_80052B7C.s against a temporary macro-call chassis (gte_SetRotMatrix/gte_ldv0/gte_rt/gte_stlvl via inline_c.h, chunk 2 omitted).
- mechanism: Plain tooling compatibility: import.py's default Assembler is mips-linux-gnu-as -march=vr4300 -mabi=32, a generic GNU MIPS assembler with no PsyQ/cop2 GTE opcode support, unlike this project's own aspsx-2.34-via-maspsx pipeline used by sandbox/build.
- probe: Applied the temporary chassis to src/main/3AB48.c, ran tools/decomp-permuter/import.py --keep src/main/3AB48.c asm/funcs/func_80052B7C.s (tmp/grind/func_80052B7C/s4/run_import.sh), reverted src/main/3AB48.c immediately after.
- result: FAIL: exit 1, no target.o produced. An earlier identical attempt (same chassis, output not saved) showed the proximate cause: target.s:105: Error: unrecognized opcode 'mvmva 1,0,0,0,0' -- asm/funcs/func_80052B7C.s:22 is exactly that mvmva cop2 instruction, which GNU binutils as cannot assemble. No campaign could be launched this session: there is no scorable workspace to launch one against.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c, temporary macro-call chassis (gte_SetRotMatrix/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h, chunk-2 omitted), no FAKE construct present, reverted after measurement; tmp/grind/func_80052B7C/s4/permuter_import_attempt.txt

## s5 (enumerate)

### H7 -- the enumerate modality's precondition (a region of ordinary C with named locals,
assignments, or commutative sub-expressions) does not exist anywhere in this function's legal
chassis

Statement: `tools/spelling_enum.py`'s documented search space is: (axis 1) inline-vs-keep a
`TYPE name = expr;` local, (axis 2) declaration-order permutation among independent locals/
assignments, (axis 3, `--swaps`) commutative-operand swap on `A*B`/`A+B` sub-expressions. The
only legal C chassis for func_80052B7C (chunks 3/4 as `gte_ldv0(v0); gte_rt();` via
`inline_c.h`, already byte-identical per s2/CONFIRMED) plus chunks 1/2/5 (no legal landing
form at all, per s3/H4 class kill on written islands) contains, in total: zero scalar locals,
zero assignments to outer variables, zero commutative arithmetic operators. The entire body is
four sequential bare `volatile asm` macro-call statements with no sub-expression of any kind;
`memory/grind/func_80052B7C/candidate.c` itself is pure comment (no function body at all) for
exactly this reason -- there is nothing syntactically present to feed the enumerator's three
axes.
Mechanism: not a codegen question -- a tooling/object-model mismatch between what
`spelling_enum.py` operates on (statement-level C: locals, assignments, operators) and what
this function's residual actually is: either (a) bytes fixed inside a vendored macro's own
`asm_operands` RTL node (chunks 1, 5 -- confirmed opaque to GCC's own passes per s3/H3's `.dbr`
dump, let alone to a C-level statement permuter) or (b) a chunk with no C statement of any kind
admissible under the current ruling (chunk 2, s3/H4 class kill, predicate_cite
`.claude/rules/inline-asm-policy.md:55`). Even a hypothetical winning spelling from the
enumerator for the (banned) written-island form would still BE the written-island construct
H4 already class-killed -- enumerating its spellings cannot change which construct it is.
Probe: Read `tools/spelling_enum.py`'s docstring (full axis definitions, lines 1-40) against
this session's re-measurement of `memory/grind/func_80052B7C/candidate.c` (confirmed via
`sandbox func_80052B7C --disable all --diff --candidate memory/grind/func_80052B7C/candidate.c`
that the file is pure comment with no `func_80052B7C` body to measure -- "not found in
...3AB48.o" -- consistent with s3's documented conclusion that no legal C body exists for this
function at all) and against the live chassis (`sandbox func_80052B7C --disable all` on
unmodified src/, confirming floor is still the INCLUDE_ASM-stub 24, no C ever landed).
Result: Confirmed. There is no block of ordinary C in this function's legal chassis for
`spelling_enum.py`/`sweep_variants.py` to sweep -- the entire residual sits either inside an
opaque vendored-macro RTL node (unreachable by any C-level statement rewrite, s3/H3) or in a
chunk whose only candidate C spelling (a written `__asm__` island) is itself the already
class-killed construct (s3/H4), not an open spelling question about a legal construct. This
extends s4's finding (no scorable permuter target) to the enumerate modality: like the
permuter, the enumerator needs a mutable C surface, and this chassis has none, for the same
underlying reason (s3/H3's dump-confirmed opaque asm_operands nodes + s3/H4's class ban).
Verdict: KILLED
Kill_scope: class
Measured_on: memory/grind/func_80052B7C/candidate.c (pure-comment, no body) re-measured via
sandbox func_80052B7C --disable all --diff --candidate this session; src/main/3AB48.c
unmodified (still INCLUDE_ASM); no FAKE construct present in either file.
Predicate_cite: .claude/rules/inline-asm-policy.md:55 (chunk 2 / written-island ban, unchanged
from s3/H4) and tools/spelling_enum.py:12-39 (the tool's own documented input precondition --
named locals / assignments / commutative operators -- which this chassis categorically lacks)

### Frontier for the next session

Unchanged from s3/s4 -- confirmed again this session from a fourth angle (enumerate, after
structural/permuter): every modality that operates on C-level statement content finds zero
surface, because the residual is not a C-spelling question for this function. The only two
live, non-C-source levers remain:
1. a new vendored macro / DMPSX word for chunk 2 (inline-asm-policy.md item 1, needs owner
   ruling + engine commit) -- would still leave chunks 1/5 unreached.
2. raising scan_hand_coded from LOW 0/12 to STRONG (engine/tools signal-set change, not a C
   edit) to let the existing whole-function inline_asm_canonical.txt:340 grant hold as-is.
Neither is actionable from a grind session restricted to src/main/3AB48.c.

## [s5] func_80052B7C's legal C chassis contains a region of ordinary C (named locals, assignments, or commutative sub-expressions) that tools/spelling_enum.py/sweep_variants.py can exhaustively sweep to close the residual.
- mechanism: spelling_enum.py's documented search space (axis 1: inline-vs-keep a TYPE name = expr local; axis 2: declaration-order permutation; axis 3: commutative-operand swap on A*B/A+B) requires a complete function body containing at least one such construct.
- probe: Read tools/spelling_enum.py lines 1-40 (full docstring) against memory/grind/func_80052B7C/candidate.c (read in full) and a fresh sandbox func_80052B7C --disable all --diff --candidate memory/grind/func_80052B7C/candidate.c run; also re-ran sandbox func_80052B7C --disable all --diff on unmodified src/ to confirm current chassis state.
- result: False. candidate.c is a pure 72-line comment block with no function definition (sandbox --candidate errors 'func_80052B7C not found in ...3AB48.o'), by design -- its own header states no src/ edit is banked because chunks 1, 2 and 5 have zero legal C landing form under the current ruling (s3/H4 class kill on written islands) and chunks 3/4's legal form (gte_ldv0(v0); gte_rt(); via inline_c.h) is two bare volatile-asm macro-call statements with no locals, assignments, or operators. There is no statement content anywhere in this chassis for the enumerator's three axes to act on. The residual itself lives either inside a vendored macro's own opaque asm_operands RTL node (chunks 1, 5 -- dump-confirmed opaque to GCC's own reorg.c in s3/H3) or in a chunk with no legal C spelling at all (chunk 2, s3/H4) -- in neither case is it a question of WHICH SPELLING of a legal construct is correct, which is the only kind of question this modality's tooling answers.
- verdict: KILLED
- kill_scope: class
- measured_on: memory/grind/func_80052B7C/candidate.c (pure-comment, no body) re-measured via sandbox --disable all --diff --candidate this session; src/main/3AB48.c unmodified (still INCLUDE_ASM); no FAKE construct present in either file
- predicate_cite: .claude/rules/inline-asm-policy.md:55

## s6 (synthesis)

- H-s6-1 CONFIRMED: gte_ldtr((s32)tr->vx,(s32)tr->vy,(s32)tr->vz) lands chunk 2 as a vendored-macro call with
  ordinary-C halfword loads -- s1's "no macro for chunk 2" was a misread (operand-taking macro). Legal
  pure-macro chassis measures 19 (s6 artifacts). Frontier item "new DMPSX word for chunk 2" is VOID.
- H-s6-2 CONFIRMED (re-audit): s2 chunk-1/chunk-5 instance kills still hold on the s6 chassis, no FAKE present.
- Frontier reset (strongest 3):
  1. Chunk 1 batched order: the target's shape (all 8 loads, then 8 ctc2) is what a single asm statement
     taking 8 register operands would produce. No vendored macro has register operands into CR0-CR4
     (grep inline_c.h for `ctc2 %`: only $5-7,$15,$21-23,$26). Next: census libgte.h/gtemac.h/SOTN psxsdk
     headers for any CR0-4 register-operand macro (e.g. a gte_ldR11R12-style word); if none, chunk 1 is
     macro-unreachable and the residual is canonical-asm-grant territory.
  2. Chunk 5 delay slot: next probe is any vendored store macro that emits the $11 store as a separate
     asm statement (e.g. a stlvl split into gte_stlvnl-like pieces) -- s3 full gtemac.h read found none;
     inline_c.h single-register store words (gte_stsz-like for $11?) not yet censused.
  3. scan_hand_coded tier (LOW 0/12) remains the whole-body grant gate; unchanged, engine-side.
- H-s6-3 KILLED (instance, s6 census tmp/grind/func_80052B7C/s6/census.txt): inline_c.h/inline_o.h/gtemac.h/libgte.h
  as vendored contain no `ctc2 %N, $0..$4` register-operand macro (chunk 1) and no swc2 $9/$10/$11 store other
  than gte_stlvl's 3-store body (chunk 5). Frontier items 1-2 above are therefore answered for the
  vendored headers; what remains is a NEW vendored macro word (owner + engine: commit) or the
  whole-body grant tier.

## [s6] gte_ldtr (inline_c.h:371), a register-operand macro, lands chunk 2 (lh x3 + ctc2 $5-$7) with the halfword loads as ordinary C, giving a pure vendored-macro body with no __asm__.
- mechanism: gte_ldtr's operands are "r" inputs, so GCC emits the tr->vx/vy/vz loads itself; (s32) casts make them sign-extending lh (TRX-Z are signed 32-bit).
- probe: src/main/3AB48.c body gte_SetRotMatrix; gte_ldtr((s32)tr->vx,(s32)tr->vy,(s32)tr->vz); gte_ldv0; gte_rt; gte_stlvl; sandbox --disable all --diff
- result: score 19 (24/24 insns); chunk 2 emits lh+ctc2 $5-7 (regs a0/v1/v0 inside hunk 1); without casts lhu, also 19. Legal floor 19 vs stub 24.
- verdict: CONFIRMED

## [s6] Kill re-audit: the s2 instance kills (gte_SetRotMatrix interleaved order; gte_stlvl final swc2 not in jr delay slot) were measured with no FAKE carrier and still hold on the s6 chassis.
- mechanism: fake_ablate finds no FAKE construct; --diff hunks 1 and 4 unchanged.
- probe: tools/fake_ablate.py on candidate.c; sandbox --diff on s6 chassis
- result: No FAKE constructs to ablate; hunk 1 (lw/ctc2 interleave) and hunk 4 (nop in jr slot) persist.
- verdict: CONFIRMED

## [s6] A vendored header macro in include/psxsdk (inline_c.h, inline_o.h, gtemac.h, libgte.h) takes register operands into CR0-CR4 (would let chunk 1 batch its loads) or stores $9/$10/$11 individually (would free the last swc2 for the delay slot).
- mechanism: grep census for ctc2 %N,$0-$4 and swc2 $9/$10/$11
- probe: tmp/grind/func_80052B7C/s6/census.txt
- result: None found: only gte_stlvl's 3-store body (inline_c.h:1086-1089); no register-operand ctc2 into CR0-4.
- verdict: KILLED
- kill_scope: instance
- measured_on: include/psxsdk/*.h as vendored at this commit, grep census, no src/ edit, no FAKE construct

## s7 (synthesis)

## [s7] The 19-point residual of the legal chassis is a decompals-cc1 vs SN-cc1psx divergence (the original compiler would emit the target's order/delay slot from the same C).
- mechanism: cc1psx calibration (tools/cc1psx.exe, calibration-only)
- probe: compile tmp/grind/func_80052B7C/s7/t.c with both compilers, diff function bodies
- result: byte-identical function bodies (cc1_vs_cc1psx.txt). Not a toolchain divergence.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 legal chassis (gte_SetRotMatrix/gte_ldtr((s32)..)/gte_ldv0/gte_rt/gte_stlvl via inline_c.h), no FAKE construct

## [s7] Some C body compiled by GCC 2.7.2 places the final swc2 $11 (GTE store) in the jr $ra delay slot.
- mechanism: reorg.c:740-744 stop_search_p rejects every asm insn as a delay-slot candidate; mips.md has no cop2 store pattern, so swc2 $11 exists only as an asm insn.
- probe: read reorg.c:736-746; grep -c cop2/swc2 config/mips/mips.md (=0); confirmed both compilers emit bare `j $31`.
- result: no C body can fill the slot with the GTE store; chunk 5's 2 hunks are closed for any C body. Only the hand-written (whole-body asm grant) branch reaches it.
- verdict: KILLED
- kill_scope: class
- measured_on: tools/gcc-2.7.2 reorg.c + config/mips/mips.md source; s6 legal chassis output from cc1 and cc1psx, no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reorg.c:740

## [s7] A sibling in the 3AB48 GTE cluster carries a STRONG scan_hand_coded tier that could anchor a cluster grant.
- probe: scan_hand_coded.py --single on all five cluster functions
- result: all LOW 0/12 (cluster_scan.txt); the scanner has no cop2-delay-slot signal.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/scan_hand_coded.py signal set at this commit, asm/funcs/*.s for the five functions

### Frontier after s7
1. Chunk 5 is now class-closed for C (reorg.c:740). Since a pure-macro C body can never reach 0, the only
   landing is the whole-body hand-written branch of the Judge constraint: a gate-class scanner signal for
   "cop2/GTE insn in a jr delay slot" (reorg.c:740 makes it a compiler-impossibility signal, same logic as
   other S-class signals) would lift all five 3AB48 GTE leaves to STRONG. Engine-side; cite this predicate.
2. Chunk 1 (batched 5 lw then 5 ctc2, t0..t7 sequential): no vendored register-operand CR0-4 macro (s6 census).
   Moot for landing given item 1, but still open as a vendored-header question.
3. Escalation modality should file the CANONICAL-ASM GRANT PATH record citing reorg.c:740 + cc1psx identity.

## [s7] The 19-point residual of the s6 legal chassis is a decompals-cc1 vs SN cc1psx divergence: the original compiler would emit the target order and delay slot from the same C.
- mechanism: cc1psx calibration (tools/cc1psx.exe via cc1psx_wrapper.sh, calibration only)
- probe: compile tmp/grind/func_80052B7C/s7/t.c with tools/gcc-2.7.2/build/cc1 and cc1psx, diff function bodies
- result: Function bodies byte-identical (interleaved SetRotMatrix lw/ctc2, lh x3, swc2 x3 then bare j $31). Residual is not a compiler-port divergence.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 legal chassis gte_SetRotMatrix/gte_ldtr((s32)..)/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h, no FAKE construct

## [s7] A C body compiled by GCC 2.7.2 can place the final GTE store swc2 $11 in the jr $ra delay slot.
- mechanism: reorg.c stop_search_p (case INSN) returns 1 for every ASM_INPUT / asm_operands insn; mips.md has zero cop2/swc2 patterns so swc2 $11 only exists as an asm insn
- probe: read tools/gcc-2.7.2/reorg.c:736-746, grep -c cop2|swc2 config/mips/mips.md (0), both compilers' output ends in bare j $31
- result: The delay-slot GTE store is never produced by GCC 2.7.2 from C; chunk 5 (diff hunks 3-4) is only reachable by the whole-body hand-written branch.
- verdict: KILLED
- kill_scope: class
- measured_on: tools/gcc-2.7.2 reorg.c and config/mips/mips.md source; s6 legal chassis output from cc1 and cc1psx; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reorg.c:740

## [s7] A sibling in the 3AB48 GTE cluster carries a STRONG scan_hand_coded tier that could anchor a cluster grant for func_80052B7C.
- mechanism: scan_hand_coded.py gate-class signals
- probe: scan_hand_coded.py --single on func_80052B7C, gte_SetMatrixRotTransIR, gte_SetMatrixRotTransIRVec, gte_SetRotTransMatrix, gte_SetRotMatrixClearTrans
- result: All five LOW 0/12, no gate-class signal; the signal set has no cop2-in-delay-slot signal.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/scan_hand_coded.py signal set at this commit on asm/funcs/*.s for the five functions

## s8 (structural)

### H7 — No block-local var split / declaration-order / type-narrowing / statement-reassociation
lever on the legal macro-call chassis can touch chunk 1 or chunk 5's residual, because both
chunks' divergent bytes are emitted entirely from literal, hardcoded `__asm__ volatile` template
strings baked into the vendored macro definitions, not computed from any C-level expression.

Statement: Re-applied s6/s7's legal chassis (`gte_SetRotMatrix(rot); gte_ldtr((s32)tr->vx,
(s32)tr->vy, (s32)tr->vz); gte_ldv0(v0); gte_rt(); gte_stlvl(out);`, `#include
<psxsdk/inline_c.h>`) to src/main/3AB48.c this session and re-measured fresh (not just trusting
the chassis-check number).

Mechanism: Read `gte_SetRotMatrix` (include/psxsdk/inline_c.h:297-309) and `gte_stlvl`
(include/psxsdk/inline_c.h:1086-1092) in full this session. Both macros are a SINGLE `__asm__
volatile` statement whose instruction sequence is a literal, fixed string (not built from any
operand-dependent control flow) — `%0` is substituted into fixed `lw`/`ctc2`/`swc2` text in a
fixed order the preprocessor never varies. GCC treats the whole statement as one opaque
`asm_operands` RTL insn (no sub-instruction scheduling freedom; confirmed s3/H3 and s7 for
gte_stlvl's reorg.c:740 non-split). Nothing outside the macro invocation — no local variable
declared before/after it, no type on an argument, no re-association of surrounding statements —
can reach inside a fixed asm string to reorder the instructions textually baked into it. This
is categorically different from the usual structural-lever targets (RA priority, CSE/combine
folds, reorg.c scheduling of REAL insns), none of which apply to a single pre-formed asm block.

Probe: `sandbox func_80052B7C --disable all --diff` (tmp/grind/func_80052B7C/s8/sandbox_diff.txt).
Result: score 19 (target/build both 24 insns), 4/4 hunks source-level, 0 operand-only, 0
not-scored — identical classification to s6/s7. Hunk 1 (chunk 1): target batches 5 lw then 8
ctc2 (interleaved loads for the halfword chunk too — the 3 `lh` in the SAME batched ctc2 run);
ours interleaves 2-lw+2-ctc2, 3-lw+3-ctc2 (gte_SetRotMatrix's fixed body) then the 3 lh/ctc2 of
gte_ldtr separately — exactly s2's finding, re-confirmed byte-for-byte this session. Hunks 2-4
(chunk 5): target's final `swc2 $11,8(a3)` sits in the `jr $ra` delay slot (hunk 3 shows it one
slot earlier in target than ours; hunk 2/4 show target's delay-slot `nop` is consumed by the
store while ours keeps an explicit `nop` after all 3 stores) — exactly s3/s7's finding.

Verdict: KILLED. kill_scope: class. measured_on: legal macro-call-only chassis
(gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h), no FAKE construct,
no written island, src/main/3AB48.c this session, reverted to INCLUDE_ASM after measurement.
predicate_cite: include/psxsdk/inline_c.h:297 (gte_SetRotMatrix's asm string, read in full this
session, confirmed literal/fixed, no operand-dependent instruction reordering possible) and
include/psxsdk/inline_c.h:1086 (gte_stlvl, same property) plus tools/gcc-2.7.2/reorg.c:740
(delay-slot filler cannot split an asm_operands insn, already cited s3/s7, re-verified against
hunk 2-4's measured placement this session).

### Frontier for the next session
Unchanged from s7: the only live lever is the engine-side `scan_hand_coded.py` gate-class
signal addition (out of grind scope; escalation/owner-cluster territory) or a new vendored
macro/DMPSX word for the batched order (needs its own owner ruling + engine: commit, and would
still leave chunk 5 unreached). No further C-level structural probe exists on this chassis —
confirmed independently in s3 and now s8 by two different sessions reading the same macro
bodies in full.

## [s8] No block-local var split / declaration-order / type-narrowing / statement-reassociation lever on the legal macro-call chassis can touch chunk 1 or chunk 5's residual, because both chunks' divergent bytes are emitted entirely from literal, hardcoded __asm__ volatile template strings baked into the vendored macro definitions, not computed from any C-level expression.
- mechanism: gte_SetRotMatrix (include/psxsdk/inline_c.h:297) and gte_stlvl (include/psxsdk/inline_c.h:1086) are each a single __asm__ volatile statement whose instruction text is a fixed string substituted only at %0/%1 operand slots; GCC treats the whole statement as one opaque asm_operands RTL insn with no sub-instruction scheduling freedom (reorg.c:740 confirmed s3/s7/this session never splits it). No C construct outside the macro invocation can reach inside a fixed asm string to reorder instructions textually baked into it.
- probe: Applied s6/s7's legal chassis (gte_SetRotMatrix(rot); gte_ldtr((s32)tr->vx,(s32)tr->vy,(s32)tr->vz); gte_ldv0(v0); gte_rt(); gte_stlvl(out); via psxsdk/inline_c.h) to src/main/3AB48.c this session, ran sandbox func_80052B7C --disable all --diff, reverted to INCLUDE_ASM afterward.
- result: score 19 (target/build both 24 insns), 4/4 hunks source-level, 0 operand-only, 0 not-scored — identical classification to s6/s7. Hunk 1: target batches 5 lw then 8 ctc2 across the whole chunk 1+2 span; ours interleaves per the fixed macro bodies. Hunks 2-4: target's final swc2 sits in the jr delay slot; ours emits an explicit nop after all 3 stores.
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call-only chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h), no FAKE construct, no written island, src/main/3AB48.c this session, reverted after measurement
- predicate_cite: include/psxsdk/inline_c.h:297

## s9 (enumerate)

### H8 — spelling_enum.py's region scan finds zero mutation axes on the legal macro-call chassis, re-confirming s5's finding on the current (floor-19) chassis

- mechanism: spelling_enum.py's SYSTEMATIC SPELLING SWEEP operates on three axes only —
  named-local declaration order, assignment reordering, and commutative-operand swaps —
  found inside an `/* ENUM-BEGIN */ ... /* ENUM-END */` region. The s6/s7/s8 legal chassis
  body is five bare macro-call statements (`gte_SetRotMatrix(rot); gte_ldtr(...);
  gte_ldv0(v0); gte_rt(); gte_stlvl(out);`) with no local declarations, no assignments and
  no binary operators of any kind in the enumerable region — s8 already established (full
  read of inline_c.h:297 gte_SetRotMatrix and :1086 gte_stlvl) that each macro's instruction
  sequence is a single fixed-string `__asm__ volatile` block, not computed from any
  C-level sub-expression, so there is nothing inside the macro bodies for a source-level
  tool to see either.
- probe: wrote the exact s6-s8 chassis to tmp/grind/func_80052B7C/s9/enum_test.c wrapping
  all 5 statements in ENUM-BEGIN/END, ran `python3 tools/spelling_enum.py --candidate
  tmp/grind/func_80052B7C/s9/enum_test.c --out tmp/grind/func_80052B7C/s9/enum --no-swaps`
  and again without `--no-swaps` (tmp/grind/func_80052B7C/s9/enum_swaps).
- result: both runs report "region: 0 named locals (), 0 assignments, 5 anchors -> 1
  distinct spellings" (the swap run: "... -> 1 distinct spellings (with commutative
  swaps)") — the tool emits only the identity spelling because its own region scanner
  finds zero instances of any axis it knows how to vary. No sweep was run against the
  sandbox score since there is nothing to sweep (a single-variant output has no
  alternatives to score).
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call-only chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/
  gte_stlvl via psxsdk/inline_c.h, identical body to s6/s7/s8's 19-floor candidate), no
  FAKE construct, no written island, tool run against a standalone test file
  (tmp/grind/func_80052B7C/s9/enum_test.c), no src/ edit made this session
- predicate_cite: tools/spelling_enum.py:27 (named-local axis), :32 (assignment axis) —
  the region-scan precondition spelling_enum.py itself enforces; corroborated by
  include/psxsdk/inline_c.h:297 and :1086 (each macro's body is one fixed __asm__ string,
  already read in full at s8, so there is no sub-expression surface inside the macros
  either)

## [s9] spelling_enum.py's region scan finds zero mutation axes (named locals, assignments, commutative swaps) on the s6-s8 legal macro-call chassis, re-confirming s5's finding at the current floor-19 body.
- mechanism: spelling_enum.py enumerates only named-local declaration order, assignment reordering, and commutative-operand swaps inside an ENUM-BEGIN/END region; the chassis body is five bare macro-call statements with no declarations, assignments, or binary operators, and each macro (gte_SetRotMatrix, gte_stlvl) is a single fixed-string __asm__ volatile block per s8's full read of inline_c.h:297/:1086, so there is no sub-expression surface inside the macros either.
- probe: wrote the exact s6/s7/s8 chassis to tmp/grind/func_80052B7C/s9/enum_test.c with all 5 statements wrapped in ENUM-BEGIN/END; ran spelling_enum.py --no-swaps and again with the swap axis enabled.
- result: Both runs: 'region: 0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings' (identity output only). No sweep was run against the sandbox score since a single-variant output has no alternatives.
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call-only chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h, identical body to s6/s7/s8's 19-floor candidate), no FAKE construct, no written island, tool run against a standalone test file, no src/ edit made this session
- predicate_cite: tools/spelling_enum.py:27

## s10 (solver)

### H9 — [s10] The residual's first divergence is RA (classify, object mode), and the target register disposition (8 data GPRs = t0..t7 / $8-$15, v0/v1 untouched) is not produced by GCC 2.7.2's allocator for any pseudo set on this function; it can only come from a fixed asm string naming $8-$15, which no vendored macro contains.
- mechanism: classify (inverse_compose.py, --target-object build/src/main/3AB48.o --ours-object tmp/sandbox/func_80052B7C/main/3AB48.o) reports "FIRST DIVERGENCE: RA" — register-blanked multisets are equal (24 insns each; target's extra nop after mvmva balances our jr-slot nop). mips.h (tools/gcc-2.7.2/config/mips/mips.h) defines no REG_ALLOC_ORDER, so local-alloc.c:2249 find_free_reg scans hard regs 0..N and takes the lowest free one; v0 ($2) and v1 ($3) are free across the whole body (void return, no calls), so any allocated GR_REGS pseudo lands on $2 first. The .lreg dump confirms this on the legal chassis: TR pseudos 77/80/83 -> regs 4/3/2 (a0/v1/v0). The rotation loads' $12-$14 are hardcoded inside gte_SetRotMatrix's fixed string (inline_c.h:297); target's $8-$11 appear in no vendored lw (grep `lw\s+\$(8|9|10|11),` across inline_c.h/inline_o.h/gtemac.h/libgte.h = 0 hits).
- probe: (1) classify in WSL (tmp/grind/func_80052B7C/s10/classify.txt). (2) dump.ps1 .lreg read. (3) Scratch RA-reachability probe (tmp/grind/func_80052B7C/s10/ra_probe.c, banked as rejected/ra-probe-8-operand-written-island.c — a written island, NEVER a candidate): all 8 values as C pseudos fed to ONE asm via 8 "r" operands, the most allocator-free spelling possible; compiled with tools/gcc-2.7.2/cc1 and the Makefile CC_FLAGS.
- result: probe assigns {lh $8,$3,$2 ; lw $9..$13} -> v0,v1,t0..t5, scheduled lh-first; target is lw t0..t4 then lh t5..t7. Even with every register left to the allocator, v0/v1 are taken first, so t0..t7 is not an allocator output. Sandbox on the legal chassis re-measured 19 (4/4 hunks source-level, tmp/grind/func_80052B7C/s10/sandbox_diff.txt). fake_ablate: no FAKE constructs in candidate.c (KILL RE-AUDIT: nothing to ablate; the current chassis IS the closest instance-kill form and re-measures 19).
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call chassis (gte_SetRotMatrix/gte_ldtr((s32)..)/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h), no FAKE construct, applied to src then reverted; plus standalone scratch probe with an 8-"r"-operand asm (not a candidate)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2249

## [s10] The residual's first divergence is RA, and GCC 2.7.2's allocator does not assign target's register disposition (8 data GPRs in $8-$15 with v0/v1 unused) to allocated pseudos in this function; those registers come only from a fixed asm string, and no vendored macro names lw $8-$11.
- mechanism: mips.h defines no REG_ALLOC_ORDER, so local-alloc.c:2249 find_free_reg takes the lowest free hard reg; v0/v1 are free across the body (void, no calls). .lreg on the legal chassis: TR pseudos 77/80/83 -> 4/3/2. Vendored headers hardcode only $12-$15.
- probe: inverse_compose.py classify in object mode (WSL); dump.ps1 .lreg read; scratch probe feeding all 8 values as C pseudos to one asm via 8 "r" operands, compiled with tools/gcc-2.7.2/cc1 + Makefile CC_FLAGS; grep census of lw $8..$11 in inline_c.h/inline_o.h/gtemac.h/libgte.h.
- result: classify: FIRST DIVERGENCE RA (blanked multisets equal). Probe: lh $8,$3,$2 + lw $9..$13 (v0,v1,t0..t5, lh-first) vs target lw t0..t4 then lh t5..t7. Census 0 hits. Legal chassis re-measured 19, 4/4 hunks source-level.
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call chassis gte_SetRotMatrix/gte_ldtr((s32)..)/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h, no FAKE construct, applied to src and reverted; plus standalone scratch 8-operand probe (not a candidate)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2249

## [s10] KILL RE-AUDIT: the closest banked instance form (the legal macro-call chassis) re-measures below 19 once its FAKE constructs are ablated.
- mechanism: Brief-mandated re-audit of instance kills after a 3-session flat floor.
- probe: fake_ablate.py on memory/grind/func_80052B7C/candidate.c; sandbox --disable all --diff on the chassis applied to src.
- result: fake_ablate: no FAKE-annotated constructs, nothing to ablate. Sandbox: 19, unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis via psxsdk/inline_c.h, no FAKE construct, src/main/3AB48.c this session, reverted
