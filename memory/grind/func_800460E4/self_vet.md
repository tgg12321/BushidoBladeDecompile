# SELF-VET — func_800460E4  (permuter-modality session, 2026-08-25, re-measured chassis)

Measured state: `sandbox func_800460E4 --disable all` = **0** (248/248 insns,
rules_dropped=10, cheat_asm_stripped=0), measured THIS session with this exact body in
src/text1a_c2.c (HEAD's rule-era body measures 35 at the identical chassis).

CONSTRUCTS: (1) declaration hoist - four scratch offset variables and a0_ptr are declared at the top of the routine instead of in inner braces, each keeping its original job (measured byte-neutral); (2) the stage-3 arm stages two stage-record values through the pre-existing scratch offsets off_a and off_b, refining each in place, every stage consumed by the next statement (FAKE-annotated; staged-value-reused-variable); (3) the early-switch off_b site is spelled with that in-place two-step refinement too (FAKE-annotated); (4) INHERITED unchanged from the body that passed layer-1 at 03:53 - the FAKE combine-foldable chain-extender on s1's init; (5) INHERITED unchanged - the scaled-index spelling of the stage-record pointer and the dropped p/p2 param-alias locals.

## T1 semantic purpose
(1) None beyond style, and none is claimed: the object is byte-identical with and without
the hoist (score 0 both ways). No variable is created or destroyed by it — the four offset
temps and a0_ptr all exist in HEAD's committed body with the same jobs.
(2) The borrow computes exactly the values the one-expression spelling computes; program
behaviour is identical. Its observable effect is on instruction order and register seats.
DISCLOSED AS SUCH — that is precisely the property the staged-value-reused-variable family
exists to sanction ("one line of C written as two"); the family's own worked example and
the func_800200DC PASS have the same property.
(3) Same class as (2): the two-step refinement computes the byte offset the flat ALIGN4
expression computes.
(4)/(5) Unchanged from the body layer-1 already ruled legitimate at 03:53; (4) folds to
zero bytes and is a live init of a value the function really uses, (5) is ordinary C.

## T2 human-programmer
(1) YES. Declaring every local at the top of the routine is C89 / PsyQ house style, and the
inner-brace declarations it replaces are decomp-era artifacts of the rule-era chassis, not
evidence of the original shape (the dispatch brief's RULE-ERA CHASSIS warning says so
explicitly for this function). I do note the honest tension: widening those temps' scope is
what makes them borrowable at all, so a reviewer should weigh (1) and (2) together rather
than separately — I am not hiding that behind byte-neutrality.
(2)/(3) A 1998 programmer reusing a scratch offset variable that is finished with, and
refining an offset in place before using it, is ordinary period C; SOTN's matched sources
ship this shape (`j = menu->unk1D; // FAKE?`). A reader asks "why reuse off_a here?" and the
truthful answer is "it is a free scratch offset". I do NOT claim the reuse was inevitable —
the licence comes from the sanctioned family, not from a naturalness claim.
(4)/(5) Ruled on at 03:53.

## T3 GCC-internals justification
YES for (2) and (3), stated openly rather than dressed up. Mechanism: GCC 2.7.2 sched.c
`adjust_priority` -> `birthing_insn_p`, which lifts a newly-ready insn to max_priority only
when `reg_n_sets[regno] == 1`; borrowing variables that are already written elsewhere clears
that boost on both stage-record loads and dissolves the block-19 scheduler tie (dump-read at
the floor-9 chassis, hypotheses.md H20,
tmp/grind/func_800460E4/dumps/text1a_c2.sched:368-407). For (3) the secondary mechanism is
local-alloc.c quantity tying (the refinement writes back into the same pseudo, so both
shifts take the carrier's own hard register). A GCC-internals mechanism is a cheat SIGNAL in
general. It is not disqualifying HERE only because it is the mechanism the owner NAMED when
sanctioning this exact family (.claude/rules/staged-value-reused-variable.md "Origin":
"Mechanism reference: GCC 2.7.2 sched.c `adjust_priority` -> `birthing_insn_p` (the
'assigned once?' check is literally `reg_n_sets[regno] == 1`)"), and because the construct
sits inside that family's six bounds (worked bound-by-bound below).
For (1) there is no GCC-internals justification at all — it is byte-neutral.

## T4 permuter/search provenance
This session's mandated modality is permuter. The closing form did NOT come from random
search and I do not claim it did: the prior permuter session ran campaign perm_e (label
s5-b1-staged-s6s4-246, base score 435, 16658 iterations, best new find 80, NO zero,
harvested and stopped in-session) and it died — the form came from the directed carrier
sweep tmp/grind/func_800460E4/s4/s5a.py .. s5g.py (variant j4), which enumerated which
PRE-EXISTING variable is borrowed. That is the route the 2026-08-25 07:19 ruling itself left
open ("the follow-on PASS closed at 0 only after moving to PRE-EXISTING carriers",
docs/grind/decisions.md:10730). This session re-derived nothing by search: it re-applied the
banked form, re-measured it at the current chassis (0), verified every citation resolves,
and rewrote this vet. No construct here passes detectors by spelling — each mechanism is
named in its own annotation.

## T5 family check
(1) No family needed — ordinary C, measured byte-neutral.
(2) and (3): staged-value-reused-variable (owner ruling 2026-07-03), all six bounds met;
scope quoted and bounds worked below.
(4) F1 combine-foldable chain-extender (owner ruling 2026-07-01), unchanged from the body
that passed layer-1 at 03:53.
Forbidden-family sweep, explicit and complete: there are no register pins, no `$N` asm, no
inline asm of any kind, no scheduling barrier, no `volatile` anywhere, no alias rename, no
dead local or dead array, no dead-param-assign, no dead-conditional-store, no `if (1)`
wrapper, no goto pad, no DImode chain, no aggregate merge of the two s16 globals, no
integer-cast byte-offset deref, no width cast, no build-pipeline or linker-script edit. I
re-read all six state.json banned entries against the diff line by line; every one of them
is absent, and the two stage-record loads in the stage-3 arm are spelled with the ordinary
`s0[s3 - 2]` / `s0[s3 - 1]` index this routine already uses elsewhere, so the address-form
objections have no purchase on this body either.
BOUNDARY I AM NOT CROSSING: the 07:19 ruling refused a FRESH twice-written carrier. Both
borrowed variables pre-exist in HEAD's committed body with their own real jobs (off_a holds
the mainline stage-block offset `s0[1]`; off_b holds the early switch's sub-block offset),
which is exactly the distinction that ruling drew.

## T6 naming-announces-intent
No `pad` / `dummy` / `unused` / `spill` / `tail` / `slack` / `_buf` anywhere. `off_a`..`off_d`
name what they hold (byte offsets into the stage record) and every one of them is read;
`a0_ptr` is the pre-existing name for the stage-record pointer. No name announces a codegen
purpose.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable (constructs 2 and 3)
  SCOPE: "**The variable already exists for a real job.** You may only borrow a variable the function genuinely uses elsewhere (a loop counter, a status flag, a poll result). Inventing a new variable just to have something to borrow is NOT this rule (a fresh named intermediate is fine C on its own merits and needs no exception — but then it also won't have the "assigned more than once" property this trick needs)."
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:64
  PRECEDENT: docs/grind/decisions.md:1844
  PRECEDENT: docs/reference/sotn-construct-index.md:70
  Bound-by-bound:
  (1) value real and used — every staged assignment is consumed by the next statement
  (off_a -> s6, off_b -> s4, off_b -> func_8003EDC0); zero dead code, zero dead stores.
  (2) variable already exists for a real job — off_a holds the mainline stage-block offset
  (`off_a = s0[1]` -> s6) and off_b holds the early switch's sub-block offset
  (`func_8003EDC0(PTR_OFF(s0, off_b), 7)`); neither is invented for the borrow. Disclosed
  qualification: both are hoisted from inner braces to the top of the routine (construct 1),
  which is what puts them in scope at the borrow site.
  (3) borrow provably safe — at the stage-3 arm off_a's mainline value has already been
  consumed into s6 and is never read again; off_b's early-switch value is dead there and on
  the path that reaches the stage-3 arm it was never written at all (stage_id 3 takes the
  `break` arm above); neither is read afterwards.
  (4) annotated — see ANNOTATION-CONFORMANCE.
  (5) last resort with receipts — five prior sessions of ladder work recorded in
  hypotheses.md (H8, H15-H21: statement order, declaration order, whole-function shape,
  pre-sched stream order, 22 once-written spellings, every dependence-graph route) all
  measured dead, and this session re-measured the resulting form rather than reopening them.
  (6) everything else applies — no dead stores, no unused variables, no pins, no asm, no
  volatile, no build-time edits.

  FAMILY: F1 combine-foldable chain-extender (construct 4, INHERITED unchanged)
  SCOPE: "The combine-foldable chain-extender to bump `reg_n_refs` was moved to the sanctioned F1 family by owner ruling 2026-07-01 — FAKE-annotated last-resort; un-annotated or byte-materializing instances remain FAIL."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:32
  PRECEDENT: docs/grind/decisions.md:10710

ANNOTATION-CONFORMANCE:
  /* FAKE: live default init of s1 routed through a delta-rebase detour that combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare priority above the s2 pointer so allocation order matches target, lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */
  /* FAKE: the sub-block byte offset is staged through the scratch offset off_b in two steps (word index, then aligned offset) rather than one ALIGN4 expression; every stage is a real value consumed by the next statement, mechanism: GCC 2.7.2 local-alloc.c quantity tying - the refinement writes back into the same pseudo, so both shifts take the carrier's own hard register ($a0) instead of a fresh one, which is also what makes off_b's pseudo multiply-set for sched.c adjust_priority -> birthing_insn_p (reg_n_sets[regno] == 1) at its case-3 borrow, lever-exhaustion: memory/grind/func_800460E4/hypotheses.md H8/H15/H17/H19/H20 + evidence.md [s1]-[s5] */
  /* FAKE: the two stage-header words are staged through the function's existing scratch offsets off_a (whose mainline value s0[1] is dead - already consumed into s6 above) and off_b (whose early-switch value is dead, and is not even written on the path that reaches here), each refined in place to an aligned offset and consumed on the next statement, mechanism: GCC 2.7.2 sched.c adjust_priority -> birthing_insn_p boosts a newly-ready insn only when reg_n_sets[regno] == 1; borrowing these multiply-set locals clears the boost on both header loads, so only ONE boosted insn is ready at block 19's reverse-cycle T-6, the tie that produced the 9-instruction residual disappears and both loads issue adjacently as in target, lever-exhaustion: memory/grind/func_800460E4/hypotheses.md H8/H15-H21 + evidence.md [s1]-[s5] (order, declaration order, whole-function shape, stream order and 22 once-written spellings all measured dead) */
  All three annotations are present verbatim at their construct sites in src/text1a_c2.c and
  each carries what + mechanism + lever-exhaustion.
