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
(2026-09-23; as amended by its 2026-09-23 extension) or of Ruling 6
(2026-09-23), whichever governs that variable, exclusively: the reused
variable itself may not also claim this entry or
[[staged-value-reused-variable]]; other locals in the same body,
including a Ruling 5 1(b)(ii) selector binding, are judged under their
own entries.) All other prongs (real value, byte-neutral, fresh not
borrowed, destination not live-pre-initialized, standard prerequisites)
are unchanged.

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
instead). This ruling governs that case exclusively: the reused variable itself
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
