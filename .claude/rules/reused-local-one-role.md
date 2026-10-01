---
name: reused-local-one-role
paths: [".claude/rules/reused-local-one-role.md"]
description: "Ordinary-C Rulings 5 (+ extension) and 6: a fresh local written more than once for ONE role repeated per block, or one record pointer written once per exclusive path."
metadata:
  type: rule
---

# Reused locals: one role per block, one pointer per path

Part of [[ordinary-c-judge-decidable]]: a fresh local written more than once is admitted only
under exactly one of Rulings 5-12 (or a Q51 SOTN citation, [[sotn-precedent-suffices]]), which then
governs it exclusively. Questions and verbatim answers: docs/grind/owner-rulings-2026-09-26.md
and the docs/grind/decisions.md OWNER RULING entry named in each heading.

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
