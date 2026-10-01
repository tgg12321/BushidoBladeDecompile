---
name: reused-local-necessity
paths: [".claude/rules/reused-local-necessity.md"]
description: "Ordinary-C Rulings 11-12: a reused local or a stack-param copy admitted only on compiler-dump proof of necessity (mechanism + search), honest name, annotation, fresh layer-2."
metadata:
  type: rule
---

# Reused locals and param copies proven necessary

Part of [[ordinary-c-judge-decidable]]: a fresh local written more than once is admitted only
under exactly one of Rulings 5-12 (or a Q51 SOTN citation, [[sotn-precedent-suffices]]), which then
governs it exclusively. Questions and verbatim answers: docs/grind/owner-rulings-2026-09-26.md
and the docs/grind/decisions.md OWNER RULING entry named in each heading.

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
