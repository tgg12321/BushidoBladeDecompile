# SELF-VET — func_800307D0 (cpu_check_tubazeri_2)

Session s9, 2026-08-25. Diff = src/code6cac_b.c:1159 `INCLUDE_ASM("asm/funcs", func_800307D0);`
replaced by the pure-C body. That is the whole diff; nothing else in the tree is touched.
Measured this session with the body in place: `sandbox func_800307D0 --disable all` =
**score 0, 76/76 target insns, rules_dropped 0**; `verify-oracle --rebuild` = **build_sha1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, build_matches true**.

CONSTRUCTS: none. The body contains no match-hack construct of any family: no invented
locals, no dead stores, no self-assigns, no aliases, no volatile, no unused variables, no
inline asm, no `do{}while(0)`, no duplicated statements, no borrowed/reused variables, no
padding arrays. Every declared local is written with a real value and read; every statement
is load-bearing for the semantics. The four spellings a reviewer may want justified are
enumerated below, and each is ordinary C.

Spellings under review:
  (A) head read `id = *(s16 *)(a0 + idx * 2 + 0x332);` — scaled-index read of the 0x332
      array off the `u8 *a0` param.
  (B) staged xor `top = top ^ cur;` then `idx = (u32)top < 1;` (two statements, not one
      folded expression).
  (C) two forward `goto pick_index;` early-skips over the tie-break computation.
  (D) `(u32)top < 1` as the "the two ids are equal" test.

## T1 semantic purpose
(A) Reads the element the function returns and the element func_80030580 is called with —
without it the function has no value to return. Semantic, not decorative.
(B) `top` holds the xor of the queue-head id and the current id; the next statement tests
it. The value is consumed; deleting either statement changes behaviour.
(C) The gotos implement the real control flow: with fewer than 2 queued items, or with the
0x88 field at -1, there is no second candidate to tie-break against, so index 0 is used.
Deleting them changes which element is removed.
(D) It is the equality test itself. Deleting it changes which element is removed.
No construct in this diff is behaviour-neutral; there is nothing here whose removal leaves
the function output unchanged.

## T2 human-programmer
(A) Yes — and demonstrably so in this very translation unit: three already-COMPLETED-C
functions in the same TU read the same 0x332 array with the same scaled-index spelling
(src/code6cac_b.c:1123, :1226, :1239; src/code6cac.c:2002). The owner adopted exactly this
basis in the 2026-08-25 ruling (a).
(B) Yes — "combine the two values, then test the combination" is an ordinary two-statement
spelling; naming the intermediate is how a person writes it when the intermediate has a
meaning (do the head entry and the current entry differ?).
(C) Yes — a forward goto that skips an optional refinement step and lands on the common
tail is ordinary C and is used throughout this codebase's decompiled peers.
(D) Yes for a decomp of this era: the source computes a difference and tests it against
zero in the unsigned domain. A reader asks no "why is this here?" question about any of the
four — each is answerable purely from what the function does.

## T3 GCC-internals justification
No. The justification for every construct in this diff is program logic (T1/T2 above), and
the diff stands on the owner 2026-08-25 ruling (a) that spelling (A) is ordinary C judged on
the construct's own merits. Two internals-adjacent facts exist in the ledger and are
disclosed here as MEASUREMENTS, not as the reason any construct is present:
  - Spelling (A) was, in earlier sessions, argued for via expr.c EXPAND_SUM reasoning. That
    rationale is explicitly NOT relied on here; the ruling vacated it as a FAIL basis and I
    do not re-raise it as a justification. (A) is submitted because it is the TU's own idiom
    for this array and it is what the function needs to do.
  - Folding (B) into one expression measures score 4 (banked as
    rejected/folded-xor-head-score4.c). That is a recorded measurement of an alternative
    spelling, not a compiler-internals mechanism claim: I do not assert, and do not need,
    any pass-level story for why the two-statement form is what the original source had.
No GCC pass, allocator, scheduler, RTL, LUID, reg_n_refs or priority concept is offered as
the mechanism by which anything in this diff works.

## T4 permuter/search provenance
No construct here is permuter output. The permuter campaigns of s4/s5 (three basins,
~144k iterations) found nothing that is in this body; their only novel find was a
score-neutral pointer-alias variant which is NOT used. The body is the banked candidate the
owner ruled on, with this session's simplifications (listed at the end) chosen to REMOVE
constructs, not to find bytes. Nothing here survives only because a detector missed a
spelling — there is no construct for a detector to catch.

## T5 family check
No forbidden family and no sanctioned family is engaged, because there is no match-hack
construct. Explicitly checked against the near neighbours:
  - variable-reuse: NOT present. This session deliberately de-reused the banked candidate's
    single reused local (its `v1` carried both the 0x14 read and the later
    `*(s16 *)(obj + 2)` read); the submitted body gives those two values separate names
    (`cur`, `kind`) and still measures 0. The diff is strictly further from that family than
    the ruled-on body was.
  - named-intermediate / staged-value: NOT present. `top` and `cur` are not invented
    carriers for a value that already had a home; they are the two operands of a real
    computation whose result the next statement consumes.
  - dead-store / self-assign / constant-holder / dead array: NOT present — no store in this
    body is dead; there is no unread local, no array, no constant holder.
  - pointer-alias / volatile / asm / do-while(0) / duplicated-statement: NOT present.
  - The 2026-07-22-refused integer-cast form `v0 + (s32)a0` is NOT used and is not respelled
    here; that refusal is untouched.
  - Banned constructs still in force are respected: I make no enumerated-variant-matrix
    argument (KEPT ban) and I self-resolve no escalation (KEPT ban) — this session acts on
    the owner's written ruling, cited by file:line below.

## T6 naming-announces-intent
No name in the diff announces coercion intent. Locals are `count`, `idx`, `top`, `cur`,
`kind`, `id`, `obj`, `i`; the label is `pick_index`. This session renamed the banked body's
register-derived names (`s1`, `s3`, `a2`, `v0`, `v1`, label `do_sll`) to semantic ones and
re-measured 0 — names are codegen-neutral here, and the semantic names are what a reader
should see. There is no `pad`, `dummy`, `unused`, `spill`, `tmp_buf`, `slack` or similar,
and no local whose only uses are discards, address-of, or declaration.

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no construct requiring a family, so no
scope sentence or precedent is claimed. The governing authority for submitting spelling (A)
is the owner's ruling, not a family grant:
  RULING: docs/grind/decisions.md:11472 — "2026-08-25 — func_800307D0
  (cpu_check_tubazeri_2) — **OWNER RULING: (a)**", which states the head read "is a
  structural choice between two ordinary, semantically identical C spellings of the same
  in-TU-idiomatic array read, and is judged on the construct's own merits, not on how the
  author found it", vacates the two head-read spelling bans plus the
  sibling-precedent-rationale and reliance-on-15:45 bans as FAIL bases, and directs the next
  session to re-apply the banked body and take the normal layer-1 + Judge path.
  IN-TU PRECEDENT for the idiom (the owner-adopted basis): src/code6cac_b.c:1123.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff. No construct of any family is
present, so nothing in this body requires or carries a /* FAKE */ annotation.

## Diff-from-the-ruled-on-candidate (full disclosure)
Relative to the body the owner ruled on, this session made three construct-removing or
neutral edits, each re-measured at sandbox 0 / 76 insns / 0 rules:
  1. Dropped the two per-arm `s32 a0_arg = ...;` locals, inlining the real expressions into
     the func_80032854 call arguments. Removes two fresh once-written/once-read locals that
     would otherwise have invited a named-intermediate family question.
  2. Split the candidate's reused `v1` into `cur` and `kind`. Removes the only
     variable-reuse in the body.
  3. Renamed register-derived locals and the label to semantic names; stripped the candidate
     file's grind-narration header so no ledger narrative enters src/ (standing Judge
     constraint jc[2]).
One alternative was measured and rejected, not adopted: folding the xor into the compare
(score 4) — banked at rejected/folded-xor-head-score4.c.
