# SELF-VET — _SsVmInit (s8, synthesis modality; MATCH at honest distance 0)

CONSTRUCTS: `u16 masked = (u8)a0;` (named local holding the clamp's masked
parameter value, read twice — in the `>= 0x18` compare and in the else-arm
store); `u16 i;` (shared loop counter for the three fixed-trip-count clear
loops and the per-voice loop); `s32 offset;` (per-iteration byte offset into
the 54-byte-stride voice-slot array); per-word byte-offset store spelling
`*(s16 *)((u8 *)&D_800F4E1A + offset) = ...` (21 sites, the established
object-model convention of this TU for the voice-slot array); three
`do { ... } while (cond);` post-test clear loops. No `__asm__`, no register
pins, no volatile, no dead stores, no pads, no FAKE constructs of any kind.

## T1 semantic purpose
- `masked`: yes. It holds the clamp's input value — the low byte of the
  parameter — and is both compared against the limit and stored into
  `_SsVmMaxVoice`. It is the value the function computes; removing it forces
  the expression to be written twice.
- `i` / `offset`: ordinary loop counter and ordinary index arithmetic; both
  are read by every statement that follows them.
- the byte-offset store spelling: it is how this TU addresses element *n* of
  the 54-byte-stride voice-slot array whose fields are declared as individual
  per-word splat symbols; the offset is the real array index arithmetic.
- the `do/while` loops: each clears a fixed-size array; the trip counts are
  the arrays' real sizes (0xC0, 0x18, 0x10) and the loop bodies do the work.
Every construct changes what the function computes or is the computation.

## T2 human-programmer
Yes. `unsigned v = (unsigned char)max_voice; if (v >= 24) g = 24; else g = v;`
is the natural way to spell "clamp the caller's voice count to 24" when the
parameter is read as a byte, and a reader never asks "why is this variable
here?" — it is the clamped value itself. The loops and the index arithmetic
(`offset = i * 54;`) read as ordinary array initialisation code. Nothing in
the body exists for a reason a reader could not infer from the specification
("initialise the SPU voice manager: clear the tables, clamp the voice count,
initialise each voice slot, reset the global attributes, flush").

## T3 GCC-internals justification
None is claimed for any construct in the final body. The reasoning that led
to `masked` is a *target-asm observation*, not a pass-internals mechanism:
target computes the masked value once and stores that same value
(`andi $a0,$s1,0xFF` … `sb $a0`), so the C must name it once and use it in
both places. No allocator/scheduler/DCE/RTL-order argument is load-bearing,
and this session REMOVED every construct that had only a codegen rationale
(the `ff` constant hoist, the `offset = 1` variable reuse, the `idx` copy and
the expanded shift-subtract stride spelling all re-measured at distance 0
when deleted, so all four are gone).

## T4 permuter/search provenance
No construct comes from a search tool. The winning form was derived by hand
from the target listing (`asm/funcs/_SsVmInit.s:52-62`) after re-reading the
ledger and noticing that every prior session's variants left the else arm
storing the raw parameter. `tools/sweep_variants.py` was used only to *score*
eight hand-written spellings in one batch, and the four simplification passes
afterwards deleted constructs rather than adding any. The s5 permuter
campaign (~26k iterations) contributed nothing to this body.

## T5 family check
No construct matches a forbidden family. Specifically:
- `masked` is not a constant-holder, not a dead scalar, not a spill carrier
  and not an alias: it is written once with a real value and read twice, and
  its value reaches the function's output (`_SsVmMaxVoice`). It is not the
  named-intermediate FAKE family either — that family exists for values that
  are byte-neutral; this one *changes* the bytes because it is the value
  target stores.
- The byte-offset store spelling is the object model this TU already ships in
  two bytes-proven, accepted-on-main siblings touching the SAME base symbols
  and the SAME 54-byte stride: `src/main.c:984` (func_800858D0,
  `*(s16 *)((u8 *)&D_800F4E1A + offset) = u;`) and `src/main.c:1419` /
  `src/main.c:1429` (_SsVmKeyOffNow, `*(s8 *)((u8 *)&D_800F4E35 + idx) = 0;`
  and `*(s16 *)((u8 *)&D_800F4E18 + idx) = 0;`). This ledger recorded the
  same verdict at s1 (H1, CONFIRMED). NOTE FOR THE REVIEWER: the dispatch
  brief's automated DECLARATION-PUNS scan flags these 21 sites and points at
  the aggregate-merge family, whose sanctioned fix is a header-canonical
  struct merge performed through an integration handoff. I cannot edit
  `include/*.h` inside my session surface, and a TU-local struct is
  explicitly NOT the sanctioned spelling, so this body uses the convention
  its two matched siblings already use rather than inventing a third. If the
  reviewer reads the convention as requiring the declaration-level merge, the
  correct disposition is an integration handoff on the declarations for the
  whole `g_satan1_slot_state_table` family (this function plus the still-
  INCLUDE_ASM siblings _SsVmKeyOnNow / vmNoiseOn / SsUtKeyOnV / _SsVmFlush),
  not a respelling inside this function.
- No forbidden-family construct is present in any spelling: no register-asm
  pin, no hardcoded-`$N` asm, no scheduling barrier, no volatile coercion, no
  unused local array, no dead param assign, no dead conditional store, no
  empty-body `if`, no `if (1)`, no dead goto, no DImode chain, no opaque
  constant variable, no alias rename, no build-time asm rewriting.

## T6 naming-announces-intent
No name in the body announces coercion intent. The locals are `masked`
(the masked parameter value), `i` (loop counter), `offset` (byte offset into
the voice-slot array) and `buf` (the 16-word command buffer passed to
`func_8008B488`). There is no `pad`, `dummy`, `unused`, `spill`, `slack`,
`tail` or `_buf`; every local is read after it is written.

SANCTIONED-FAMILY-CLAIMS: none — the body contains no FAKE construct and
claims no sanctioned-family carve-out.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff (nothing in the
body requires a `/* FAKE: ... */` annotation; the three constructs that would
have needed one under earlier chassis — the `ff` constant hoist, the
`offset = 1` variable reuse and the expanded shift-subtract stride — were all
deleted this session after each re-measured at distance 0 without them).
