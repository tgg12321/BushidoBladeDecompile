# SELF-VET — func_800401CC
CONSTRUCTS: s32-widened local v (was s16), lowmask named intermediate local (live, set once, read twice), v = 0xFF000000 staged-mask assignment (FAKE-annotated)

## T1 semantic purpose: 
- `s32 v`: type choice of a live local; v holds the halfword texture-V read and later the 32-bit OT mask — s32 is required for the second (real, read) value. Emitted head bytes are identical to the s16 spelling (lhu load + (s16) casts at the call are unchanged); the tail reads of v are real ANDs. Not a no-op construct.
- `lowmask`: holds 0xFFFFFF which is read by both packet-link statements — live, used, observable in the emitted ANDs. Removing it changes the emitted bytes (probe C measured score 2 without it), so it is not byte-identical-with-or-without.
- `v = 0xFF000000`: the assigned value is read by four emitted instructions (two ANDs feeding two ORs/stores). Live code, zero dead stores. The CHOICE of the existing v local over a fresh local is codegen-motivated — that choice is exactly what the claimed sanctioned family covers and is FAKE-annotated.

## T2 human-programmer: 
- `lowmask`: naming the 24-bit OT address mask is idiomatic graphics code (PsyQ OT-link idiom); a human writes this naturally.
- `s32 v` + `v = 0xFF000000`: a human writing from spec would more likely use a second named mask local; borrowing the finished-with v-coordinate variable is the "why is this here?" construct. Answered honestly: this is the staged-value-reused-variable family's defining trait; SOTN ships the same shape in matched code ("fake reuse of i", `j = menu->unk1D; // FAKE?`). Annotated accordingly.

## T3 GCC-internals justification: 
- The v-borrow's justification IS GCC-internal (global.c call-arg copy preference lands the multi-set pseudo in $a3; local-alloc.c qty_compare_1 then gives the lone remaining mask qty $a2) — declared openly in the FAKE annotation as the family's rules require. The staged VALUE itself is real program logic (the OT high-byte mask actually used).
- `lowmask` set-order bias (restores the target li+ori/li emission order) is the sanctioned named-intermediate LUID-bias technique.

## T4 permuter/search provenance: 
None. No permuter or auto-search ran this session; every form was hand-derived from the s1 QTYDBG mechanism analysis and measured individually with sandbox + the instrumented-cc1 QTYDBG dump (artifacts in tmp/grind/func_800401CC/s2/).

## T5 family check: 
- `v = 0xFF000000`: staged-value-reused-variable (sanctioned 2026-07-03). All bounds hold: (1) value real and read by the next two statements; (2) v exists for a real job (texture V coordinate loaded from the record table, passed to the draw call); (3) borrow provably safe — v's prior value is consumed by the SetDrawMove call and never read afterward, and the staged mask is not needed past the function's last store; (4) annotated with what+mechanism+lever-exhaustion; (5) last resort with receipts (hypotheses.md s1 K1-K4 kills, s2 P1/P2 measured kills, K1c proves the both-fresh-locals spelling cannot flip the allocation).
- `lowmask`: named-intermediate declaration order (sanctioned SOTN family) — a sub-expression named as a separately-declared local to bias LUID/emission order. Live and read twice; not a dead scalar, not a constant-holder-across-calls, not frame coercion.
- Checked against the forbidden catalog: no dead stores, no unused locals/arrays, no pins, no asm, no volatile, no alias renames, no param dead-assign (the a2-param forms P1/P2 were probes, measured, REJECTED, and are not in the diff).

## T6 naming-announces-intent: 
`lowmask` names its semantic content (the low-24-bit OT address mask), not a coercion role. `v`, `u`, `buf`, `tbl`, `pkt`, `ot` are pre-existing names. No pad/dummy/unused/spill/slack names.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:58

  FAMILY: named-intermediate declaration order
  SCOPE: "Named-intermediate declaration order ... declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189

ANNOTATION-CONFORMANCE: One FAKE construct. The emitted annotation (src/text1a_pre.c, immediately above the staged assignment):
  /* FAKE: OT-code mask staged through the dead v-coord local (its value is
     consumed by the draw call above; the mask is read by both packet-link
     statements below), mechanism: global.c call-arg copy preference keeps
     the multi-set pseudo in $a3 while local-alloc.c qty_compare_1 gives the
     remaining single mask qty $a2, lever-exhaustion: grind ledger
     hypotheses.md K1-K4 + s2 P1/P2 (param-reuse forms measured broken) */
It carries all three template elements: what (the staged OT-code mask + which dead local), mechanism (named GCC passes: global.c preference + local-alloc.c qty_compare_1), lever-exhaustion (ledger hypotheses.md K1-K4 + s2 P1/P2). The mechanism differs from the family's origin case (RA preference rather than sched.c adjust_priority); the rule's bounds are shape-defined (real used value, existing dead variable, safe borrow, annotation, receipts) and the annotation template explicitly takes any named compiler pass — flagged here for the reviewer's attention rather than hidden.
