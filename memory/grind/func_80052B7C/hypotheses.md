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
