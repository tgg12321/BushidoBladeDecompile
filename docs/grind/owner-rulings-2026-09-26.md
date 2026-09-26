<!-- Provenance: recorded verbatim by the orchestrator from the owner's (Trenton's) AskUserQuestion answers on 2026-09-26; batch 1 (originally tmp/orch/owner_rulings_2026-09-26.md) then batch 2 (originally tmp/orch/owner_rulings_2026-09-26b.md), concatenated unedited (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (conversation with orchestrator, via AskUserQuestion) — VERBATIM RECORD

Context given to the owner (orchestrator's plain-language framing, which the owner approved):

## Q1 — Reused variables
Question: "Allow a local reused for several unrelated values when compiler dumps prove no one-variable-per-value spelling can match?"
Owner chose: **"Allow with proof (Recommended)"** — option text: "Admit only when allocator-dump proof shows necessity, honest generic name, and layer-2 still reviews. Unblocks 80055138, 8003993C, 8002DE20 (C part)."
Framing: "Allow it only with proof: the worker must show from compiler dumps that no single-purpose spelling can match, give the variable an honest generic name, and still pass layer-2."

## Q2 — Table merge (aggregate merge prong (a))
Question: "Accept 'only one object compiles to these bytes' (confirmed by the original PsyQ compiler) as evidence for merging adjacent data labels into one table?"
Owner chose: **"Accept, minimal span (Recommended)"** — option text: "Compiler-necessity proof + cc1psx confirmation + merged object limited to the labels actually used. Unblocks 800620B8, likely 8005D814."
Framing: "accept compiler necessity as evidence when it's confirmed by the original compiler and the merged object is the smallest span that covers the labels actually used."

## Q3 — Inner-loop break (Ruling 9 prong (c))
Question: "Ruling 9 prong (c): may a `break` that only exits an inner loop sit between the write and its use?"
Owner chose: **"Yes, inner-loop break OK (Recommended)"** — option text: "The break never skips the use. Unblocks func_8006F97C (full match, everything else already accepted)."

## Q4 — inline_o.h GTE blocks
Question: "Grant GTE blocks copied verbatim from PsyQ's inline_o.h (with its `move $12` setup step) as a class?"
Owner chose: **"Grant as a class (Recommended)"** — option text: "Character-identical to a pinned inline_o.h copy; no per-function owner row needed."
Framing: "grant this class, provided the block matches a pinned header copy character for character."

Not decided (left for separate investigation): func_80036140's build-model changes (-G8 per file, maspsx COMMON no-gp model).

# Owner rulings 2026-09-26 (second batch, via AskUserQuestion) — VERBATIM RECORD

Context given to the owner: func_80034708 (and func_80036140) need a new per-file -G8 TU. Evidence for func_80034708: the target reads D_800A3174 gp-relative 16 times, the neighbouring functions none; the original PsyQ cc1psx emits those gp reads at -G8 and none at -G0. The 2026-09-26 first batch explicitly left per-file -G8 undecided.

## Q5 — Per-file -G8
Question: "Allow giving a function its own source file compiled at -G8 (small-data setting) when the shipped code proves it — gp-relative reads in the original bytes that neighbours lack, confirmed by the original PsyQ compiler producing them only at -G8?"
Owner chose: **"Allow with that proof (Recommended)"** — option text: "Requires gp-relative accesses in the original bytes + cc1psx confirmation + neighbours moved unchanged; layer-2 still reviews. Unblocks func_80034708 (and part of func_80036140)."

## Q6 — `0($12)` in the inline_o.h class
Question: "Should the inline_o.h class grant also accept the `0($12)` spelling our assembler tool forces in place of the header's `($12)`?"
Owner chose: **"Keep strict wording"** — option text: "Only exact character copies; func_8002DE20's blocks need a per-function grant."
(No rule change; this confirms 262db111c's prong (C) as written.)

Not decided: func_80036140's maspsx COMMON-no-gp model (a maspsx behaviour change) — Q5 covers only the per-file -G8 part.

## Q7 — Mixed-field struct under the aggregate-merge compiler-necessity alternative
Question: "Your table-merge ruling covered one repeated record type. func_80034708 needs 0x78–0x87 declared as ONE struct with mixed fields (a u16 pair, four byte pairs, four bytes). Both compilers, including the original PsyQ one, match the shipped code only with that struct, and it spans exactly the bytes the function uses. Extend the ruling to mixed-field structs under the same proof?"
Owner chose: **"Allow with same proof (Recommended)"** — option text: "Same dump proof + cc1psx confirmation + minimal span of bytes actually used; layer-2 still reviews. Unblocks func_80034708 (with -G8)."
