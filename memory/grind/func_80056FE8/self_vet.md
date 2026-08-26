# SELF-VET — func_80056FE8 (ang_hosei_80056FE8), grind session s7 (solver)

CONSTRUCTS: none (no match-hack construct of any family). The diff replaces
`INCLUDE_ASM("asm/funcs", func_80056FE8);` in src/text1b.c with a plain 14-line
C body. Its only notable shape is an ACCUMULATOR: `s32 base` is initialised to
`a3 * 40` and each dispatch arm adds its own angle adjustment into it
(`base += <arm-specific table lookup> * <arm-specific scale>`), instead of
caching the adjustment in a shared temporary and adding it once after the
if/else. The three arms add THREE DIFFERENT expressions (three different tables
D_8009A830/D_8009A838/D_8009A840, two different scales, two different index
fields), so this is not a duplicated statement — it is the ordinary
accumulate-as-you-go spelling. There is no variable reuse, no dead store, no
dead local, no constant holder, no pointer alias, no volatile, no do-while(0),
no register pin, no inline asm, no goto, no local array, no cast widening added
for codegen purposes.

## T1 semantic purpose
Every construct in the diff is load-bearing for the function's value. `a2` is
the struct pointer read from *arg0, used for four field reads. `a3 * 40` is the
per-character table stride. Each arm's `base +=` contributes that arm's angle
adjustment to the returned value. The final expression re-reads *arg0 and adds
the +0x40A field and the +300 bias. Delete any of it and the returned value
changes. Nothing in the body is byte-identical-with-or-without.

## T2 human-programmer
Yes — this is what a human writes from the specification "start from the
character's base angle slot, add the situational correction for whichever case
applies, then add the stored offset and the 300 bias". The DRY alternative
(cache the correction in a temp, add once) is equally natural; both are ordinary
C, and a reader would not ask "why is this here?" about any line. Notably s7's
form is SIMPLER than the s5/s6 floor-9 candidate it replaces, which carried a
deliberate double variable-reuse tail; that coercion is now gone entirely.

## T3 GCC-internals justification
Not required as the mechanism. The C stands on its own as the natural spelling;
the RA explanation (per-arm accumulation gives `base` 8 references instead of 4,
which raises its allocno priority above the struct pointer's so global.c colors
it first and it takes $a1) is POST-HOC — it explains why this ordinary form is
the one that matches, exactly as the owner adjudicated for the
hoist-shared-arm-computation lever on 2026-07-12. The solver was used to
identify WHICH honest quantity to move (reg_n_refs of pseudo 77), not to
manufacture a construct whose only reason to exist is a GCC internal. Remove the
GCC reasoning entirely and the C is still the C a decompiler would write.

## T4 permuter/search provenance
No permuter output is in this diff. The s4/s5 permuter's only weighted-0 form
was `base++; base--;` (a dead-op cheat) and it is NOT what landed here — s7 did
not run a permuter at all. The path was: RA-solver triage (RA layer), inverse
search (vector "[refs_up] pseudo 77: refs 4->8"), then a hand-written C form
whose ordinary semantics happen to produce that ref count. The form was then
simplified twice further (e1 -> e2 -> e3) purely to remove leftover artifacts,
each simplification re-measured at score 0.

## T5 family check
No forbidden family applies: no register-asm pin, no hardcoded-$N asm, no regfix
insert, no scheduling barrier, no volatile coercion, no alias rename, no dead
local/array/param assign, no dead conditional store, no always-true wrapper, no
dead goto/label pad, no DImode chain, no opaque constant variable, no redundant
width cast, no linker/rodata reorder. The nearest-sounding family,
duplicated-statement-into-arms, is NOT claimed and does not describe this shape:
each arm's statement is a DIFFERENT expression, and the arms already existed in
the target's control flow.

## T6 naming-announces-intent
No name in the diff is `pad`/`dummy`/`unused`/`spill`/`tail`/`slack`/`_buf` or
any coercion-announcing synonym. `a2`, `a3`, `base`, `arg0` are the existing
decomp-style names already used throughout src/text1b.c; every one of them is
read for its value.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct

## Verification performed this session
- `sandbox func_80056FE8 --disable all` (cheat-invisible, rules dropped 0):
  score 0, build_insns 43 == target_insns 43.
- `verify-oracle`: full clean build, `build_sha1` ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa == locked oracle, `ok: true`.
- `tools/ra_solver/extract.py` on the final body confirms the predicted
  allocation mechanism: pseudo 77 (base) refs 8 / pri 10434 colored first ->
  $a1; pseudo 73 (struct pointer) -> $a2 — target's disposition.
