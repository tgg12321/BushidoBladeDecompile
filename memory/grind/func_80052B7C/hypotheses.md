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
