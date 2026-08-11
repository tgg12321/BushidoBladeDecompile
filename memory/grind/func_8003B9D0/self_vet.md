# SELF-VET — func_8003B9D0

Diff under review = `src/code6cac_c2.c` at HEAD vs the body now in the working
tree (identical to `memory/grind/func_8003B9D0/candidate.c`).  Re-applied and
INDEPENDENTLY RE-MEASURED this session: `sandbox func_8003B9D0 --disable all`
prints `score 0`, `target_insns 185`, `build_insns 185`, `rules_dropped 1`,
`strip_cheat_asm true`.

Three changes vs HEAD:
  (D1) DELETION of three cheat-asm constructs (one
       `__asm__ __volatile__("" : "=r"(eda) : "0"(eda))` identity-reload
       barrier and two `__asm__ __volatile__("" ::: "memory")` scheduling
       barriers).
  (D2) the two flag-selected argument initialisations spelled
       `if (c) x = V; else x = -1;` instead of `x = -1; if (c) x = V;`
       (the session-2 form).
  (D3) the `+0x44C` halfword's address staged through the function's existing
       `u8 *p` scratch local — `p = (u8 *)&eda[0x226]; saved_44c = *(s16 *)p;`
       — instead of `saved_44c = eda[0x226];`.  Carries a `/* FAKE: ... */`
       annotation.

CONSTRUCTS: staged-address-through-existing-scratch-pointer (D3); if/else
spelling of a two-way initialisation (D2); (D1 is a deletion, not a construct)

## T1 semantic purpose:
- D3: the statement pair has real semantic content — it computes the address of
  the halfword the function must save and dereferences it to produce
  `saved_44c`, which is later restored to that same location.  Nothing is dead:
  `p` is written and read, and the value it yields is used.  What has no
  observable effect is the CHOICE to route through `p` rather than writing
  `eda[0x226]` directly — the two spellings are semantically identical.  That is
  why it is annotated FAKE rather than presented as ordinary code.
- D2: both spellings are semantically identical; the if/else is if anything the
  more direct statement of "this argument is either the global or -1".  Nothing
  dead, nothing unused.
- D1: deletion — strictly reduces coercion.

## T2 human-programmer:
- D2: yes, unremarkable C; a reader would not ask why it is there.
- D3: a reader WOULD ask why the save goes through `p` rather than reading
  `eda[0x226]` directly.  I am not claiming otherwise.  It is defensible as an
  original-source shape (`p` is a general scratch pointer this function already
  reuses for another role later in the body), but I am not treating that as
  proof — it is claimed under the sanctioned variable-reuse / pointer-alias
  families with a FAKE annotation, not as a construct a human writes naively.

## T3 GCC-internals justification:
Honest answer: YES for D3 — the reason the spelling changes the bytes is a GCC
mechanism (cse.c `find_best_addr`'s unconditional address fold at cse.c:2663
substituting the base pseudo's `qty_const` in `fold_rtx`, cse.c:5171-5176), and
that mechanism is named in the FAKE annotation as the rule requires.  The
construct is therefore NOT presented as program logic; it is presented as a
last-resort codegen-driven spelling inside a sanctioned family, after the
documented exhaustion below.  D2 needs no GCC-internals justification to be
written (it is ordinary C), though its measured effect is also a cse effect.

## T4 permuter/search provenance:
D3 originated as a PROPOSAL from the permuter campaign
(`tmp/grind/func_8003B9D0/s4/wsA`, output-200-2, permuter score 330 -> 200).  It
was NOT taken as a winner: the permuter's own form read a BYTE through `p`
(`saved_44c = *p;`, semantically wrong vs target's `lh`) and its sibling
output-200-1 was undefined behaviour (uninitialised `eda`).  Both were rejected.
The submitted form is the halfword-correct derivation, independently
re-measured on the real pipeline (sweep2.py / sweep3.py + sandbox, and again
from a clean HEAD src this session).  It does not pass detectors "because they
miss this spelling" — it is a live, dereferenced pointer assignment: not a dead
store, not a pin, not asm, not volatile.

## T5 family check:
D3 matches the FROZEN sanctioned families "Variable reuse for codegen control"
and "C-level pointer aliases to globals" (both claimed and cited below).  It is
not a member of any forbidden one: there is no register pin, no hardcoded-`$N`
asm, no regfix insert, no scheduling barrier, no volatile coercion, no
`asm("sym")` alias rename, no unused local or array, no dead store, no dead
conditional store, no `if (1)` wrapper, no goto/label padding, no
declaration-order trick, and no redundant width cast (the `(s16 *)` cast
preserves the halfword access width that target's `lh` performs — dropping it
would change semantics).
D2 belongs to no family at all — it is ordinary control flow with identical
semantics; no exception is claimed for it.

## T6 naming-announces-intent:
No new identifiers are introduced by D3 at all — it reuses the pre-existing `p`.
No `pad`/`dummy`/`unused`/`spill`/`tmp`/`slack` naming anywhere in the diff.
Every declared local in the function is written AND read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: .claude/rules/no-new-park-categories.md:170
  FAMILY: C-level pointer aliases to globals
  SCOPE: "a local pointer that provides a second C handle to a global — where using the global directly would be semantically identical — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: .claude/rules/pointer-alias-fake-exception.md:13

LEVER-EXHAUSTION (verifiable in hypotheses.md, not merely claimed):
  K1/K7 own-local for the displaced address (measured twice, folds);
  K2 pointer rebasing at the other end (24, worse);
  K4 two independent pointers at offset 0 (17, worse);
  K5 no pointer at all / plain globals (23, worse);
  K8 ten access/type spellings in a mini TU, all fold;
  K9 30 matched siblings with this addressing shape — no spelling to copy;
  H3 the if/ELSE cse-boundary form closes region A but costs 11 (magic + la
  placement);
  K10 six dedicated-pointer spellings — block-scope init-at-decl, block-scope
  split decl/assign, assign-before-the-zero-offset read, `u8 *` + cast,
  function-scope all-three-accesses, function-scope read-only — none reaches
  185 insns (all 188).

ANNOTATION-CONFORMANCE:
  /* FAKE: the +0x44C halfword is reached through the scratch pointer
     already declared above instead of directly as eda[0x226];
     mechanism: cse.c find_best_addr's unconditional address fold
     (cse.c:2663) substituting the base pseudo's qty_const in fold_rtx
     (cse.c:5171-5176), which the staged pointer defeats;
     lever-exhaustion: memory/grind/func_8003B9D0/hypotheses.md
     K1/K2/K5/K8/K9 (10 access/type spellings + 30-sibling census). */
  — carries WHAT (staged address through the existing scratch pointer),
  MECHANISM (named GCC pass: cse.c find_best_addr / fold_rtx, with line cites),
  and LEVER-EXHAUSTION (pointer into the hypothesis ledger).
