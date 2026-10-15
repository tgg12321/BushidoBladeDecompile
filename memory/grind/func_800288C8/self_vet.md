# SELF-VET — func_800288C8

CONSTRUCTS: GTE island (gte_Lzc, 6 __asm__ statements, src/main/17AFC.c:947-952), FAKE local `dist` (:902), FAKE local `tbl` (:906), FAKE local `shift` (:957, block-scoped inside the else arm)

## T1 semantic purpose
GTE island: real effect — computes the leading-zero count of `tbl` via the GTE coprocessor
(no C form exists for cop2 ops). `dist`: real effect — holds the squared distance then its
square root, consumed by `var_15`/the push-apart math. `tbl`: real effect — holds the GTE
input value, then the table byte consumed by the shift/dist expression. `shift`: real effect —
holds the leading-zero count then the derived table shift, consumed by the two expressions
below it. None of the four is a no-op / dead construct; all are read for their stored value.

## T2 human-programmer test
GTE island: yes — a programmer implementing this GTE-accelerated integer sqrt would write
exactly this macro expansion (or call the SDK macro directly); it's the mechanical PsyQ
macro body, not an invented shape. `dist`/`tbl`: yes — a programmer naturally reuses one
local for "the distance, then its square root" and another for "the GTE's input value, then
the table lookup" in a routine that computes an approximate 3D distance; this reads as
ordinary incremental computation, not something a reader would flag as "why is this here".
`shift`: yes — `shift = lzc_out; shift = 0x16 - (shift & ~1);` is an ordinary two-statement
rewrite of one expression; a reader asks nothing more than "why not one line", which Ruling 4
already answers as ordinary C (no cheat smell).

## T3 GCC-internals justification test
GTE island: the island's STRUCTURE is justified by hardware (cop2 has no C syntax), not by a
GCC internal; the SURROUNDING locals are not asm-internals-justified either. `dist`: the
inline comment does cite a measured register-seat cost ("score 7") as supporting evidence that
splitting the statement is the known SOTN-admitted form, but the construct's SEMANTIC reading
(squared distance -> its square root) stands on its own without the cost figure. Same for
`tbl` (comment cites "score 13 (Q28)" as the Q28-required dump-style note, not as the sole
justification for the construct's existence — Q28 itself requires the semantic reading: a
named value later holding a real computation). `shift`: no GCC-internals justification is
needed or given beyond "the count lands in $a0" parenthetical, which documents target register
behavior but is not the reason the split exists — Ruling 4 holds regardless of register
outcome. None of the four fails T3 outright, but `dist`/`tbl` lean on cited rule numbers
(Q28, Ruling 11) precisely because those rules exist to require more than codegen-only
justification — see SANCTIONED-FAMILY-CLAIMS below for why they still clear item 3.

## T4 permuter/search provenance
None of the four constructs originated from this session's search (I ran no permuter or
enumerator this session — recon modality, read-only investigation). They were already present
in src/ at session start, carrying their own citations (Q28, "score 7", "score 13"), which
point to a prior session's dump-based derivation, not an auto-search artifact being surfaced
uncritically. I did not add, remove, or reshape any of them this session.

## T5 family check
GTE island: inline_o.h verbatim-macro class (inline-asm-policy.md, owner ruling 2026-09-26) —
matches exactly; `inline_asm_canonical.txt:406` already documents this island under that class.
`dist`: ordinary-c-judge-decidable Ruling 11 (C)(3), real-computation prong — every write is a
load/arithmetic result; the lone `dist = 1` clamp is an ordinary conditional override, not a
Ruling-11 "value" of its own. `tbl`: ordinary-c-judge-decidable Ruling 11 (C)(3) Q28 exception
(GTE-macro input copy) — `tbl = dist` is read only as the bare operand of the admitted gte_Lzc
macro unit immediately below, and `tbl` is later written a real computed value. `shift`:
ordinary-c-judge-decidable Ruling 4 (compound-assignment split) — `shift = lzc_out; shift =
0x16 - (shift & ~1);` is the `v = a; v = f(v);` shape Ruling 4 names explicitly as truthful C
needing no annotation (the existing `/* FAKE: */` label is conservative, not false — see note
below).

## T6 naming-announces-intent test
`dist`, `tbl`, `shift`, `lzc_out` are all honest kind-names true of every value they hold
(distance/table-byte/shift-amount/leading-zero-count) — none of `pad`, `dummy`, `unused`,
`spill`, `slack`, `_buf`, `tail` or similar coercion-announcing names appear. Names pass T6.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: inline_o.h verbatim GTE macro block (class grant)
  SCOPE: "Islands in PsyQ inline_o.h form need no per-function `tools/grinder/owner_cluster_grants.txt` row and no STRONG tier when ALL hold: (A) the reference is PsyQ 4.3 `inline_o.h` (and `gtemac.h`) exactly as `engine/gtemacro.py` `PINNED` records them ... (B) each island is a qualifying macro unit, statement for statement and character for character incl. the header's `move $12,%0` statements and `\"$12\",\"$13\",\"$14\",\"$15\",\"memory\"` clobbers (only whitespace differs; joined statements are not covered); (C) NO respelling of any kind ... (D) conditions 2-5 of § Owner ruling 2026-09-23 apply, and the `canonical` gate's route is recorded; (E) layer-2 checks every island against `PINNED`."
  PRECEDENT: inline_asm_canonical.txt:406

  FAMILY: Ruling 11 (C)(3) real-computation multi-write local
  SCOPE: "(3) every value is a real computation (a write that is a load, arithmetic or call result in the target's bytes)."
  PRECEDENT: .claude/rules/reused-local-necessity.md:25

  FAMILY: Ruling 11 (C)(3) Q28 exception -- GTE-macro input copy
  SCOPE: "*GTE-macro input copy (Q28)*: exactly one write `var = src;` (src a named local, no cast, read again after the copy); every read of that value is the bare variable as the whole input operand of an admitted qualifying GTE macro unit ([[inline-asm-policy]]); the variable later holds a real computed value; the copy's move is in the target at the same position."
  PRECEDENT: .claude/rules/reused-local-necessity.md:29

  FAMILY: Ruling 4 -- compound-assignment split
  SCOPE: "`ratio *= 0x103B; ratio >>= 12;` for `ratio = (ratio * 0x103B) >> 12;`, or `v = a; v += b;`, is truthful C under Ruling 1(3) when no statement is dead, no annotation is needed and no pad is introduced."
  PRECEDENT: .claude/rules/ordinary-c-judge-decidable.md:57

ANNOTATION-CONFORMANCE: The three locals carry `/* FAKE: ... */` comments already in src/ (unchanged this session):
  src/main/17AFC.c:899-901 `/* FAKE: two values -- the squared distance, then its square root (clamped to 1); a separate squared-distance local reorders the copy and the r*r test (score 7) */` -- what: dist holds two values; mechanism: register-seat reorder (named, measured "score 7"); lever-exhaustion: implicit in the cited score (prior session's measurement, not re-derived this session).
  src/main/17AFC.c:903-905 `/* FAKE: two values -- the gte_Lzc input copy of the squared distance, then the table byte; a fresh copy local (or dist itself) takes $v1, not the target's $a0: score 13 (Q28) */` -- what: tbl holds two values; mechanism: register-seat (named, measured "score 13"); lever-exhaustion: cites Q28's own requirement that no fresh/dist-reuse spelling reaches $a0.
  src/main/17AFC.c:954-956 `/* FAKE: two values -- the leading-zero count, then the table shift; as one expression `shift = 0x16 - (lzc_out & ~1)` the count lands in $a0 (target: $v1 / $v0) */` -- what: shift holds two values; mechanism: register-seat (named); lever-exhaustion: the one-expression alternative is named and its wrong register cited. NOTE: under Ruling 4 this construct is ordinary C and the annotation is arguably unnecessary (not false, just conservative) -- flagged in evidence.md s1 and hypotheses.md F1 as a possible layer-2/Judge-requested cleanup, not changed this session (recon modality, no src/ edits made).
  No new FAKE construct was added this session; all three were already in src/ at session start, unmodified.
