---
name: ordinary-c-judge-decidable
paths: ["tools/grinder/**", "engine/queue.py", "docs/grind/*.md"]
description: "Owner rulings 2026-08-31+: ordinary-C candidates are judged on the C text against the frozen family list (non-membership = FAIL, never a question); dead-store deadness is store-level; the reused-local Rulings 5-12; clearly-fine unprecedented C may PASS (13B)."
metadata:
  type: rule
---

# Ordinary C is Judge-decidable (owner ruling 2026-08-31 and later rulings)

Owner: *"My primary concern is agents slipping through new 'cheats' by another name, like what
historically happened with regfix and asmfix. But if what the agent is trying to do is SOTN
standard, and not a cheat, i dont want them escalating it to me either."* Out-of-band mechanisms
(pins, asm patches, build-time rewriting) stay dead by construction. Choosing among truthful C
spellings by observing codegen is the METHOD of matching decompilation, not a cheat signal.
Each ruling's question and verbatim answer: docs/grind/owner-rulings-2026-09-26.md and the
docs/grind/decisions.md OWNER RULING entry named in its heading.

## Ruling 1 — the checklist (no escalation)

1. **Zero non-C mechanisms** (sandbox + detectors).
2. **Construct-class membership.** Every no-semantic-purpose construct sits inside the frozen
   list ([[no-new-park-categories]] § SOTN-accepted) with that entry's prerequisites met.
   Non-membership = FAIL(CONSTRUCT) + borderline entry, never an escalation. A verified SOTN
   citation also admits a construct (Q50, [[no-new-park-categories]] § SOTN precedent
   suffices; manual-path layer-2 only).
3. **The rename test**: judge the C TEXT. Does each construct have a truthful semantic
   reading, do neutral names survive, is the mandated annotation present? A semantically
   truthful spelling is never a cheat merely because it was chosen after observing the
   scheduler. Constructs with no semantic reading stay governed by their family entries.
4. **Simplest-known-form**: of several byte-exact forms, the one with the fewest
   no-semantic-purpose constructs lands.

**Named intermediate relaxed to "once-written"** (SOTN `new_var_temp` class): a fresh local
holding a real, consumed value may be read any number of times. Evidence caveat presented to
the owner: SOTN's index shows such locals exist, not that their shape matches line-for-line.
**Multi-WRITE locals stay banned** unless they meet every prong of exactly one of Rulings 5, 6,
8, 9, 10, 11 or 12, or a Q51 SOTN reuse citation; whichever applies governs that variable
exclusively (it may not also claim the named-intermediate entry or
[[staged-value-reused-variable]]). Where a ruling below says a variable "fails", Ruling 11 (or
a Q51 citation) can still admit it.

## Ruling 2 — dead-store deadness is STORE-level

In [[dead-store-fake-exception]], a store whose STORED VALUE is never read is in scope even if
the variable is later re-assigned and read (`x = a; ... x = b;`, all reads after the second).
All other prerequisites of that rule are unchanged. No previously FAILed instance is
legalized; each is judged fresh.

(Ruling 3, the silent `foreclosed` state, is superseded by [[rotation-not-foreclosure]].)

## Ruling 4 — compound-assignment splits are ordinary C (2026-09-02)

`ratio *= 0x103B; ratio >>= 12;` for `ratio = (ratio * 0x103B) >> 12;`, or `v = a; v += b;`,
is truthful C under Ruling 1(3) when no statement is dead, no annotation is needed and no pad is
introduced. Multi-write carriers whose extra write is dead stay banned.

## Ruling 5 — one role repeated per block (2026-09-23)

A fresh local written more than once, admitted only if ALL hold (no FAKE needed for the reuse):
1. **One consumer role, one template.** (a) Every write feeds the SAME consumer (same member of
   the same object, or same argument slot of the same callee), never another local. (b) The
   write statements are textually identical except the SELECTOR: the integer-constant
   subscript of ONE AND THE SAME base expression (one subscript position; blocks may reuse a
   subscript). The base is a declared array or pointer variable, used without a cast, whose
   elements are interchangeable sibling records (or pointers to them) used the same way. Never
   a selector: a base declared/initialized from a cast of an address (`(T *)&obj`,
   `(T *)&obj.f`, `(T *)obj.arr`, `(T *)G` where G is not an array of those records); an
   array/pointer over an object whose slots play different roles; a cast or pun at the use
   site; subscripts reaching distinct named fields; two different named variables. (Loading a
   pointer VALUE from memory is not a cast.) The selector appears directly in the write or in a
   block-local binding written once per block, textually identical except the subscript.
   Consumer statements are textually identical across blocks. (c) Each write is read exactly
   once, by the consumer, in the same block. (d) Writes sit in distinct sibling blocks (or arms
   of one conditional), each ending in the consumer: repetitions of one piece of code.
   (e) Every RHS is a load or arithmetic computation whose instructions appear in the target;
   a literal constant or bare copy fails. (f) The name names the consumer role (`p1` for
   `s.p1`; for a call argument, what it means: `ot`, `rect`). Generic (`tmp`, `t`, `temp`,
   `val`, `v`, `ptr`, `p`, `new_var`, register-style), single-letter and `arg`/`argN`/`aN`
   names fail.
2. **Nothing added, nothing reloaded.** (a) Every write is used before the next. (b) The reuse
   and the one-local-per-write spellings have the SAME statement list (differ only in
   declarations/identifiers). (c) No write stores a value the variable already holds on EVERY
   feasible incoming path (jointly satisfiable branch conditions), judged by C semantics; an
   intervening store is ignored only if it provably targets a distinct object. A write that
   re-stores a held value on some path is recorded in the ledger with a feasible path where the
   variable holds something else. A re-load of the same lvalue fails when, on every feasible
   path, neither lvalue nor variable was written since the last load (2026-09-26
   clarification). (d) No single computation split across writes.
3. **Not a borrow.** No other job; no other declaration moved; declared once at the innermost
   scope enclosing all writes.
4. **Receipts.** The one-local-per-write spelling measured and failing, plus the ladder
   (structural respellings, permuter from the carrier-free candidate, allocation dump).

**Extension (2026-09-23) — identical writes, record picked beforehand.** Replaces only 1(b)
and 1(d), for a variable meeting ALL of: (A) every write and every consumer statement is
character-identical (no selector at all); (B) the RHS reads the record through one member of
the SAME object whose member the consumer writes; before each write, on every path, that
member is assigned (same nesting level) an element read from ONE base expression spelled
identically at every site, and is the LAST write to that member/object before the carrier
write (no write, copy or address-taking in between). A pointer base is a local/param whose
address is never taken and that is not written between the first member store and last
carrier write (no global/static/field pointer). The subscript is an integer constant or the
innermost loop's counter (+ constant); the counter is a non-static local, address never taken,
initialized to a constant immediately before the loop or in its `for` init, and stepped exactly
once per iteration by a nonzero constant `++/--/+=/-=`, never otherwise written. The member
must have a reader other than the carrier write. The member store appears identically in the
one-local-per-write spelling. (C) Each write is followed in the same straight-line block by its
consumer; one site per sibling block, except at most ONE function-scope site preceding all
others. (D) Prong 3 and 2(c) apply; additionally the member was re-assigned to a DIFFERENT
array element since the previous write.

Still banned (Ruling 5 and extension): func_800200DC `y1` (different consumers);
func_80045878 `c` (different templates, a constant write); func_80060A68 `src`/`idx`
(re-load); func_800460E4 off_a/off_b (borrow, split); func_8002D780 `tmp`; constant-holders
spelled as reused locals; cast-laundered or multi-role bases. Allocator effect alone is never
sufficient.

## Ruling 6 — one record pointer, one write per exclusive path (2026-09-23)

Judged instead of Ruling 5 when ALL hold: (A) every write lies in exactly one of ≥2 mutually
exclusive REGIONS (if/else arms, blocks every path leaves by `return`, or the final region);
every read is in its write's region; no read outside regions. (B) Exactly one write per region,
before every read on every path; a write inside a loop is that loop body's first statement and
all its reads are in the same iteration's body. (C) Every RHS is the address of an element of
ONE record table, spelled `&tbl[IDX]` or `&BASE + IDX * STRIDE` identically except IDX (an
integer constant or a side-effect-free read of one integer scalar); no cast; the pointer is only
dereferenced at constant offsets or passed (± constant offset, optional cast to the parameter
type) to a callee — never stored, compared, returned or otherwise computed with. (D) Pointer
type, no other job, declared once at the innermost scope enclosing every region, named for the
record role (generic names fail). (E) Same statement list as one-local-per-region. (F) That
split spelling is measured failing, with a structural respelling, and its sandbox `--diff` has
NO source-level hunk: only register operands and branch targets differ (targets compared as
offsets from the function start; one constant raw delta everywhere). (G) Normal review.

## Ruling 7 — sprintf's SOTN buffer-end line (sprintf only, 2026-09-23)

Only `sprintf` (`src/text1b_b.c`) may carry, exactly once, SOTN's
`bufPtr = (char*)&args - sizeof(printf_info) - 4;` (sotn `src/main/psxsdk/libc/sprintf.c:90`),
with the frame confirming `&buf[sizeof(buf)]`, an inline comment (SOTN-verbatim, what it
computes, this ruling; not SOTN's inaccurate comment), the truthful spelling recorded failing in
the ledger, and no other address derived from another local. Everything else in the body is
judged normally (do-while(0) wraps per [[do-while-zero-exception]]).

## Ruling 8 — vmNoiseOn's SOTN pan `temp` (vmNoiseOn only, 2026-09-24)

Only `vmNoiseOn` (`src/main.c`) may carry one function-scope `u32 temp;` used solely by the
SOTN-verbatim three-step pan cascade (sotn `libsnd/vmanager.c:251-270` @aa53500; fields
`tone_pan`/`mpan`/`pan`), placed right after the `volr_t` scaling and before
`if (_svm_stereo_mono == 1)`, with an inline comment, and the field-direct and three-local
spellings recorded failing. Everything else is judged normally.

## Ruling 9 — one meaning, several constant offsets (2026-09-25)

Judged instead of Rulings 5/6 when ALL hold: (a) one consumer, consumer statements identical.
(b) Every write is `VAR = BASE + K;` with ONE identical BASE (a local, parameter or local
object's member, read without side effect), K a nonzero integer constant, no cast/call/other
operator; at every write BASE + K is a sub-object of ONE kind of BASE's record, shown in the
ledger with evidence independent of the byte-chasing (layout, other readers by file:line).
(c) Each write is read once, by the consumer, in the same compound statement, write first, no
`return`/`continue`/`goto` and no `break` between them, except a `break` that exits a
`for`/`while`/`do` loop lying wholly between write and consumer (clarification 2026-09-26);
that nested loop must not touch the variable. (d) Each addition is in the target's bytes at
that site. (e) Every consumer store's object is read (by a non-consumer statement) before the
next consumer store or function exit. (f) A descriptive name true of every write (not generic,
not single-letter, not true of only some writes). (g) Ruling 5 prong 2; (h) prong 3;
(i) receipts + the (b) evidence; normal review.

**(b′) Layout the code assumes (2026-09-25).** (b) may be judged by the layout the code
consistently imposes when ALL hold: (1) that layout is shown by the code's own accesses other
than the `VAR = BASE + K` writes (target instructions, other readers, cited); (2) every normal
path reaches a same-kind sub-object; (3) every other reachable path is a documented anomaly in
the original (trigger, measured bytes, why malformed), where BASE + K lies OUTSIDE BASE's
record. Past the record's end counts as outside when the address is not the start of any
referenced object (code access/walk range, pointer/table entry, used symbol), with the search
banked; landing on a referenced object or a different-kind sub-object fails. (4) Comments
describe what the code assumes, not a claim about all data.

Still banned under Ruling 9: `y1`, `c`, `src`/`idx`, `half`, prnt's `n`, func_8001BE20
`shift`, func_8005490C `obj`, func_8002A458 dx/dy/dz, func_8008B488 `rate` (except via Q51),
func_80057E84 `vtx`/`node`, func_800288C8 `tbl`, func_8002A458 `lzc_in`.

## Ruling 10 — verified original source, verbatim reuse (2026-09-25)

Judged instead of Rulings 5/6/9 when ALL hold: (A) the function's original source (or its
port's upstream) is public and pinned: archive URL at a commit/tag, its version id (e.g.
sccsid), file SHA-256, cited lines confirmed against a second independent copy. A
decompilation (SOTN, psyz, …) is NEVER an original source. (B) Statement-by-statement mapping
of the whole function with EVERY difference listed. (C) The reused local is the original's own
variable, same name, its type or the direct project equivalent, written/read only where the
original does, in order; `register` not carried over. (D) Inline comment citing source and this
ruling. (E) One-variable-per-role spelling + one other respelling measured failing; normal
review. (F) Nothing else in the body gains from this ruling.

## Ruling 11 — a reused local proven necessary (2026-09-26)

For a fresh multi-write local none of Rulings 5-10 admits (a Q51-cited SOTN reuse needs no
(A)-(H) package). "Value" = a group of writes that can reach a common read. ALL of:
- **(A)** A local (not param/global/static/register), declared once at the innermost scope
  enclosing its writes, no other declaration moved, address never taken.
- **(B)** (1) Every write's value is read on some path (no dead stores). (2) Ruling 5 2(c)
  incl. the path-wise clarification and ledger record.
- **(C)** (1) The one-variable-per-value spelling is recorded; (2) it has the same statement
  list; (3) every value is a real computation (a write that is a load, arithmetic or call
  result in the target's bytes). Exceptions to (3):
  - *Per-branch constants (Q20)*: ≥2 writes of DIFFERENT constants on different feasible paths
    selected by a runtime condition.
  - *GTE-macro input copy (Q28)*: exactly one write `var = src;` (src a named local, no cast,
    read again after the copy); every read of that value is the bare variable as the whole
    input operand of an admitted qualifying GTE macro unit ([[inline-asm-policy]]); the
    variable later holds a real computed value; the copy's move is in the target at the same
    position.
  - *Plain copy (Q34)*: exactly one write `var = src;` (src a named local with another read,
    not a param/global/field/element; no cast; no instruction-adding conversion); the target
    has the register move (`addu/or rd,rs,$zero`) at that position, cited; EVERY other value of
    the variable is a real computation; at most one copy-clause value per variable; the fresh
    local and the no-copy body are banked and measured.
  Constants-only, copies of params (Ruling 12), staging chains (F1) stay refused.
- **(D) Dump proof of necessity.** (1) Allocation dumps (`.lreg`, `.greg`, `.flow`, and/or the
  instrumented cc1's `BB2_*_DEBUG`) for the reuse and the one-variable-per-value spellings,
  with command lines; (2) the deciding compiler decision named by pass and source location in
  `tools/gcc-2.7.2`, going the target's way only in the reuse spelling. Any named pass counts
  (Q58), with that pass's own dumps (`.cse`, `.combine`, …) banked for both. (3) **Mechanism +
  search (Q31)**: the mechanism named from dumps; every one-variable-per-value spelling the
  author or any reviewer proposes is banked and measured; no COUNTING spelling reaches the
  target (whole function byte-identical under the build's cc1). A new reviewer spelling that
  misses is evidence, not a FAIL; an unbanked proposal is a banking step. A better score alone
  is not a mechanism. **Q30/Q32 set-aside**: a spelling needing a FAKE/!FAKE-annotated construct
  the reuse body does not carry (while carrying all of the body's own, unchanged or respelled
  within its rule) does not count; bank it with its score and the family sentence requiring
  the annotation. (4) Measured: the full split, per-value ablations (3+ values), a structural
  respelling, a permuter campaign from the split body.
- **(E)** An honest generic name: `temp`/`tmp`/`work`/`scratch` (+ digits or `_x`), or a
  kind-name true of every value; single-letter, register-style, `new_var`, `arg*` and
  coercion-announcing names never qualify.
- **(F)** Inline comment: holds several values, names them, cites this ruling and the ledger.
- **(G)** A fresh layer-2 PASS walking (A)-(H); a Judge PASS is not enough (manual path).
- **(H)** Everything else judged normally.

## Ruling 12 — a local copy of a stack-passed parameter (2026-09-26)

Each copy, ALL of: (A) the parameter is stack-passed and never written, address never taken.
(B) Fresh local of the parameter's type (or typedef-equivalent), written once with the bare
parameter, no cast, never rewritten, address never taken; not also claimed under Ruling 1/11 or
staged-value. (C) Its reads are the function's real uses of the parameter. (D) Per copy:
allocation dumps with and without it; the halved-priority mechanism named by pass/location;
necessity against EVERY spelling without it, including frozen-list constructs, checked against
the real allocator (local-alloc suggestions, `find_reg` preferences, `expand_preferences`,
reload, reorg); measured alternatives incl. a permuter campaign. (E) An honest role name.
(F) Inline comment citing this ruling and the ledger. (G) Fresh layer-2 PASS (manual path).

## Ruling 13 (B) — clearly-fine ordinary C without precedent (standing, owner Q64 2026-09-30)

"No precedent" is not a FAIL ground for a construct with a truthful semantic reading under
Ruling 1(3), writable from the function's behaviour, doing real consumed work (no dead store,
no no-op copy, nothing the compiler deletes, no pad/dummy). The reviewer still judges it on its
merits ("in doubt, FAIL"), and any entry or ruling that governs the construct decides instead.
**The wall stays (13 (D), standing):** no pins; no hardcoded-`$N` or non-canonical `__asm__`;
no scheduling barriers; no build/Makefile/linker/gate/compiler change that alters bytes; no
build-time assembly rewriting; no no-semantic-purpose construct outside the frozen list; no
multi-write local outside Rulings 5-12 / Q51; nothing on a banned list or refused by an owner
ruling. The run-scoped orchestrator-decision and rotation clauses of Rulings 13/14 have
expired.

Related: [[no-new-park-categories]] · [[dead-store-fake-exception]] · [[judge-sole-gate]] ·
[[review-discipline-before-commit]] · [[staged-value-reused-variable]]
