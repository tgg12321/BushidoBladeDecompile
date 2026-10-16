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
