# SELF-VET — _exeque (session 4, permuter)

CONSTRUCTS: (1) `s32 *p = &D_8009BE7C;` (pointer alias to a global, carried
unchanged from s2 H5b); (2) outer `do { ... } while (0);` wrap around the
post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70), NEW this
session, `/* FAKE */`-annotated; (3) NESTED `do { ... } while (0);` wrap
around just the first two stores (D_8009BF68[0]/D_8009BF6C), NEW this
session, `/* FAKE */`-annotated.

## T1 semantic purpose
(1) `p` has an observable effect: dereferenced for both the guard read and
the clear store — removing it changes how the address is computed (two
separate named-global accesses vs one shared pointer). (2) and (3): a
`do { ... } while (0);` wrap has NO observable effect on the STATEMENTS
inside it — the same assignments execute in the same order with the same
values whether or not the wrap is present. This is the honest answer for
every do-while(0) wrap; it is why the family is FAKE-annotated rather than
treated as ordinary C. The wrap's only effect is on the compiled RTL/insn
boundary the scheduler sees (measured: floor 12->7->2), not on runtime
behavior.

## T2 human-programmer test
(1) Yes — a programmer would naturally write `s32 *p = &D_8009BE7C;` for
"the guard flag we're about to test-and-clear." (2)/(3) No — a programmer
writing "save the queue slot's function/arg/count into the debug-record
globals" would NOT spontaneously wrap two or three of those four
assignments in a `do { } while (0);` with no loop semantics; a reader would
ask "why is this here?" This is the honest T2 answer for both wraps, which
is exactly why they require the FAKE annotation + family membership rather
than passing as ordinary C.

## T3 GCC-internals justification test
(1) The pointer's justification does reference `defeat-combine-symbol-fold`
by name, but per [[ordinary-c-judge-decidable]] Ruling 1(3) a construct
with a real semantic reading is not a cheat merely because GCC internals
informed its selection; the pointer is genuinely read AND written through.
(2)/(3) YES, explicitly and unavoidably: the wraps' entire justification is
"defeats GCC 2.7.2's `rank_for_schedule` INSN_LUID tiebreak by moving the
compound-statement RTL boundary" (H9/H10, hypotheses.md). This is precisely
what test #3 flags — and it is precisely why do-while(0) is NOT treated as
ordinary C under Ruling 1(3)'s rename test (a wrap has no truthful semantic
reading distinguishing it from the unwrapped form) and instead requires its
OWN named, owner-sanctioned family with a mandatory FAKE annotation
([[do-while-zero-exception]]). Naming the mechanism here is not a defect —
it is the rule's own required content for the annotation.

## T4 permuter/search provenance
YES for constructs (2) and (3) — both were found by a directed
decomp-permuter campaign (tmp/grind/_exeque/s4/perm_ws/), not hand-derived
first. Per [[no-new-park-categories]]'s "Auto-search tools — output is
PROPOSALS, not winners" section, permuter provenance alone does not
disqualify a construct; each find was vetted against the cheat catalog
before being applied to src/display.c (see the two REJECTED permuter finds
below — volatile coercion and a behavior-changing statement move — which
WERE disqualified and never applied). The surviving two are vetted as
falling inside the ALREADY-sanctioned do-while(0) family, not treated as
self-justifying because a search tool found them.

## T5 family check
(1) pointer-alias family lineage, treated as ordinary C under
[[ordinary-c-judge-decidable]] Ruling 1(3) (real semantic reading, read AND
written) — same position as s2/s3 banked this construct under, unchanged.
(2)/(3) do-while(0) wrap family — [[do-while-zero-exception]], the ONE
sanctioned no-semantic-purpose wrapper, sanctioned for ANY codegen effect
including register/scheduling per the 2026-07-06 owner ruling. The nested
form (3) additionally requires (per that rule's prerequisite 3) a
documented "single-level-insufficient" justification, which this session
measured directly: the single-level wrap alone (construct 2 applied without
3) leaves the floor at 7; adding the nested wrap (3) drops it to 2 — an A/B
measurement, not an assertion. No other construct in the diff matches any
forbidden family: no register pins, no `__asm__`, no dead stores/discards,
no unused locals, no volatile coercion on D_8009BF6C/D_8009BF70 (the
permuter's volatile-coercion proposals were identified and rejected, not
applied — see below).

## T6 naming-announces-intent test
No construct is named `pad`, `dummy`, `unused`, `spill`, `_buf`, `tail`,
`slack`, or similar. `p` is a plain minimal pointer name (unchanged from
s2). The do-while wraps introduce no new named variable at all — they are
pure control-structure wraps around existing real statements, which is
exactly the sanctioned shape (contrast: a NAMED constant-holder or dead
local would need the naming test; a bare `do {} while(0);` wrap has nothing
to name).

## Rejected permuter proposals (NOT applied to src, recorded for the ledger)
Campaign 1 also surfaced `extern volatile int/short/char D_8009BF6C`/
`D_8009BF70` (output-715/765/915-*) — REJECTED: these are plain debug-record
game-state scalars with no identifiable IRQ writer independent of
`_exeque`'s own queue-drain loop and no catalogued
[[legitimate-volatile-interrupt-touched]] use-site shape; volatile coercion
on them also contradicts this ledger's s1-banked evidence
(`volatile_extern_allowlist.txt:75`, the D_8009BF68 grant, explicitly names
D_8009BF6C/D_8009BF70 as the non-volatile siblings). And `output-905-1`
(moving `SetIntrMask(D_8009BF84)` inside the loop) — REJECTED as not even a
valid candidate: it changes runtime behavior (re-arms the interrupt mask
every loop iteration instead of once after the loop), which the permuter's
byte-score search does not itself verify. Neither was applied to
src/display.c at any point this session.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap (single-level, construct 2)
  SCOPE: "sanctioned for ANY codegen effect, including register allocation
  (owner ruling 2026-07-06, which SUPERSEDES the 2026-06-04
  reorg.c/LABEL_OUTSIDE_LOOP_P-only scoping; that scoping is abolished and
  must not be cited as a FAIL ground). Match device, not a first resort —
  prefer natural geometry, but exhaustion is not a hard gate for
  single-level wraps (rule prerequisite 2); mandatory `/* FAKE: ... */`
  annotation naming the observed effect."
  PRECEDENT: .claude/rules/no-new-park-categories.md:265

  FAMILY: do-while(0) wrap (nested, construct 3)
  SCOPE: "nested wraps need a written single-level-insufficient
  justification (rule prerequisite 3)."
  PRECEDENT: .claude/rules/no-new-park-categories.md:272 (same entry,
  nested-wrap prerequisite clause); justification measurement is this
  session's H9-vs-H10 A/B (floor 7 with single-level alone, floor 2 with
  the nested nesting added), recorded in hypotheses.md and
  evidence.md [s4].

  FAMILY: pointer-alias-to-global (construct 1, carried from s2, treated
  as ordinary C per Ruling 1(3), not claimed under the FAKE-gated family)
  SCOPE: "a construct with a real semantic reading ... is NEVER a cheat
  merely because the agent chose it after observing the scheduler --
  'scheduling-motivated respelling' is not a FAIL ground when the spelling
  is semantically truthful."
  PRECEDENT: .claude/rules/ordinary-c-judge-decidable.md:55 (Ruling 1,
  point 3)

ANNOTATION-CONFORMANCE: the two `/* FAKE: ... */` lines actually emitted in
src/display.c this session (both carried into memory/grind/_exeque/candidate.c
and applied to the working tree during measurement, then reverted per
[[asm-until-matched]] since this session did not reach sandbox 0):

  /* FAKE: do-while(0) wrap, mechanism: reorg.c list-scheduler
   * ordering of the post-call debug-record triple-store
   * (D_8009BF68[0]/D_8009BF6C/D_8009BF70), lever-exhaustion:
   * memory/grind/_exeque/hypotheses.md H7/H8 (five independent
   * statement/variable respellings of this block measured
   * byte-identical, s2-s3) */

  /* FAKE: NESTED do-while(0) wrap, mechanism: same
   * reorg.c/rank_for_schedule ordering as the outer wrap, on
   * a narrower sub-block; lever-exhaustion: single-level
   * wrap around the whole triple-store (this file's outer
   * do-while) measurably left floor at 7/187 -- nesting a
   * second level around just the first two stores measured
   * floor 2/187 on the identical surrounding chassis
   * (memory/grind/_exeque/evidence.md [s4] campaign 1 vs 2),
   * satisfying do-while-zero-exception's
   * single-level-insufficient prerequisite for nested wraps */

Both carry what (do-while(0) wrap, single/nested), mechanism (named GCC
pass: reorg.c list scheduler / rank_for_schedule RTL-boundary ordering),
and lever-exhaustion (pointers into hypotheses.md H7/H8 for the outer wrap;
the direct H9-vs-H10 A/B floor measurement for the nested wrap).

NOTE: this session did NOT reach sandbox 0 (floor is 2/187, down from
12/187) — this outcome is `progress`, not `candidate-ready`. This self-vet
is written proactively so the next session (or a future candidate-ready
session building on this chassis) inherits a vetted starting point rather
than re-deriving the do-while(0) analysis from scratch.
