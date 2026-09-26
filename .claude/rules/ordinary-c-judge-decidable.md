---
name: ordinary-c-judge-decidable
paths: [".claude/rules/*.md", "tools/grinder/**", "engine/queue.py", "docs/grind/*.md", "src/*.c"]
description: "Owner ruling 2026-08-31: agent escalations to the owner are retired. Ordinary-C candidates are Judge-decidable against the frozen construct-class list (non-membership = FAIL, never a packet); dead-store deadness is store-level; named-intermediate relaxed to multi-read (SOTN new_var_temp); exhaustion dispositions become the silent `foreclosed` queue state. The anti-cheat wall (no non-C mechanisms, by any spelling) is unchanged."
metadata:
  type: rule
---

# Ordinary C is Judge-decidable — escalations to the owner are retired (owner ruling 2026-08-31)

Owner (Trenton), verbatim, 2026-08-31:

> "My primary concern is agents slipping through new 'cheats' by another
> name, like what historically happened with regfix and asmfix. But if what
> the agent is trying to do is SOTN standard, and not a cheat, i dont want
> them escalating it to me either."

Approved as three rulings ("Okay go ahead", same conversation, after the
operator's proposal was laid out in full).

## The category distinction this ruling rests on

**The regfix/asmfix cheat class was out-of-band mechanisms** — register
pins, asm patches, build-time output rewriting: things living OUTSIDE the
committed C that forced bytes the C didn't produce. That class is dead by
construction (cheat-invisible sandbox, detectors, default-FAIL Judge on
anything asm-shaped, no third byte source) and NOTHING in this ruling
touches that wall.

**Ordinary compilable C is a different category.** A `sandbox --disable
all` score of 0 proves the candidate is a genuine preimage of the original
bytes under the original compiler. Choosing among semantically-truthful C
spellings by observing codegen is the METHOD of matching decompilation,
not a cheat signal. The SOTN norm is exactly this: the bytes decide; the
committed C is a preimage; naturalness is aspirational, not gating.

## Ruling 1 — ordinary-C candidates are Judge-decidable, never escalated

A candidate that is 100% ordinary compilable C (zero asm, pins, pragmas,
gate-list or build changes — mechanically enforced, unchanged) is decided
by the pipeline under a mechanical checklist:

1. **Zero non-C mechanisms** (sandbox + detectors, unchanged).
2. **Construct-class membership**: every no-semantic-purpose construct in
   the candidate sits inside the frozen SOTN-accepted list
   ([[no-new-park-categories]] § SOTN-accepted, as amended by this ruling)
   with that entry's own prerequisites met. The list stays OWNER-ONLY to
   extend — but **non-membership is now a clean FAIL(CONSTRUCT) plus a
   borderline-ledger entry, never an ESCALATE**. The Judge's
   `family-extension` and `policy-question` escalate kinds are retired.
3. **The rename test replaces motive-testing.** Judge and reviewers rule
   on the C TEXT: does each construct have a truthful semantic reading, do
   neutral names survive, is the mandated annotation present? A construct
   with a real semantic reading (e.g. typing a 16-byte GTE scratchpad slot
   as `VECTOR` when the SDK macros consume it as one) is NEVER a cheat
   merely because the agent chose it after observing the scheduler —
   "scheduling-motivated respelling" is not a FAIL ground when the
   spelling is semantically truthful. Constructs with NO semantic reading
   remain governed by their family entries (annotation, exhaustion,
   review), unchanged.
4. **Simplest-known-form**: when multiple byte-exact forms are known, the
   one with the fewest no-semantic-purpose constructs lands.

**Class amendment ratified by this ruling (the SOTN `new_var_temp`
class):** the named-intermediate entry's 2026-08-17 prong (1) is relaxed
from "once-written, once-read" to **"once-written"** — a fresh local
holding a real, consumed value may be read any number of times. Evidence:
SOTN-master PSX `new_var_temp` declarations
(`docs/reference/sotn-construct-index.md:649` — `src/dra/cd.c:520-522`,
`src/dra/42398.c:254-259`, `src/dra/menu.c:1956`, `src/dra/5087C.c:213`,
`src/st/rnz0/e_fire_demon.c:494`, `libsnd/vmanager.c` ×6).

**Evidence caveat, carried verbatim from the record and PRESENTED TO THE
OWNER before approval** (decisions.md:16536-16539): "the index carries
declaration lines only. Whether those temporaries are written once or
several times is NOT verifiable from this repo, so this is precedent for
the existence of fresh RA-purposed locals in SOTN PSX master, not a
demonstration that their shape matches ours line-for-line." The operator's
proposal to the owner restated it in terms: "the SOTN new_var_temp
citations prove fresh RA-purposed locals exist in SOTN master, but not
that their shape matches — so this is a judgment call that's genuinely
yours, once, as a class ruling." The owner ruled WITH that caveat in hand;
this relaxation is an owner judgment call on flagged-as-partial evidence,
not a claim the evidence is line-for-line conclusive.

**Multi-WRITE carriers remain banned** (the y1 FAIL, decisions.md:1833,
and the 2026-08-30 21:30 `c` FAIL both stand; a fresh local written more
than once is admitted ONLY if it meets every prong of Ruling 5
(2026-09-23; as amended by its 2026-09-23 extension), of Ruling 6
(2026-09-23), of Ruling 8 (2026-09-24, vmNoiseOn's `temp` only), of Ruling 9
(2026-09-25) or of Ruling 10 (2026-09-25), or, when none of those admits
it, every prong of Ruling 11 (2026-09-26, allocator-necessity proof),
whichever governs that variable, exclusively: the reused
variable itself may not also claim this entry or
[[staged-value-reused-variable]]; other locals in the same body,
including a Ruling 5 1(b)(ii) selector binding, are judged under their
own entries.) All other prongs (real value, byte-neutral, fresh not
borrowed, destination not live-pre-initialized, standard prerequisites)
are unchanged. Where Rulings 5-10 below say that a variable they do not
admit "fails", that is read subject to Ruling 11: such a variable is
admitted if, and only if, it meets every prong of Ruling 11.

## Ruling 2 — dead-store deadness is STORE-level

In [[dead-store-fake-exception]], the scope sentence governs and the
bullet is amended to match: a store whose STORED VALUE is never read is in
scope **even if the destination variable is later re-assigned and read**
(the defensive-init pattern: `x = a; ... x = b;` with every read after
the second store). That is what GCC's DCE actually deletes and what the
rule's own scope sentence always said. All other prerequisites of that
rule (lever exhaustion, named mechanism, `/* FAKE */` annotation, dual
review) are unchanged.

**Informed-approval record.** This resolution was not presented to the
owner as a neutral textual footnote: the operator's proposal stated in
terms that the scope sentence and bullet "disagree on exactly this case
(a store that's dead but to a variable that is later read)", that the
contradiction was surfaced by the func_80045878 packet, and that under
Rulings 1+2 that function's candidate could close — and the owner
approved knowing that consequence. **The clarification does not by itself
legalize any previously-FAILed instance**: every candidate, including
func_80045878's, is adjudicated fresh by layer-1 + the default-FAIL Judge
against the amended text with all prerequisites verified.

## Ruling 3 — exhaustion dispositions are silent: the `foreclosed` state

> **SUPERSEDED (owner ruling 2026-09-08, [[rotation-not-foreclosure]]):** the
> `foreclosed` status is retired. The disposition is now `rotated` (back of
> the active worklist, automatic return on queue drain / toolchain change /
> sibling movement); the cc1psx self-disproof runs before exhaustion may be
> declared, and every ladder instrument must have run in the flat window
> first. The silent-record principle (nothing surfaced to the owner, no
> packet) is unchanged.

The `escalated` queue status and session-filed OWNER-ESCALATION decision
packets are **retired**. A function whose frontier is empty with both
endgame-lock gates failing (the 2026-07-27 standing-ruling shape) takes
the **`foreclosed`** status:

- The driver records the disposition to `docs/grind/decisions.md` +
  `docs/grind/journal.md` (proof-of-foreclosure, evidence pointers) and
  marks the queue item `foreclosed`. **Nothing is surfaced to the owner;
  no packet is filed; no question is asked.**
- `queue next` skips foreclosed items. The function stays
  `INCLUDE_ASM` on main — this path can only ever refuse C, never accept
  it, so it carries zero cheat risk.
- **Re-activation triggers**: a new owner class grant covering the
  function's residual, a toolchain-fidelity finding, or an explicit owner
  `queue unpark`. Foreclosure is a disposition with triggers, never a
  pending decision.

Mechanical questions the pipeline can execute itself (integration
handoffs, scope grants, stale-ban clearance, canonical-asm grants with
qualifying evidence) keep their existing driver-executed paths
([[integration-handoff-self-serve]], [[judge-sole-gate]] rule 3) — those
were never owner-waits and remain the ONLY two Judge ESCALATE kinds.

## Migration (2026-08-31, executed with this ruling)

The 18 `escalated` items resolve as follows:

- **Returned to active**: `func_80045878`, `func_800324D0` (mechanical
  integration question — driver path), `func_80062020` (the owner answers
  its packet's question YES: ruling 6a supersedes the stale
  `banned_constructs` entries; the bans are cleared and the ordered
  adjudication proceeds on the merits). **Nothing about any function's
  outcome is pre-decided by this document**: each returned item's
  candidate is adjudicated fresh — layer-1 + default-FAIL Judge, on the
  merits, against the amended rule text — exactly like any other
  candidate. In particular, whether func_80045878's s13b form satisfies
  the amended prongs and every dead-store prerequisite is the Judge's
  call, not this ruling's.
- **Foreclosed**: the remaining 15 (exhaustion dispositions under the
  2026-07-27 standing ruling; each item's reason points at its latest
  decisions.md entry).

## Ruling 4 (owner, 2026-09-02) — compound-assignment splits are ordinary C

Splitting one assignment into consecutive compound assignments on the SAME
variable — `ratio *= 0x103B; ratio >>= 12;` for `ratio = (ratio * 0x103B) >> 12;`,
or `v = a; v += b;` for `v = a + b;` — is a semantically-truthful spelling under
Ruling 1(3), provided no statement is dead, no annotation is needed and no pad
is introduced. It is NOT a construct requiring a family entry, and a reviewer
may not FAIL it on the ground that the agent chose it after observing register
allocation. This supersedes the provisional caveat in the 2026-06-13
split-init-accumulation directive ("adjacent spellings need their own ruling").
Multi-WRITE carriers whose extra write is dead remain banned (unchanged).
Record: docs/grind/decisions.md 2026-09-02 OWNER RULING, Ruling B
(_spu_2pitch 10eadce5 stands as COMPLETED-C).

## Ruling 5 (owner, 2026-09-23) — one role repeated per block: a reused local

Owner (Trenton), verbatim: "If this is a genuine C situation that makes sense
to add, we can add it. I dont want to slip and allow a cheat or workaround
though." Then, after the proposal was laid out: "I approve for now assuming
this is not a cheat".

A FRESH local written more than once is admitted ONLY if it meets EVERY prong
below (a variable meeting every prong of Ruling 6 is judged under Ruling 6
instead, vmNoiseOn's `temp` meeting every prong of Ruling 8 is judged
under Ruling 8 instead, and a variable meeting every prong of Ruling 9 or of
Ruling 10 is judged under that ruling instead). This ruling governs that case exclusively: the reused variable itself
may not claim the named-intermediate relaxation (Ruling 1; its reads are
governed by this ruling's prong 1(c) alone) or [[staged-value-reused-variable]], and no consumer
of it may be a staged-value borrow. Other locals in the body, including a
1(b)(ii) selector binding, are judged under their own entries. No FAKE
annotation is required for the reuse itself; any other family the body relies
on (duplication, dead-store, etc.) keeps its own annotation requirement.
Failing any prong is a FAIL(CONSTRUCT) under Ruling 1, exactly as before.

1. **One consumer role, one template.**
   (a) Every write feeds the SAME consumer: the same struct member of the
       same object, or the same argument slot of the same callee, never
       another local.
   (b) The write statements are textually identical except for the
       SELECTOR, the ONLY permitted textual difference, which identifies
       which sibling record the block processes. The selector difference is
       confined to the subscript of ONE AND THE SAME base expression (e.g.
       `ctx[0]` / `ctx[1]`), where each subscript is an integer constant;
       only one subscript position may differ between blocks, and
       different blocks may use the same subscript (e.g. func_8006F528
       uses `ctx[0]` in both its first and third blocks); the base
       expression is a declared array or pointer variable, used without any
       cast, whose elements are interchangeable instances of one role:
       sibling records, or pointers/addresses to sibling records, that the
       function uses in the same way (e.g. func_8006F528's `ctx[0..2]`, each
       used as a record base at the same +0xC offset). The base variable
       must not be declared, initialized or assigned from a cast of the
       ADDRESS of a struct, scalar or field (`(T *)&obj`, `(T *)&obj.f`,
       `(T *)obj.arr` where `arr` is an array member, `(T *)G` where G is not
       itself an array of those records). Loading a pointer VALUE stored in
       memory is not such a cast (e.g. func_8006F528's
       `ctx = *(s32 **)(D_800A35A8 + 0x5C);` reads the pointer held at
       +0x5C). A loaded base, like any base, qualifies only if the object it
       points to is an array of interchangeable sibling records; the
       different-roles and distinct-named-fields clauses below apply to it
       unchanged. It also
       must not be an array/pointer declaration over an object whose slots
       the function uses in different roles (e.g. a parameter `s32 *arg0`
       whose [1], [5], [7], [8] hold a return chain, a prim cursor and OT
       pointers). Either is a pun by declaration and is never a selector,
       whether or not the fields have names. A cast or pointer-pun base at
       the use site (e.g. `((u16 *)G)[N]`, `((s16 *)&d)[N]`), or subscripts
       that reach distinct named fields of one struct, are likewise never a
       selector. It
       appears either (i) directly in the write, or (ii) in a block-local
       variable declared in each block and written exactly once there, whose
       binding statements are textually identical except for that subscript
       (e.g. `s32 base = ctx[0];` / `s32 base = ctx[1];`). Two different
       named variables, parameters, globals or fields are never a selector,
       even when they have the same type or point at records of the same
       type. The consumer statements are textually identical across the
       blocks, with NO selector and no other difference (e.g. `s.p1 = p1;`
       in every block); a consumer whose destination differs by subscript,
       member, or object fails 1(a).
   (c) Each write is read exactly once, by the role consumer, in the same
       block as the write. A variable with any other reader fails.
   (d) The writes sit in distinct sibling blocks (or the arms of one
       conditional), each ending in the consumer (statements other than the
       write, its selector binding and the consumer may differ between
       blocks): repetitions of one piece of
       code, the kind a programmer could have written as a loop or macro body.
   (e) **Real computation.** Every write's right-hand side is a load or an
       arithmetic computation whose instructions appear in the target's own
       bytes. A write whose value is a literal constant, or a bare copy of
       another named variable or parameter, fails. Constant-holders stay under
       [[named-local-fake-exception]] (FAKE required), and the F1
       constant-staging chain stays refused.
   (f) The name names the consumer role (`p1`, because it is stored to
       `s.p1`). Generic names (`tmp`, `t`, `temp`, `val`, `v`, `ptr`, `p`,
       `new_var`, register-style names) never satisfy this prong. For a
       call-argument consumer, the name states what the argument means (e.g.
       `ot`, `rect`, `count`); single-letter names and `arg`/`argN`/`aN`
       never satisfy this prong.
   Writes feeding DIFFERENT consumers fail even when their expressions look
   alike: y1 fed `dx` and then `dy`, so y1 fails.
2. **Nothing added, nothing reloaded.**
   (a) Every write is used before the next write.
   (b) The reuse spelling and the one-local-per-write spelling measured
       under prong 4 have the SAME statement list: they differ only in
       declarations and identifiers. A write or consumer that exists only in
       the reuse form fails.
   (c) No write stores a value the variable already holds, judged by C
       semantics: an intervening store is ignored only when it provably
       targets a distinct object (the same base with a non-overlapping
       constant offset). Re-loading the same lvalue with no intervening write
       to that lvalue fails (func_80060A68 `src`/`idx`).
       **Clarification (owner, 2026-09-26, fifth batch): "already holds"
       means on EVERY feasible path.** The question put to the owner, verbatim: "The
       reused-variable rules forbid 'a write that stores a value the
       variable already holds'. In func_8002DE20 two writes re-store the
       same value on ONE of the paths reaching them, but on the other paths
       the variable holds something else, so the write is needed there and
       removing it breaks the program. Does that count as the banned
       redundant write?" Owner (Trenton) chose, verbatim: **"No — only if
       redundant on all paths (Recommended)"**, whose text is: "A write is
       banned only when it's removable: the variable already holds that
       value on EVERY path reaching it. A write needed on some path is
       allowed." (Record: docs/grind/owner-rulings-2026-09-26.md, batch 5.)
       The author's narrowing. A feasible incoming path is a path reaching
       the write whose branch conditions are jointly satisfiable under C
       semantics; a path whose conditions contradict each other is not
       feasible and never counts. A write fails this prong only when,
       judged by C semantics as above, the variable already holds the
       written value on EVERY feasible incoming path, so that deleting the
       write leaves the program's behaviour unchanged. A write that stores
       a value the variable holds on some feasible incoming paths but not on
       at least one other is not a re-store under this prong. It is audited
       mechanically: for each write that re-stores a held value on any
       feasible incoming path, the ledger names at least one feasible
       incoming path (by its branch conditions) on which the variable holds
       a different value at the write, and states the value it holds there.
       A write without that record fails. Nothing else in this prong
       changes: the intervening-store rule (ignored only when it provably
       targets a distinct object) stands, and so does the re-load ban,
       which is this same test applied to a load: a re-load of the same
       lvalue into the variable fails only when, on EVERY feasible incoming
       path, the variable was loaded from that lvalue and neither the lvalue
       nor the variable has been written since (func_80060A68 `src`/`idx`).
       A re-load that is admitted because this does not hold on some path
       carries the same ledger record: a feasible incoming path, named by
       its branch conditions, on which the lvalue or the variable has been
       written since the last load, stating that write. Every
       rule that applies this prong by reference (the Ruling 5 extension's
       (D), whose additional member re-assignment requirement is unchanged,
       Ruling 9 (g), Ruling 11 (B)(2)) reads it with this clarification. Record: docs/grind/decisions.md 2026-09-26 OWNER
       RULING — path-wise re-store.
   (d) No single computation is split across writes (Ruling 4 governs
       same-statement splits separately).
3. **Not a borrow.** The variable has no other job, and no declaration OTHER
   THAN the variable's own is moved or re-scoped. The variable is declared
   once, at the innermost scope that encloses all of its writes. Borrowing
   stays under [[staged-value-reused-variable]] and all of its bounds.
4. **Receipts.** The one-local-per-write spelling is measured and recorded
   as failing, AND the function's ledger shows the ordinary ladder was run
   (structural respellings, the permuter from the carrier-free candidate,
   an allocation dump) before the reuse form was adopted. Normal review
   (layer-1/Judge, or layer-2 on the manual path) applies.

**What stays banned, and why each still fails:** func_800200DC `y1`
(decisions.md 2026-07-28 01:47: fed `dx` then `dy`, fails prong 1(a),
including respelled with a punned array consumer, 1(b));
func_80045878 `c` (2026-08-30 21:30: `s1[3]`, then `a0 + 3`, then `0x8000`:
different templates, fails 1(b); the `c = 0x8000` constant write also fails
1(e)); func_80060A68 `src`/`idx`/`cp` (2026-08-19: a fresh local re-loaded
with the same unchanged pointer, or given a folded extra write, fails prong
2(b)/(c)); func_800460E4 off_a/off_b (2026-08-25: a declaration-hoist borrow
and a one-statement split, fails prongs 2(d) and 3); func_8002D780 `tmp`
(2026-09-15 23:16: held `z2 - z0` and then a `*LUT` value, fails 1(a)/(b)/(c));
any constant-holder spelled as a reused local (`mode = 0; ... mode = 1;`),
which fails 1(e); a cast-laundered base (`T *v = (T *)&obj; ... v[0] ...
v[1]`) or struct-as-`s32 *` slots used as a selector, which fail 1(b). This ruling reopens none of them.

**Known weakness, presented to the owner:** in func_8006F528 the author also
chose which variables to make block-scoped (`base`) and which to make
function-scoped (`p1`) by measuring. The reuse and per-block spellings
compile the same statements. The only codegen effect of the function-scope
declaration is that one pseudo spans several blocks, the same allocator
effect the banned y1/`c`/`src` carriers relied on. This ruling admits that
effect only when the reuse also reads as one role repeated per prongs 1-2.
Allocator effect alone is never sufficient. Record: docs/grind/decisions.md
2026-09-23 OWNER RULING.

### Ruling 5 extension (owner, 2026-09-23) — identical writes, record picked beforehand

Owner (Trenton), answering the func_8006B120 question, selected verbatim:
"Yes, extend Ruling 5" — described to the owner as "Record a narrow addition:
allowed when the reassignment line is word-for-word identical at every site
and each site feeds the same field. The record can be chosen by a loop
counter, and one site can be at function level. Then land func_8006B120 as
COMPLETED-C after a fresh reviewer pass."

This extension replaces ONLY prongs 1(b) and 1(d), and only for a variable
meeting ALL of (A)-(D) below. Every other prong of Ruling 5 (1(a), 1(c),
1(e), 1(f), 2, 3, 4) and the exclusivity clause apply unchanged.

- **(A) No selector at all.** Every write statement of the variable is
  textually identical, character for character (e.g. `p1 = s.p0 + 0xC;` at
  every site), and every consumer statement is textually identical (e.g.
  `s.p1 = p1;`). A write that differs in any way, including by subscript,
  is judged under Ruling 5 as written, not under this extension.
- **(B) The record is picked before the write, by an ordinary member
  store.** The right-hand side reads the record through one member of the
  SAME object whose member the consumer writes (`s.p0`, consumer `s.p1`).
  Before each write, on every path reaching it, that member is assigned,
  at the same nesting level as the write, an element of one array of
  interchangeable sibling records. The member store's right-hand side is
  the array element itself, read in that statement. On every path from a
  member store to the carrier write that relies on it, the member store is
  the LAST write to that member: no statement in that stretch writes the
  member or the whole object (assignment, compound assignment, `++`/`--`,
  a struct copy, or a copy routine), or takes the address of the object or
  the member. An intervening write of any kind, even one that re-selects
  from the same array, fails (B). Every member store, at
  every site, reads its element from ONE AND THE SAME base expression (one
  declared array or pointer variable, spelled identically at every site);
  only the subscript may differ between sites. Member stores whose bases
  are different named variables, parameters, globals or fields never
  qualify, even when the objects have the same type. If the base is a
  pointer variable, it must be a local variable or parameter of this
  function whose address is never taken (no `&base` anywhere in the
  function), and no statement of the function writes it (assignment, compound
  assignment, `++`/`--`) between the first member store and the last
  carrier write. A global pointer, a static
  pointer, or a pointer held in a field of any object never qualifies as the
  base, because a callee could re-point it between sites. The subscript is either
  an integer constant or the counter of the INNERMOST loop whose body
  contains the member store, plus an optional integer constant (e.g.
  `s.p0 = tbl[i + 1];`). The counter must be a non-static local variable of this
  function whose address is never taken (no `&counter` anywhere in the
  function). Each loop whose counter is used in a subscript initializes
  the counter to an integer constant, either by a plain assignment `i = K;`
  that is the statement immediately before the loop, or by the init clause
  of that loop's `for` header (`for (i = K; ...)`). This initialization is
  required for every such loop, including the first, and it is not counted
  as an in-loop write. Apart from that initialization, the counter is
  written within the loop (its body, its condition, and the rest of a `for`
  header) by exactly one statement or expression.
  That write is an increment or decrement by a nonzero integer constant
  (`i++`, `++i`, `i--`, `--i`, `i += K`, `i -= K`) and runs exactly once on
  every path through each iteration. The counter has no other write
  anywhere in the function between the first such initialization and the
  last carrier write. A counter stepped any other way (e.g.
  `i = next[i]`, `i *= 2`, `i = f(i)`) never qualifies. It must not be written
  between the member store and the carrier write. No other subscript
  expression qualifies. The array's base is subject to every base
  condition of Ruling 5 1(b) unchanged: a declared array or pointer
  variable, no cast or pun of an address in its declaration, initialization
  or use, not an object whose slots the function uses in different roles,
  elements used by the function in the same way. The member-store statement
  must appear identically in the one-local-per-write spelling (prong 2(b));
  it is ordinary program logic, not part of the carrier.
  The member must also be read by at least one statement other than the
  carrier write (for example the object is passed to a callee, or the
  member is an argument). A member whose only reader is the carrier write
  is a staging slot and fails (B), just as a staging local does.
- **(C) Sites.** Each write is followed, in the same straight-line
  sequence, by its consumer, and the two sit in the same block (prong 1(c)).
  The sites are repetitions of one piece of code; each site sits in its own
  distinct sibling block (a loop body or conditional arm), and no block
  contains more than one site, except that at most ONE site may sit at
  function scope rather than in a sibling block, and it must precede every
  other site in the source.
- **(D) Nothing else relaxes.** The variable is fresh, has no other job,
  and is declared once at the innermost scope enclosing all its writes
  (prong 3). No write stores the value the variable already holds (prong 2(c)),
  judged by C semantics exactly as in prong 2(c); IN ADDITION, the member
  read by the right-hand side must have been re-assigned since the previous
  write, and that re-assignment must select a different array element than
  the one the previous write read (a re-assignment that can re-select the
  same element, e.g. a repeated constant subscript, fails). The per-write spelling, permuter
  and allocation dump receipts (prong 4) are required as before.

**Still banned under this extension:** everything Ruling 5 lists as still
banned. y1 (different consumers) fails (A)/1(a); `c` (different
templates) fails (A); `src`/`idx` (re-load of an unchanged value) fails (D);
a carrier whose writes are identical but whose member was not re-assigned
in between, or was re-assigned to the same element, fails (D); a record picked through a punned or multi-role
base fails (B); a record picked from different named arrays or pointers at
different sites, or through a pointer base that is reassigned between sites, is a
global/static/field pointer, or has its address taken, fails (B); a member
written again, or whose object's address is taken, between its qualifying
member store and the carrier write fails (B); a subscript whose loop counter is not initialized to an integer constant
immediately before (or in the `for` init clause of) each loop, or is stepped
by anything other than one
nonzero-constant increment/decrement per iteration (including in a loop
header or condition) fails (B); a counter that is static or
has its address taken, or a pointer base changed by `++`/`--`/compound
assignment between sites, fails (B); two or more function-scope writes fail (C).

**Known weakness (carried over from Ruling 5; NOT restated in the extension
question as recorded in decisions.md):** as with Ruling 5, the only codegen effect of reusing the
variable is one pseudo spanning all sites (func_8006B120: global.c seats it
in v1 as the target does; per-site pseudos are local-alloc'd to v0,
11/278). The extension admits that effect only for identical text feeding
one field. Record: docs/grind/decisions.md 2026-09-23 OWNER RULING —
Ruling 5 extension.

## Ruling 6 (owner, 2026-09-23) — one record pointer, one write per exclusive path

**Question as put to the owner** (manual session, plain language; the filed
form is docs/grind/borderline.md 2026-09-23 func_8001FBE8): "a programmer
declares `p`, uses it for record A in one branch, and reuses it for a
different record in another branch that never runs alongside the first.
Should that count as ordinary C, or stay banned?" The owner asked for the
author's view and was given it: not a cheat, allow it with tight limits,
"I'd close that off by allowing it only when all of these hold:"

> 1. **Same role and type in every path.** Here, a pointer to a record in one
>    table. Reusing a counter as a pointer, or a variable that meant something
>    else earlier, stays banned.
> 2. **The paths are mutually exclusive.** One returns, or they're if/else arms.
> 3. **Each path assigns it once, before reading it.** No re-staging inside a
>    path. Once per loop iteration counts as once.
> 4. **Receipts are recorded.** The split-variable version is measured, and its
>    remaining diffs are register-only.

The view also stated: "The register placement here is *evidence* of what the
original source looked like. The *justification* is that one shared `rec` is
ordinary code that would pass on its own merits, even if it changed no bytes."
It also stated that the banned func_8003FA24 shape fails conditions 2 and 3.

Owner (Trenton), verbatim: "Agree, go ahead".

**Rule text.** This is the author's narrowing of those four conditions, not the
owner's words. A fresh local written more than once that meets EVERY prong
(A)-(G) below is judged under this ruling INSTEAD of Ruling 5 and its
extension. A variable that misses any prong of (A)-(F) gets nothing from this
ruling. It is judged under Ruling 5 (as amended by its extension) and fails
unless it meets every prong there; a claim of this ruling that fails a prong
is a FAIL(CONSTRUCT) under Ruling 1 unless Ruling 5 independently admits the
variable. The variable may not also claim Ruling 1's named-intermediate
relaxation or [[staged-value-reused-variable]], and no consumer of the
variable may be a staged-value borrow. Other locals in the body are judged
under their own entries.

- **(A) Mutually exclusive regions** (condition 2). Every write statement of
  the variable lies in exactly one REGION, and there are at least two regions.
  A region is one arm of an if/else, or a block that every path through it
  leaves by `return`, or the final region of the function: the statements from
  the end of the last other region to the end of the function. No execution
  passes through statements of two different regions. Every read of the
  variable lies in the same region as the write it reads. The variable is
  never read outside the regions.
- **(B) One write per region, before every read** (condition 3). Each region
  contains exactly ONE write statement of the variable. On every path through
  the region, that write executes before any read of the variable in the
  region. If the write sits inside a loop, it is the FIRST statement of that
  loop's body, and every read of the variable lies inside that loop's body
  and reads the value written earlier in the same iteration. A read after the
  loop, or in the loop's header or condition, fails (B).
  A second write in the same region, of any kind (assignment, compound
  assignment, `++`/`--`), fails (B).
- **(C) Same role, same table** (condition 1). Every write's right-hand side
  is the address of an element of ONE AND THE SAME record table, spelled
  identically at every write except for the index expression. The spelling is
  either `&tbl[IDX]`, where `tbl` is a declared array of records, or the
  record-base form `&BASE + IDX * STRIDE` with identical `BASE` and `STRIDE`
  at every write. The record-base form is the convention the Judge cleared for
  func_8002C61C (decisions.md 2026-09-06 06:44). The right-hand side contains
  no cast. `IDX` is an integer constant or a read, with no side effects, of
  one integer scalar variable (a local, a parameter or a global). In every
  region the variable is used ONLY to reach the record that region processes.
  It is dereferenced at a constant field offset (reading or writing a field,
  or loading a pointer stored in the record). Or the record's address, plus an
  optional constant field offset and optionally cast to the callee's
  parameter type, is passed to a callee. It is never stored to memory,
  compared, returned, or used in any other arithmetic.
- **(D) Fresh, one job** (condition 1). The variable has a pointer type and no
  other job. It is not a loop counter, it is not reused from an earlier
  purpose, and it is declared once, at the innermost scope that encloses every
  region. Its name names the record role (e.g. `rec`). Generic names (`tmp`,
  `t`, `temp`, `val`, `v`, `p`, `ptr`, `new_var`, register-style names) never
  satisfy this prong.
- **(E) Nothing added.** The shared spelling and the one-local-per-region
  spelling have the SAME statement list. They differ only in declarations and
  identifiers.
- **(F) Receipts** (condition 4). The one-local-per-region spelling is
  measured and recorded in the ledger as failing, together with at least one
  structural respelling. Its sandbox `--diff` has NO source-level hunk. EVERY
  hunk, whether classed operand-only or not-scored, pairs target and split
  instructions with the same opcodes in the same order and identical
  immediates, memory offsets, symbols and relocation addends. Only register
  operands and branch/jump targets may differ. Branch and jump targets are
  compared as offsets from the function's first instruction, not as raw
  addresses. The sandbox object is unlinked, so raw targets differ by one
  constant for the whole function. That delta must be the same constant at
  every branch and jump in every hunk; any other raw-target delta counts as a
  non-register difference. A not-scored hunk that differs in anything other
  than a branch/jump target (for example a section-relative addend) is a
  non-register difference. Any non-register difference means the sharing is
  doing more than holding one pointer, and this ruling does not apply.
- **(G) Normal review.** Layer-1 and the Judge, or layer-2 on the manual path,
  apply unchanged.

**What stays banned, and why each still fails:** func_8003FA24 `half`
(borderline.md 2026-09-22: straight-line staging of constants in one path,
which fails (A), (B) and (C)); func_800200DC `y1` (fed `dx` then `dy` in one
path, which fails (B)); func_80045878 `c` (several writes in one path, one of
them a constant, which fails (B) and (C)); func_80060A68 `src`/`idx` (re-load in
one path, which fails (B)); a counter or any other variable pressed into
service as the record pointer (fails (D)); a pointer carried out of its region
into later code (fails (A)); a record picked through a cast or from a different
table in another region (fails (C)). This ruling reopens none of them.

**Known weakness:** as with Ruling 5, the only codegen effect of the sharing is
one pseudo spanning every region (func_8001FBE8: the call-free D_800A3758 block's
pointer is seated in callee-saved `$s1` with the loop's pointer, as in the
target; split locals put it in `$a1`, 14/289, every scored hunk operand-only).
Allocator effect alone is never sufficient. This ruling admits the effect only
when the sharing reads as ordinary one-role C under (A)-(F). Record:
docs/grind/decisions.md 2026-09-23 OWNER RULING — Ruling 6.

## Ruling 7 (owner, 2026-09-23) — sprintf's SOTN buffer-end line (sprintf only)

**Question as put to the owner** (manual session, plain language; the filed
form is docs/grind/borderline.md 2026-09-23 sprintf): "sprintf only matches
using one line copied from SOTN. It finds the end of its local text buffer by
taking the address of a *different* local variable (the varargs pointer) and
counting backwards past a neighbouring struct. SOTN ships that line, but our
reviewer failed it: it relies on how locals happen to be laid out on the
stack. Every honest way to write it scores 82–96 off, because the compiler
then keeps the varargs pointer in a register. Should that SOTN line be allowed
for sprintf?" Options offered: keep it banned / allow it, sprintf only /
decide later.

Owner (Trenton), selected option verbatim: "Allow it, sprintf only".

**Rule text.** This is the author's narrowing of that answer, not the owner's
words. It admits ONE statement in ONE function and nothing else:

- **(A) Scope.** Only the function `sprintf` (PsyQ LIBC, 0x80079A30,
  `src/text1b_b.c`). No other function may cite this ruling; a second function
  wanting the same shape needs its own owner ruling.
- **(B) The statement.** Only the SOTN line
  `bufPtr = (char*)&args - sizeof(printf_info) - 4;`
  (SOTN `src/main/psxsdk/libc/sprintf.c:90`), appearing EXACTLY ONCE in the
  body. `bufPtr` is the digit cursor, `args` is the function's own `va_list`
  local and `printf_info` its own flags struct. The statement's only job is
  to set the cursor to the end of the local `buf`, and the target frame must
  confirm the arithmetic: `buf` base + sizeof(buf) == the address of `args` -
  sizeof(printf_info) - 4 (sprintf.s: buf sp+0x10, info sp+0x210, args
  sp+0x220 -> sp+0x210). The standard `va_start`/`va_arg` macro expansions
  (address of the last named parameter, advancing `args`) are exempt. Apart
  from those, no other statement takes the address of `args`, and no other
  statement derives an address from the address of a different local or
  parameter.
- **(C) Annotation.** The line carries an inline comment that says it is
  SOTN-verbatim, says that it computes `&buf[sizeof(buf)]` via the frame
  layout, and cites this ruling. SOTN's own comment claiming
  `&buf[0x200 - 4]` is inaccurate for this frame and must not be carried
  over.
- **(D) Receipts.** The truthful `bufPtr = &buf[sizeof(buf)];` spelling is
  recorded as measured and failing in `memory/grind/sprintf/` (96/535 on
  2026-09-23), together with the other stdarg spellings tried.
- **(E) Everything else is judged normally.** Every other construct in the
  body passes the ordinary review on its own merits. In particular, each
  load-bearing do-while(0) must meet every prerequisite of
  do-while-zero-exception.md exactly as that file states them (an inline
  FAKE annotation at the wrap naming the observed effect; natural geometry
  preferred over a wrap; a written single-level-insufficient justification
  for any nested wrap). This ruling neither adds to nor waives any of them,
  and the presence of a FAKE comment does not by itself show that the other
  prerequisites are met. This ruling sanctions nothing but the (B) statement.

This does NOT relax the cross-symbol address-derivation ban (2026-07-05
ruling, do-while-zero-exception.md forbidden #5) for globals or named
symbols, nor admit any other derivation between locals. Record:
docs/grind/decisions.md 2026-09-23 OWNER RULING — Ruling 7.

## Ruling 8 (owner, 2026-09-24) — vmNoiseOn's SOTN pan-stage `temp` (vmNoiseOn only)

**Question as put to the owner** (manual session), verbatim: "vmNoiseOn now
matches the original exactly, but only with one line pattern copied from
SOTN. SOTN's matched version of this same Sony function uses one scratch
variable, `temp`, for three pan values in a row (tone pan, program pan, voice
pan). Every other spelling I measured is off by a few register choices. Those
were: reading the fields directly, and one separate variable per pan. Our
rules don't allow a local that is assigned three times with different fields,
so this needs your call. Should that SOTN `temp` reuse be allowed?"

Options, verbatim:
- "Allow, vmNoiseOn only (Recommended)": "Record a narrow ruling first as its
  own rules commit: the SOTN-verbatim `temp` pan cascade is allowed in
  vmNoiseOn only. Then land vmNoiseOn (plus the voice-table cleanup) after a
  fresh adversarial review."
- "Allow as a class": "Record a standing ruling: a scratch variable reused
  exactly as SOTN's matched code reuses it in the same Sony library function
  is allowed. Covers future cases like SpuVmSetVol's pan cascade too."
- "Don't allow": "Keep vmNoiseOn as INCLUDE_ASM with the candidate saved in
  the ledger, log the question to borderline.md, and keep searching for
  another spelling."

Owner (Trenton), selected option verbatim: **"Allow, vmNoiseOn only
(Recommended)"**.

**Correction to the question (recorded 2026-09-24, before any code spends
this ruling).** The question said the other spellings are "off by a few
register choices". Re-measured on the whole translation unit
(tmp/vmn/tucheck.py), each alternative also schedules the pan loads later and
emits one extra load-delay `nop` (388 lines against the target's 387), so the
difference is not register-only. In the author's judgment this does not
change the question put to the owner, which was whether SOTN's verbatim
`temp` reuse is allowed. It is recorded so the record is accurate.

**Rule text.** This is the author's narrowing of that answer, not the owner's
words. It admits ONE local in ONE function and nothing else:

- **(A) Scope.** Only the function `vmNoiseOn` (PsyQ LIBSND vm_no1.c,
  0x80086CF8, `src/main.c`). No other function may cite this ruling. A pan
  cascade elsewhere (e.g. SOTN's SpuVmSetVol) needs its own owner ruling.
- **(B) The local and the cascade.** One `u32 temp;`, declared once at
  function scope with no initializer. The function contains the following
  statements exactly once, as one contiguous run, immediately after the
  statement `volr_t = (volr_t * _svm_cur.tone_vol) / 0x7F;` and immediately
  before `if (_svm_stereo_mono == 1)`. Only whitespace and line breaks may
  differ:

  ```
  temp = _svm_cur.tone_pan;
  if (temp < 0x40) {
      voll = voll_t;
      volr = (volr_t * temp) / 0x3F;
  } else {
      voll = (voll_t * (0x7F - temp)) / 0x3F;
      volr = volr_t;
  }
  temp = _svm_cur.mpan;
  if (temp < 0x40) {
      volr = (volr * temp) / 0x3F;
  } else {
      voll = (voll * (0x7F - temp)) / 0x3F;
  }
  temp = _svm_cur.pan;
  if (temp < 0x40) {
      volr = (temp * volr) / 0x3F;
  } else {
      voll = (voll * (0x7F - temp)) / 0x3F;
  }
  ```

  This is SOTN's matched vmNoiseOn cascade (sotn-decomp
  `src/main/psxsdk/libsnd/vmanager.c`:251-270 at aa53500, `u32 temp;` at
  :238; that file is `[0x12A0C, c, psxsdk/libsnd/vmanager]` in
  `config/splat.us.main.yaml` and has no INCLUDE_ASM). The only change from
  SOTN is that its field names become BB2's `struct struct_svm` names at the
  same offsets: `field_E_pan` (+0xE) -> `tone_pan`, `field_B_mpan` (+0xB) ->
  `mpan`, `field_0x5` (+0x5) -> `pan`. `temp` appears nowhere else in the
  function: no other read, write, compound assignment, `++`/`--`, and no
  `&temp`. `voll`, `volr`, `voll_t` and `volr_t` are the function's ordinary
  volume variables and are judged under (E).
- **(C) Annotation.** An inline comment at the declaration or the first write
  says that the reuse is SOTN-verbatim, says what the three writes hold, and
  cites this ruling.
- **(D) Receipts.** `memory/grind/vmNoiseOn/evidence.md` records two failing
  spellings, each measured on the full translation unit with the rest of the
  candidate unchanged, and gives each one's result. The first is the (B)
  cascade with every `temp` replaced by the field it holds. The second is the
  (B) cascade with three once-written `u32` locals (`tone_pan`, `mpan`, `pan`)
  in place of `temp`. Both differ from the target, in vmNoiseOn only.
- **(E) Everything else is judged normally.** Every other construct in the
  body passes ordinary review on its own merits. That includes the `idx`
  named intermediate, under the named-intermediate entry and its
  prerequisites, and the `_svm_voice` record declaration, under the
  aggregate-merge prongs (a)-(e). This ruling sanctions nothing but (B).

A `temp` that misses any prong of (A)-(D) gets nothing from this ruling. It
is judged under Ruling 5 (as amended by its extension) and fails there; the
name `temp` alone fails Ruling 5 prong 1(f). `temp` may not also claim Ruling
1's named-intermediate relaxation, Ruling 6, or
[[staged-value-reused-variable]]. This does NOT relax Ruling 5, its
extension, or Ruling 6 for any other variable or function. It also does not
relax the multi-WRITE carrier bans (y1, `c`, `src`/`idx`), which stand.
Record: docs/grind/decisions.md 2026-09-24 OWNER RULING — Ruling 8.

## Ruling 9 (owner, 2026-09-25) — one meaning, several constant offsets

**Question and answer.** After the 2026-09-25 manual-lane run, the owner asked
the operator for recommendations on four open questions in
docs/grind/borderline.md. This one is the 2026-09-25 entry "func_800759D0 (and
func_8007636C) — one role, differing constant offsets": may one local, stored
to the same struct member at every site, be written at several sites whose
right-hand sides differ only by a constant offset (`q = s.sp18 + 0xC;` /
`q = s.sp18 + 0x24;`)? The operator's recommendation, verbatim:

> **Recommendation: allow it under tight conditions.** In func_800759D0 every
> write feeds the same consumer (`s.sp1C = q`) and means the same thing: "the
> sprite image pointer for this draw". The offsets `+0xC` and `+0x24` pick
> which image, which acts as a selector even though it isn't a subscript. A
> single-meaning variable assigned at several places is ordinary C. The rule
> exists to stop meaningless carrier variables, and this isn't one.
>
> Conditions I'd attach:
> - every write has the same meaning and the same kind of consumer;
> - the variable gets a descriptive name (e.g. `img`, not `q`);
> - the rule never extends to variables that change meaning between writes.
>
> Then rename func_8007636C's `q` in its re-audit instead of reverting it. This
> differs from the 2026-09-24 class you declined, which was "reuse that matches
> SOTN". The test here is one meaning, not a precedent.

Owner (Trenton), verbatim, answering all four recommendations together: **"Go
ahead and do your recommendations then"**.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words. A fresh local written more than once that meets EVERY prong
(a)-(i) below is judged under this ruling INSTEAD of Ruling 5, its extension
and Ruling 6 (a variable meeting every prong of Ruling 10 is judged under Ruling 10 instead). A variable that misses any prong gets nothing from this ruling.
It is judged under Ruling 5 (as amended by its extension) or Ruling 6, and
fails unless one of them independently admits it. The variable may not also
claim Ruling 1's named-intermediate relaxation, Ruling 8, Ruling 10 or
[[staged-value-reused-variable]], and no consumer of it may be a staged-value
borrow. Other locals in the body are judged under their own entries. No FAKE
annotation is required for the reuse itself; any other family the body relies
on keeps its own annotation requirement.

- **(a) One consumer** (Ruling 5 1(a), unchanged). Every write feeds the SAME
  consumer: the same struct member of the same object, or the same argument
  slot of the same callee, never another local. The consumer statements are
  textually identical at every site (e.g. `s.sp1C = img;`). Statements after
  the consumer that use its destination (e.g. `s.sp1C += ...`) are ordinary
  program logic, judged on their own merits.
- **(b) One meaning: one base, constant offsets, sub-objects of one kind**
  (the recommendation's first and third conditions). Every write statement
  has the form `VAR = BASE + K;`. The write statements are textually identical
  except for K, a nonzero integer constant; several writes may use the same K.
  BASE is ONE AND THE SAME expression, spelled identically at every write: a
  local variable or parameter of this function, or a member of a local object
  (e.g. `s.sp18`), read with no side effect. The right-hand side contains no
  cast, no call and no operator other than that one `+`. At every write, BASE
  holds the address of a record, and BASE + K is the address of a sub-object
  of that record. Every write reaches a sub-object of ONE kind: the same type
  and layout, used in the same way by every other reader of it. The
  function's ledger must SHOW this, with evidence independent of the
  byte-chasing: for each K, which sub-object it reaches, the record layout
  that places it there (a stride or struct visible in the original binary, or
  an existing declaration), and other readers that use that sub-object the
  same way (other functions, cited by file and line). An assertion is not
  evidence. A write that reaches a sub-object of a different kind (e.g. a
  count at one offset and an image at another), or one that other readers use
  differently, fails (b). That is what "changes meaning between writes"
  means here, and this ruling never extends to it. Where every condition of
  amendment (b′) below holds (owner, 2026-09-25, second batch), "at every
  write" is judged by the layout the code assumes, and a documented anomaly
  path does not by itself fail (b). Nothing else in this prong changes.
- **(c) Read once, beside its write** (Ruling 5 1(c)). Each write is read
  exactly once, by the consumer. The write and its consumer sit in the same
  compound statement (the same `{ }` body, or both at function scope), the
  write first, with no `return`, `break`, `continue` or `goto` between them.
  A variable with any other reader fails.

  **Clarification (owner, 2026-09-26): a `break` that only exits a loop in
  between.** The question put to the owner, verbatim: "Ruling 9 prong (c):
  may a `break` that only exits an inner loop sit between the write and its
  use?" The owner chose "Yes, inner-loop break OK (Recommended)", whose text
  is, verbatim: "The break never skips the use. Unblocks func_8006F97C (full
  match, everything else already accepted)." What follows is the author's
  narrowing of that option, not the owner's words.
  - **Admitted.** A `break` statement between a write and its consumer does
    not violate (c) when BOTH hold: (1) the statement it exits under C's
    rules (the innermost `for`, `while`, `do` or `switch` enclosing it) is a
    `for`, `while` or `do` loop, not a `switch`; and (2) that whole loop
    statement, its header included, lies after the write and before the
    consumer, inside the compound statement that holds both of them. Such a
    `break` transfers control to the point just after that loop, which is
    still before the consumer, so it cannot skip the consumer.
  - **Still violates (c).** A `return` or `goto` anywhere between the write
    and the consumer, including inside the nested loop. A `break` whose
    loop begins before the write, ends after the consumer, or encloses
    either of them. A `break` that exits a `switch`, and any `continue`: the
    question put to the owner was about a `break` that exits a loop, and
    this clarification decides nothing else, so the unclarified text
    governs them and they violate (c).
  - **Nothing else changes.** Each write is still read exactly once, by the
    consumer. The nested loop neither reads nor writes the variable (a read
    there is another reader and fails (c); a write there fails (g)). The
    loop itself is ordinary program logic, judged on its own merits. Every
    other prong, amendment (b′) and the still-banned list stand unchanged.
    Record: docs/grind/decisions.md 2026-09-26 OWNER RULING — Ruling 9 (c)
    clarification: a `break` confined to a loop in between.
- **(d) Real computation** (Ruling 5 1(e)). Every write's addition appears in
  the target's own bytes at that site (e.g. `addiu a1,v1,0xC`). A write whose
  value is a constant, or a bare copy of another variable (K = 0), fails.
- **(e) Every site is a complete use** (replaces Ruling 5 1(d) and the
  extension's (C)). Sites need not be sibling blocks, and two sites may share
  one block. On every path from each consumer store to the next consumer store or, for the last one, to the function's exit, the consumer's object is read by a statement that is not a consumer store (for
  example `func_8007352C((s32)&s);` draws with it). A consumer store that is
  overwritten before anything reads the object fails. This prong is the
  author's interpretation, not a narrowing of Ruling 5 1(d). The
  recommendation named func_8007636C for re-audit, not reversion, and its
  third loop holds two sites in one loop body, each followed by its own draw
  call. Keeping 1(d) would decide that re-audit in advance. The
  complete-use condition keeps what 1(d) guards against: straight-line
  staging, where a value is set and then replaced before anything uses it.
- **(f) A descriptive name** (the recommendation's second condition; replaces
  Ruling 5 1(f)). The name states the one meaning of prong (b), and it is
  true of every write (e.g. `img`, the image entry this draw uses).
  Single-letter names never qualify. Nor do the generic names Ruling 5 1(f)
  lists (`tmp`, `t`, `temp`, `val`, `v`, `ptr`, `p`, `new_var`,
  register-style names, `arg`/`argN`/`aN`), or a name true of only some
  writes (e.g. `frame1`). Where (b) relies on amendment (b′), the name and
  every source comment on the variable describe the meaning under the layout
  the code assumes, and say so as what the code assumes (e.g. "loop 2 treats
  every sheet as three headers"). A comment that states it as a fact about
  all the data (e.g. "every sheet has three headers") fails (f) when a (b′)
  anomaly path exists.
- **(g) Nothing added, nothing reloaded** (Ruling 5 prong 2, unchanged).
  2(a)-(d) apply as written. In particular, the reuse spelling and the
  one-local-per-write spelling have the same statement list, and no write
  stores a value the variable already holds, judged by C semantics as in
  2(c).
- **(h) Not a borrow** (Ruling 5 prong 3, unchanged). The variable has no
  other job, no other declaration is moved or re-scoped, and it is declared
  once, at the innermost scope that encloses all of its writes.
- **(i) Receipts and review** (Ruling 5 prong 4, unchanged, plus the (b)
  evidence). The one-local-per-write spelling is measured and recorded as
  failing. The ledger shows the ordinary ladder ran before the reuse form was
  adopted: structural respellings, the permuter from the carrier-free
  candidate, and an allocation dump. The (b) layout and reader evidence is
  recorded. Normal review applies: layer-1 and the Judge, or layer-2 on the
  manual path.

**What stays banned, and why each still fails:** func_800200DC `y1` (fed `dx`
then `dy`: different consumers and meanings, (a)/(b)); func_80045878 `c`
(`s1[3]`, `a0 + 3`, `0x8000`: no common base, one write a constant,
(b)/(d)); func_80060A68 `src`/`idx` (re-load of an unchanged value, (g));
func_8003FA24 `half` (constants staged in a straight line, (b)/(d)/(e));
prnt's `n`, and any multi-role scratch of its kind (width and precision
digits, then padding counts: different consumers and meanings, (a)/(b)/(f);
only Ruling 10 can admit prnt's `n`); func_8001BE20 `shift` (2026-09-25:
`arg0 * 16`, then `0` or `arg0 * 4`: different quantities and a constant,
(b)/(d)); func_8005490C `obj` (2026-09-25: different consumers, each init
write read twice, the right-hand side a call, (a)/(b)/(c)); func_8002A458
dx/dy/dz (2026-09-25: each written three times with different quantities,
(b)); func_8008B488 `rate` (2026-09-25: five different ADSR fields feeding two
different registers, with clamp constants, (a)/(b)/(d)); func_80057E84 `vtx`/`node` (2026-09-25: a cast, scaled cursor and `&buf->node[c]`, not `BASE + K`, (b)); func_800288C8 `tbl` and func_8002A458 `lzc_in` (2026-09-25: bare copies and loads, (b)/(d)). This ruling does NOT
reopen the class the owner declined on 2026-09-24 ("a scratch variable reused
exactly as SOTN's matched code reuses it", decisions.md Ruling 8 entry): its
test is one meaning shown by layout evidence, and a SOTN or other precedent
counts for nothing under it.

**func_8007636C directive.** func_8007636C landed COMPLETED-C in d844de59a
(2026-09-24). Its function-scope `q` has this shape: five writes of
`s.sp18 + 0xC` / `s.sp18 + 0x24`, each read by `s.sp1C = q;`. No ruling
admitted it then. As the approved recommendation says, it is re-audited under
this ruling with `q` renamed to a name that meets (f). It is NOT reverted to
`INCLUDE_ASM` ahead of that re-audit. The rename must be verified oracle-green.
The re-audit is a fresh layer-2 cheat-reviewer on the renamed body against
(a)-(i), including the (b) evidence and the (i) receipts in
`memory/grind/func_8007636C/`. This ruling does not pre-decide the outcome.
A FAIL is handled like any other failed re-audit of a landed function.
func_800759D0's rejected form is likewise re-submitted to a fresh layer-2
under this text, with `q` renamed.

**Known weakness (carried over from Ruling 5):** the only codegen effect of
the reuse is one pseudo spanning all sites (func_800759D0: global.c seats it
in `$a1` at every site as the target does; per-site locals 25/364). Allocator
effect alone is never sufficient. This ruling admits the effect only when the
reuse reads as one meaning under (a)-(i). Record: docs/grind/decisions.md
2026-09-25 OWNER RULING — Ruling 9.

### Amendment (b′) (owner, 2026-09-25, second batch) — the layout the code assumes

**Question and answer.** func_800759D0's body landed under Ruling 9 in
23045f51f and was reverted in da429a8c2. A combined layer-2 re-review found
one reachable write that is not a sub-object of its record. When the cursor
is on an unavailable cell, func_80075F80 writes the placeholder 0x14 into the
pick slot. In the same frame, loop 2 of func_800759D0 computes
`cells = table[0x14 + 1] + 0x24`. D_SEL.BIN sheet [21] has one header and ends
at 0xCA0, so +0x24 = 0xCA8 lies past the record. Prong (b) failed at that
write, and (f) failed because the "three headers" comment was false there.
The docs/grind/borderline.md entry "func_800759D0 — Ruling 9 (b) on a
latent-bug path" asked the owner whether (b) should be judged by the layout
the code assumes rather than by the data on a buggy path. After the first
batch was carried out, the operator reported three open decisions and the
owner asked, verbatim, "What are your recommendations?". The operator's
recommendation on this one, verbatim:

> ## 1. func_800759D0: judge Ruling 9 by the layout the code assumes
> **Recommendation: amend prong (b) narrowly, then re-land the function.**
>
> The purpose of prong (b) is to show that a variable has one meaning in the
> code, and that is a property of the source the programmer wrote. Here the
> code consistently treats these sheets as 3-header sheets. The target itself
> addresses headers 1 and 2 at +12/+24, and the draw function walks the table
> as cells. The only problem is one placeholder sheet that is read out of
> bounds. That's a data or state bug in the original game. Failing the
> function for it means refusing to reproduce the original's own bug, which is
> the opposite of the project's goal.
>
> To keep this from becoming a loophole, the amendment should require:
> - the assumed layout is established by the code's own accesses (target
>   instructions, other readers), not just argued;
> - every write reaches a same-kind sub-object on every path shown to be
>   normal;
> - any path where it doesn't is documented in the ledger as a reachable
>   anomaly in the original (bad data, a placeholder, a bug), with its
>   trigger;
> - the source comment describes what the code assumes, not a claim about all
>   the data. The old "three headers" comment was false, which is why prong
>   (f) failed.

Owner (Trenton), verbatim, answering all three recommendations together:
**"Go ahead with your recommendations"**.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words. Conditions (1)-(4) are the recommendation's four, tightened
where marked. In prong (b), "at every write, BASE + K is the address of a
sub-object of that record, of ONE kind" is judged by the layout the code
consistently imposes on BASE's record, instead of by the data at every
reachable write, only when ALL of (1)-(4) hold:

- **(1) The layout is established by the code, not argued.** The assumed
  layout (record stride, header count, the sub-object at BASE + K) is shown by
  the code's own accesses: instructions in the target function's bytes, and
  other readers of the same object, cited by file and line. For
  func_800759D0 these are the highlight step `s.sp18 + 12 + arg3 * 12`
  (`asm/funcs/func_800759D0.s`:112 `addiu $s7,$v0,0xC` and the loops'
  copies, per its ledger), which addresses headers 1 and 2, and
  func_8007352C, which walks `.table` as 8-byte cells. A layout inferred only
  from the data, from a name, or from the fact that the body matches is not
  established. The `VAR = BASE + K` writes under judgment are not evidence of the layout. (1) requires accesses other than those writes.
- **(2) Every normal path reaches a same-kind sub-object.** On every path the
  ledger shows to be normal, every write reaches a sub-object of the kind the
  assumed layout places at BASE + K. This is shown with (b)'s ordinary
  evidence, for example a census of every record each write site reaches.
  A path is normal unless (3) documents it as an anomaly.
- **(3) Every other reachable path is a documented anomaly in the original.**
  Each reachable path on which a write does not reach such a sub-object is
  recorded in the ledger as an anomaly in the original game: bad data, a
  placeholder, or a bug. The record gives:
  - its trigger, meaning the state or input that reaches it (e.g. the 0x14
    placeholder func_80075F80 writes for an unavailable cell);
  - the measured bytes: the address BASE + K takes there and what the
    original disc or EXE holds at it (e.g. sheet [21] at 0xC84 ends at 0xCA0;
    +0x24 = 0xCA8). This requirement is the author's addition;
  - why the access is malformed under the assumed layout.

  A path is an anomaly only if BASE + K lies outside BASE's record (past its end or before its start) as the original data lays it out. A path on which BASE + K lands inside a record on a sub-object of a different kind is a second meaning and fails (b), however the code then uses it. No anomaly record cures it.
  This test is the author's narrowing. A reachable path that the
  ledger neither shows to be normal nor documents as an anomaly fails (b).

  **Clarification (owner, 2026-09-25): past the end, into unreferenced
  bytes.** The owner chose the option "Past the end (Recommended)", whose
  text is, verbatim: "Record a narrow clarification: the landing point counts
  as outside the record if it is past the sheet's end and isn't the start of
  any object the program actually uses. func_800759D0 then re-lands after a
  fresh review." The full question, options and answer are in the decision
  record. What follows is the author's narrowing of that option, not the
  owner's words.
  - **Anomaly.** A BASE + K that lies past the end of BASE's record, as the
    original data lays it out, counts as outside the record under (3). This
    holds even when the address falls among other bytes of the same image,
    provided it is not the start of any object the program actually
    references. "Inside a record" in the sentence above means BASE's own
    record, or a referenced object whose start BASE + K hits.
  - **Referenced object** (author's narrowing). An object is referenced if
    the program reaches it by any of these:
    - a code access that uses the address as that object's start or base, including a computed access (a stride, count or offset walk over the containing data) whose reachable range includes the address;
    - a pointer, offset or table entry in the original EXE or data that
      resolves to it;
    - a symbol that code uses.

    A field or other sub-object that code accesses within a referenced
    object counts as an object of its own. A header-shaped run of bytes that
    nothing references is not an object for this test.
  - **Still fails.** A BASE + K that is the start of a referenced object, or
    of a used sub-object of one, is a second meaning and fails (b) as before.
  - **Evidence.** The ledger records the search that shows the landing
    address is unreferenced: the containing file's pointer tables and root
    slots, other data or EXE words that resolve to it, and every code walk over the containing data, with its reachable range, showing that none reaches the address. The measured bytes are recorded as (3) already requires.
    The search requirement is the author's narrowing. An unsearched address
    fails (b).
  - **Scope.** This decides only the past-the-end case the owner was asked
    about. It does not decide a BASE + K before the start of BASE's record
    that falls among other bytes. It does not change (1), (2), (4), the
    rest of (b), or the still-banned list. Record: docs/grind/decisions.md
    2026-09-25 OWNER RULING — Ruling 9 (b′)(3) clarification: past-the-end
    into unreferenced bytes.
- **(4) The comment describes the assumption.** Every source comment on the
  variable describes what the code assumes, never a claim about all the data
  (prong (f) as amended above).

**What (b′) does not change.** Every other requirement of (b) stands: the
`VAR = BASE + K;` form, one BASE spelled identically, no cast, call or other
operator, K a nonzero integer constant, and the layout and reader evidence.
Prongs (a) and (c)-(i), the exclusivity clause and the Known weakness stand
unchanged. **(b′) does NOT admit a variable whose writes differ in meaning on
normal paths.** If any normal path reaches a sub-object of a different kind,
(b) fails exactly as before, and no anomaly record cures it. It does not
reopen any item on the still-banned list above, nor the class the owner
declined on 2026-09-24.

**Application.** func_800759D0's reverted body
(`memory/grind/func_800759D0/rejected/ruling9-cells-placeholder-overrun-0.c`)
may be re-submitted to a fresh layer-2 under Ruling 9 with (b′). First, its
ledger records (1)-(3) in the form above, and its declaration comment is
rewritten to meet (4). The re-landing is an ordinary completion with its own
layer-2 review, and this amendment does not pre-decide the outcome.
func_8007636C's re-audit PASS (71b14499d) did not rely on (b′) and is
unaffected. Record: docs/grind/decisions.md 2026-09-25 OWNER RULING — Ruling 9
amendment (b′).

## Ruling 10 (owner, 2026-09-25) — verified original source, verbatim reuse

**Question and answer.** Same exchange as Ruling 9. This is the 2026-09-25
borderline.md entry "prnt — original-library-source shared scratch `n`".
prnt (PsyQ LIBC2 PRNT) is a `putchar` port of 4.3BSD-Reno `_doprnt`, whose
own source declares `register int n; /* random handy integer */` and reuses
`n` for the width and precision digits and the padding loops. The body
matches only with that single `n` (sandbox 0/418, full-build SHA1 == oracle);
split into one variable per job it scores 21/418. The operator's
recommendation, verbatim:

> **Recommendation: allow a narrow "verified original source" exception.** On
> its own merits `n` is the multi-purpose scratch variable the rule is meant
> to ban. But the goal of the project is to recover the original source, and
> here the original source actually exists: 4.3BSD-Reno's `doprnt.c` is
> public, and the version is known (sccsid 5.39). Copying its own variable
> verbatim is the most faithful C possible.
>
> This is much narrower than what you declined on 2026-09-24, because SOTN's
> code is itself a decompilation, not original source. Conditions:
> - the original source is public and checkable;
> - the function is shown to be a transcription of it, with the differences
>   listed;
> - the reuse is copied verbatim.
>
> That would cover only a few library functions.

Owner (Trenton), verbatim: **"Go ahead and do your recommendations then"**.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words. A fresh local written more than once that meets EVERY prong
(A)-(F) below is judged under this ruling INSTEAD of Ruling 5, its extension,
Ruling 6 and Ruling 9. A variable that misses any prong gets nothing from this
ruling. It is judged under those rulings and fails unless one of them
independently admits it. The variable may not also claim Ruling 1's
named-intermediate relaxation, Ruling 8 or [[staged-value-reused-variable]].
Other locals in the body are judged under their own entries.

- **(A) The original source is public and pinned** (the recommendation's first
  condition). The function's original source text is public and
  independently checkable. That is the source file the function was compiled
  from, or the upstream file its port was made from. The ledger records an
  archive URL pinned to a commit or tag, the version identifier the file
  itself carries (e.g. the sccsid `@(#)doprnt.c 5.39 (Berkeley) 6/28/90`),
  and the file's SHA-256. The ledger also confirms, character for character,
  every cited line against a second, independent copy of the same version
  (another archive). The SHA-256 and second-copy requirements are the
  author's narrowing, mirroring the provenance bar of inline-asm-policy.md
  § Owner ruling 2026-09-23. **A decompilation is NEVER an original source.**
  SOTN, psyz and every other decompilation project's C is a reconstruction by
  its authors, whatever its match status. Library code with no public
  original source (e.g. func_8008B488's LIBSPU body, for which only
  decompilations were found) cannot use this ruling.
- **(B) The function is shown to be a transcription** (second condition). The
  ledger maps the BB2 function onto that source statement by statement, for
  the whole function. It lists EVERY difference: added, removed or changed
  statements, parameters, types and expanded macros (for prnt, e.g.:
  `if (fmt0 == NULL) return 0;`, `putchar` in place of the FILE buffering,
  ordinary characters not counted, the float cases absent). Where versions of
  the source differ in any statement that touches the reused variable, the
  ledger shows that the target follows the cited version. A function that is
  only similar to the source fails (B).
- **(C) The original's own variable, verbatim** (third condition). The
  reused local is a variable the original declares in that function. It keeps
  the original's name (e.g. `n`), and its type is the original's or the
  project's direct equivalent (`int` -> `s32` or `int`). It is written and read
  ONLY at statements that correspond to the original's writes and reads of that
  variable, in the original's order, with the same expressions up to the
  differences listed under (B). No write or read the original lacks. A use may
  be missing only where the whole surrounding code is absent from the port and
  listed under (B). The original's `register` storage class is not carried
  over. Plain `register` hints stay under the standing cheat policy, and the
  sandbox strips them (engine/inlineasm.py).
- **(D) Annotation.** An inline comment at the declaration says that the
  reuse is the original source's own variable. It cites the source (archive
  URL, version identifier, the declaration's line and the lines that reuse
  it) and this ruling.
- **(E) Receipts.** The ledger records the one-variable-per-role spelling as
  measured and failing, and at least one other respelling (e.g. a partial
  split). Normal review applies: layer-1 and the Judge, or layer-2 on the
  manual path.
- **(F) Everything else is judged normally.** Every other construct in the
  body passes ordinary review on its own merits. Other constructs copied from
  the original (gotos, macros, casts, statement order) get nothing from this
  ruling. This ruling sanctions nothing but the (C) variable.

**Scope.** This covers few functions: library code whose original source
survives publicly, such as prnt from 4.3BSD-Reno. It does NOT reopen the
class the owner declined on 2026-09-24 ("a scratch variable reused exactly as
SOTN's matched code reuses it"). SOTN is a decompilation, so such a reuse fails
(A). func_8008B488's shared `rate` (SOTN's `var_a2` in `_SpuSetVoiceAttr`)
therefore stays inadmissible. Ruling 8 stays vmNoiseOn-only. Ruling 5, its
extension, Ruling 6, Ruling 9 and the multi-WRITE carrier bans are unchanged
for every variable this ruling does not admit. prnt's rejected form is
re-submitted to a fresh layer-2 under this text. Its ledger
(`memory/grind/prnt/evidence.md`, 2026-09-25) records the URL, sccsid and
excerpts, but not yet the SHA-256 or the second-copy confirmation (A). It
needs them first. Record: docs/grind/decisions.md 2026-09-25 OWNER RULING —
Ruling 10.

## Ruling 11 (owner, 2026-09-26) — a reused local proven necessary by the allocator

**Question and answer.** Three 2026-09-26 docs/grind/borderline.md entries
asked whether a local may hold values that no earlier ruling lets one
variable share: func_80055138 (one `s32 v` for five values), func_8003993C
(`win` for a weapon-set selector and the replay window; `key` set from a
different table in each arm of an if/else) and func_8002DE20 (one
`side_a`/`side_b` pair shared by twelve same-side tests). The question put to
the owner, verbatim: "Allow a local reused for several unrelated values when
compiler dumps prove no one-variable-per-value spelling can match?" The
framing, which the owner approved: "Allow it only with proof: the worker must
show from compiler dumps that no single-purpose spelling can match, give the
variable an honest generic name, and still pass layer-2."

Owner (Trenton) chose, verbatim: **"Allow with proof (Recommended)"**, whose
text is: "Admit only when allocator-dump proof shows necessity, honest generic
name, and layer-2 still reviews. Unblocks 80055138, 8003993C, 8002DE20 (C
part)."

**Rule text.** This is the author's narrowing of that answer, not the
owner's words. It applies only to a fresh local written more than once that
none of Ruling 5 (with its extension), Ruling 6, Ruling 8, Ruling 9 or
Ruling 10 admits. A variable one of them admits is judged under that ruling
and gets nothing from this one. A variable in scope here is admitted ONLY if
it meets EVERY prong (A)-(H) below; missing any prong is a FAIL(CONSTRUCT)
under Ruling 1. The variable may not also claim Ruling 1's named-intermediate
relaxation or [[staged-value-reused-variable]], and no consumer of it may be
a staged-value borrow. Each variable is judged on its own: a pair such as
func_8002DE20's `side_a`/`side_b` is two variables, and each must meet every
prong. Other locals in the body are judged under their own entries.

"Value", in this ruling, means one group of writes that can reach a common
read: two writes of the variable belong to the same value when some read of
the variable can read either one. For example, in
`t = x / 3; if (t > 9) t = 0; use(t);` both writes reach `use(t)`, so they
are one value. Writes that no common read can reach belong to different
values.

- **(A) A fresh local, not a borrow.** The variable is a local of this
  function: not a parameter, a global, or a `static` or `register` variable.
  It is declared once, at the innermost scope that encloses all of its writes,
  and no other declaration is moved or re-scoped. Its address is never taken
  (no `&var` anywhere in the function).
- **(B) Every write is live.** (1) Each write's value is read on at least one
  path before the variable is next written or goes out of scope. A write
  whose stored value is never read (a dead store in Ruling 2's store-level
  sense) fails; this ruling admits nothing the dead-store family would need.
  (2) No write stores a value the variable already holds, judged by C
  semantics exactly as in Ruling 5 prong 2(c), including its 2026-09-26
  clarification: a write (including a re-load) fails only when the
  variable already holds the written value on EVERY feasible incoming path
  (branch conditions jointly satisfiable), and each write that re-stores a
  held value on some feasible incoming path carries the ledger record of a
  feasible incoming path where the variable holds a different value (for a
  re-load, where the lvalue or the variable has been written since the last
  load).
- **(C) The one-variable-per-value spelling has the same statements.**
  (1) The ledger records the one-variable-per-value spelling: each value gets
  its own fresh local, declared at the innermost scope enclosing that value's
  writes. (2) That spelling and the reuse spelling have the SAME statement
  list: they differ only in declarations and identifiers (Ruling 5 prong
  2(b)). A write or read that exists only in the reuse form fails.
  (3) Every value is a real computation: at least one of its writes is a
  load, an arithmetic computation or a call result whose instructions appear
  in the target's own bytes. A value whose writes are all literal constants,
  or bare copies of another named variable or parameter, fails.
  Constant-holders stay under [[named-local-fake-exception]], staging copies
  under [[staged-value-reused-variable]], and the F1 constant-staging chain
  stays refused.
- **(D) Allocator-dump proof of necessity** (the owner's first condition).
  The function's ledger (`memory/grind/<func>/`) banks all of:
  (1) **The dumps.** The compiler's allocation dumps for BOTH the reuse
  spelling and the one-variable-per-value spelling, compiled with the build's
  flags: the `.lreg`, `.greg` and `.flow` dumps (cc1 `-dl -dg -df`) and/or the
  instrumented cc1's `BB2_*_DEBUG` output (`tools/gcc-2.7.2/cc1`), with the
  command lines. Excerpts are enough if they carry the pseudo numbers, the
  register each value receives and the allocator decision at issue.
  (2) **The mechanism.** The allocator decision that seats the target's
  register(s) is named by pass and by source location in `tools/gcc-2.7.2`
  (e.g. local-alloc.c `combine_regs` tying a pseudo that lives in one basic
  block to a dying input; global.c's allocno priority order). The dumps show
  that decision going the target's way in the reuse spelling and the other
  way in the one-variable-per-value spelling.
  (3) **Necessity, not effect.** The ledger states the property of the reuse
  spelling that the decision depends on (e.g. "the pseudo is live in more
  than one basic block, so local-alloc does not allocate it"), and shows that
  EVERY one-variable-per-value spelling lacks that property BECAUSE each value
  has its own variable, whatever its declaration order, scope, type,
  statement order or other respelling. An argument that covers only the
  spellings that were measured fails (D). So does one showing only that the
  reuse spelling scores better: that is an allocator effect, not necessity.
  (4) **Measured alternatives.** `sandbox --disable all` scores for: the full
  one-variable-per-value spelling; where the variable holds three or more
  values, each value split out on its own with the rest still shared (the
  ablation); at least one structural respelling; and a permuter campaign from
  the one-variable-per-value body, with its best score and what its finds
  reuse ([[permuter-fresh-seed-discipline]]).
- **(E) An honest generic name** (the owner's second condition). The name
  claims nothing false about any value the variable holds. It is either
  (i) a generic scratch word, `temp`, `tmp`, `work` or `scratch`, optionally
  followed by digits or by `_` and one lowercase letter to tell two such
  variables apart (e.g. `temp2`, `tmp_a`); or (ii), when every value is the
  same kind of quantity, a name for that kind that is true of every write,
  judged as in Ruling 9 prong (f) (e.g. `cross_a` when every write is a cross
  product). A name that states the role of only some values fails (e.g. `win`
  for a variable that also holds a weapon-set selector). Single-letter names,
  register-style names (`v0`, `a1`, `s2`), `new_var`, `arg`/`argN`/`aN`, and
  the coercion-announcing names of [[no-new-park-categories]] (`pad`, `dummy`,
  `spill`, `slack`, `_unused`, `_tmp`) never qualify. Form (ii) is the
  author's reading of "honest generic name" for same-kind values.
- **(F) Annotation** (the author's addition). An inline comment at the
  declaration says that the variable holds several values, names each value
  (or, under (E)(ii), their kind), and cites this ruling and the ledger file
  that holds the (D) proof.
- **(G) Layer-2 still reviews** (the owner's third condition). Admission
  needs a fresh layer-2 `cheat-reviewer` PASS on the body, given the (D)
  ledger, that walks every prong of (A)-(H). On the Grinder path a Judge PASS
  is not enough: the Judge may not admit a variable under this ruling, and a
  body that relies on it lands only through the manual path's layer-2. The
  ordinary completion gates (sandbox 0, full-build SHA1 == oracle) apply.
- **(H) Everything else is judged normally.** Every other construct in the
  body passes ordinary review on its own merits. This ruling sanctions
  nothing but the variable that meets (A)-(G).

**Relation to "allocator effect alone is never sufficient".** Rulings 5, 6
and 9 each close with a Known weakness: the only codegen effect of their
reuse is one pseudo spanning several sites, and allocator effect alone is
never sufficient. That sentence stands for those rulings and for every
variable this ruling does not admit. Ruling 11 is the owner's one exception to
it: it admits a variable on allocator NECESSITY, proven under (D) for every
one-variable-per-value spelling, never on a measured effect.

**What stays banned, and what is not reopened.** A re-load of an unchanged
value (func_80060A68 `src`/`idx`) fails (B)(2). Constants staged through a
local (func_8003FA24 `half`, the F1 chain) and bare copies (func_800288C8
`tbl`, func_8002A458 `lzc_in`) fail (C)(3). A write whose value is never read
fails (B)(1). A variable whose per-value spelling needs an extra or a missing
statement fails (C)(2). This ruling reopens no earlier FAIL by itself:
func_800200DC `y1`, func_80045878 `c`, func_8008B488 `rate` and every other
variable on the still-banned lists of Rulings 5, 6 and 9 can return only as a
fresh submission that meets every prong, and nothing here pre-decides one.
It does NOT reopen the class the owner declined on 2026-09-24 ("a scratch
variable reused exactly as SOTN's matched code reuses it", decisions.md
Ruling 8 entry): a SOTN or other precedent counts for nothing under this
ruling. Only the (D) proof admits.

**Application.** func_80055138 (`v`), func_8003993C (`win` and `key`) and
func_8002DE20 (`side_a`/`side_b`) may each be submitted to a fresh layer-2
under this text once their ledgers carry the (D) proof and their variables
meet (E) (`v` and `win` need new names) and (F). The outcomes are not
pre-decided. func_8002DE20's GTE islands are a separate admission step
(inline-asm-policy.md § Owner ruling 2026-09-26). Record:
docs/grind/decisions.md 2026-09-26 OWNER RULING — Ruling 11.

## What this ruling does NOT change

- The completion bar, the oracle, the cheat catalog for non-C mechanisms,
  the frozen family list's owner-only extension, no-deferral, the
  default-FAIL posture of layer-1 and the Judge, and two-layer
  adversarial acceptance for manual work.
- [[escalation-not-parked]] (2026-08-24) is superseded ONLY in disposition
  shape: its two-state model becomes active/foreclosed, its
  decision-packet mechanism is retired. Its AUTO-REJECT class survives
  intact — a would-be packet whose YES lowers a standard is now simply a
  FAIL.

## Related

[[no-new-park-categories]] · [[dead-store-fake-exception]] ·
[[judge-sole-gate]] · [[integration-handoff-self-serve]] ·
[[escalation-not-parked]] · [[completion-standard]] ·
[[no-deferral-work-to-completion]] · [[difficult-is-not-impossible]]

## Amendment 2026-09-02 — foreclosure mechanics (owner ruling, decisions.md "foreclosure mechanics")

Measured 2026-09-02: 11 of the 12 items unparked on 2026-09-01 were re-foreclosed
after ONE session because an unpark did not reset the driver's flat-floor window,
and six items were foreclosed under the 2026-07-27 standing ruling with ledger
floors of 8..20 — outside that ruling's "a few instructions short" subject. Owner
ruled three mechanical changes (no standard changes):

1. **An unpark resets the exhaustion window.** The driver stamps
   `exhaustion_base` in the ledger when a queue item returns to active;
   `_exhaustion_ready` counts only sessions after it. A ruling buys a full
   fresh window, never one session.
2. **Standing-ruling foreclosure is scoped to the endgame-lock floor
   (`ENDGAME_LOCK_MAX_FLOOR = 5`).** Flat floor <= 5: existing 8-session /
   4-modality trigger and the `RESOLVED BY STANDING RULING (2026-07-27)` record.
   Flat floor > 5: the ladder runs a SECOND full cycle (20 flat sessions, >= 6
   modalities) before exhaustion may fire, and the record is titled
   `LADDER EXHAUSTED (non-endgame residual, floor N): FORECLOSED` — never
   claiming endgame-lock status. Flat at 0 keeps the 8-session trigger.
3. **Spending a probe is not a disposition.** Foreclosure records are only
   valid in driver-assigned `escalation` modality (both titles); post-unpark
   sessions are ordinary ladder sessions, so a killed probe is `progress`.

Retroactive: the 11 same-day re-foreclosures and the 6 out-of-scope floors
returned to active with the window reset (17 items); `main` and
func_80027640 stay foreclosed under their own records.

## Amendment 2026-09-04 — escalation deferral and equal-floor sibling propagation (owner ruling, decisions.md "foreclosed-bucket re-evaluation", Ruling B)

Measured 2026-09-04: the driver's exhaustion backstop fired on one predicate
(escalation modality AND floor did not drop), so CD_sync s116 — which CONFIRMED a
lever for the first time in 116 sessions, named the new wall, and expressly
declined to self-file — was auto-foreclosed anyway; and the sibling-progress
stamp fired only on a STRICTLY lower floor, so CD_datasync s58's link-identical
struct spelling reached neither twin at the same floor 2. Owner ruled two
mechanical changes (no standard changes; the windows, gates and
`ENDGAME_LOCK_MAX_FLOOR` are untouched):

1. **A confirmed lever is progress, not a dodge.** An escalation-modality
   session that returns `progress` at a flat floor is honored as ordinary
   progress when it banks at least one QUALIFYING CONFIRMED hypothesis
   (numeric measurement in `result`, a `measured_on` chassis, and a statement
   not already CONFIRMED in an earlier session of the same ledger) AND names a
   frontier item — at most `ESCALATION_DEFERRALS_MAX = 2` times per exhaustion
   window (reset on unpark and on any floor drop). The driver forces the next
   session, one-shot, to the first of solver / permuter / structural /
   forensics not run since the window base. The third such session is
   backstopped exactly as before. This is not a second ladder cycle
   (asm-until-matched R1 unchanged) and a re-confirmation buys nothing.
2. **Sibling propagation fires at or below your floor.** A genuine sibling
   floor drop TO your floor (from a different chassis) forces the same
   one-shot `rederive` transplant session that a drop below it already did.

`main`'s residual (two branch-displacement words behind the 2026-08-24 maspsx
branch-fill DECLINE) is a fidelity question logged to `docs/grind/borderline.md`
for the owner's own hand; nothing is granted and `main` stays foreclosed.
