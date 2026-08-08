# SELF-VET — func_80021A98

This vet covers the v6 diff (grind session 2, synthesis modality, 2026-08-08).
The diff surface is src/code6cac.c ONLY; the header file is untouched. None of
the constructs this function's ban list enumerates appears in the diff in any
form; per prior operator guidance the bans are not quoted or described here —
see evidence.md (session "synthesis 2026-08-08") and candidate.c's header for
the full prose account.

CONSTRUCTS: CA direct assignment of the first arm's second lookup sum into the
0x58 field; CB direct assignment of the alternate arm's second lookup sum into
the 0x58 field (mirror of CA, no intermediate local of any kind); CC
FAKE-annotated do-while(0) wrap around the 0x6A store; CD FAKE-annotated empty
do-while(0) between the 0x60/0x61 byte stores; plus one DELETION (a
constant-holder local was removed and its store now uses the literal — measured
score-neutral, per the Judge notice's preferred branch).

## T1 semantic purpose: CA and CB compute and store exactly the value the
function's behavior requires; they are live stores with full semantic purpose.
CC's inner store is likewise a required, live store; the wrap itself and CD have
no semantic purpose of their own — they are match devices, which is why both
carry the mandatory inline FAKE annotation and rest on the family claimed below.
The deletion only removes a device; the literal store keeps the behavior.

## T2 human-programmer: CA/CB are the most natural way to write "field =
lookup sum" — no intermediate, symmetric across both arms. A reader would not
question them. CC/CD would prompt "why is this here?" — answered at the
construct site by their FAKE annotations, as the sanctioning rule requires.

## T3 GCC-internals justification: CA/CB are justified by program logic alone
(store the computed value); no internals reasoning is needed or used. CC/CD are
justified by observed effect (removal moves the measured score 0 -> 2 for CD;
CC closes the remaining 4-insn seating diff), stated effect-level in their
annotations per the rule's prerequisite; they are carried by the family claim
below, not by an internals argument.

## T4 permuter/search provenance: the wrap shape (CC) and the first-arm fold
(CA) were first surfaced by the session-9 permuter campaigns, then individually
re-measured and hand-vetted; the raw search output's spurious extras were
rejected and banked at the time. CB was derived this session by hand as the
natural mirror of CA, replacing a rejected earlier construct — it is not a
search artifact. CD predates this session and was re-measured load-bearing this
session. Every construct's individual effect is measured, not search-asserted.

## T5 family check: CA/CB are ordinary assignments — no family needed. CC/CD
are exactly the sanctioned do-while(0) match-device family claimed below (CD is
the empty-body shape the same rule text covers). No forbidden-catalog entry
matches any construct: no pins, no asm, no volatile, no alias handles, no
unused locals or arrays, no dead stores, no always-taken wrappers of other
shapes, and none of this function's banned forms in any respelling.

## T6 naming-announces-intent: the only identifiers the diff touches are
pre-existing descriptive locals (a3, v0, v1, a0_58, a1_val, v1_58) named for
the registers/fields they mirror; the deletion removes the one holder-style
name. No pad/dummy/unused/spill-style name exists in the diff.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap (covers CC and CD)
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:23
  PRECEDENT: cf3e6ce7

ANNOTATION-CONFORMANCE: two FAKE annotations, quoted verbatim below; each names
the observed effect at the construct site (effect-level is sufficient per
.claude/rules/do-while-zero-exception.md:55-58); lever exhaustion for this
seating is documented across the ledger (hypotheses.md sessions 1-2 probe kills
P1-P13; evidence.md session-9 re-baseline showing the plain form floors at 20
and the surviving alternatives measured worse).
  /* FAKE: load-bearing match device — removing this empty do-while(0)
   * moves the sandbox score 0 -> 2 (measured 2026-08-08); mechanism:
   * the sanctioned do-while(0) wrap's codegen effect on the seating
   * of the surrounding byte stores (do-while-zero-exception.md,
   * owner ruling 2026-07-06). */
  /* FAKE: the do-while(0) wrap's weighting seats a0_58 in $a0
   * and a1_val in $a1 as in target (cluster-2 $4/$5
   * close-out). */
