# SELF-VET — func_80017FA0

STATUS (s5-synthesis, 2026-08-20): SUBMITTED. The tripwire that blocked the
previous staging is gone: the 2026-08-20 03:43 Judge ruling (docs/grind/decisions.md:7835)
explicitly cleared the goto-formed-inner-loop entry via `unban_construct`, ruling
it "NOT a cheat and the banned_constructs[1] entry is a mis-banked paperwork FAIL,
not a construct ban". `state.json.banned_constructs` now carries ONLY the s4
volatile-scratchpad entry, which this body does not contain (the word `volatile`
does not occur anywhere in the function). The one defect the Judge required fixed
before resubmission � candidate.c's file-header line claiming "zero volatile, zero
FAKE constructs", which the annotation made false � is fixed: the header now reads
"zero volatile, no dead locals, no aliases. Exactly ONE FAKE-annotated construct:
the goto-formed spelling of the inner counted loop (annotated inline at the
`inner:` label below)." Nothing else about the form changed.

Re-measured THIS session (s5-synthesis) with the body in src/code6cac.c:
`sandbox func_80017FA0 --disable all` = {"score": 0, "target_insns": 61,
"build_insns": 61, "rules_dropped": 0, "scorable": true} and a full `build` gives
SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the oracle.

Diff under vet: `src/code6cac.c` only — `INCLUDE_ASM("asm/funcs", func_80017FA0);`
replaced by the body in `memory/grind/func_80017FA0/candidate.c`, plus the two
mechanical call-site/prototype fixes the body requires
(`extern void func_80017FA0(void);` -> `extern void func_80017FA0(s32 *);` and
`func_80017FA0();` -> `func_80017FA0(p0);`). No other file in the build pipeline
is touched. Measured THIS session (s5-synthesis, 2026-08-20) with the annotated body in place:
`sandbox func_80017FA0 --disable all` = {"score": 0, "target_insns": 61,
"build_insns": 61, "rules_dropped": 0} and a full `build` SHA1 =
62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle).

CONSTRUCTS: (1) the inner counted loop is spelled goto-formed (`inner:` label plus
`if (j < 2) goto inner;`) rather than as a `do { ... } while (j < 2);` statement,
and carries an inline `/* FAKE: ... */` annotation at the label; (2) rotated
outer-loop entry guard `if (i < ptr[1])` against the live counter `i`;
(3) `goto end;` early-exit to a trailing empty label; (4) ordinary pointer/offset
locals (`scr`, `ptr`, `p68`, `ac_base`, `dp`, `sp_off`, `sp_inner`, `data_off`)
and numeric absolute-address stores `*(s32 *)(0x1F800064 + sp_inner)`. NO volatile
anywhere, no inline asm, no register pins, no dead locals, no local arrays, no
aliases, zero regfix/asmfix rules.

## T1 semantic purpose
(1) The goto IS the inner loop's back edge: delete it and the body executes once
instead of twice — the function then writes 3 of the 6 words per group and is
simply wrong. It is live control flow executed on every group, not a semantically
inert addition. Its SPELLING (goto-formed vs do-while) is what is
purely-for-matching, which is exactly what the FAKE annotation declares.
(2) `if (i < ptr[1])` is the loop's real entry test (`i` is 0 there, so it is the
identical predicate to `ptr[1] > 0`, and it is the same comparison the loop's own
back edge performs); it guards the loop. (3) `goto end;` is the null-pointer
early-out, executed whenever a0[3]==0. (4) Every local carries a value that is
read and stored; every store lands in the target's bytes. Nothing in this diff is
removable without changing what the function computes.

## T2 human-programmer
Yes on all four. A goto-formed counted loop is idiomatic late-90s C and is
already the shipped, byte-matched form of two COMPLETED-C functions in THIS VERY
FILE (`func_800206B0`, src/code6cac.c:2125 + :2143 — including the same "wrap a
temp in a bare block inside the loop body" shape — and `func_80021280`,
src/code6cac.c:2220 + :2224; neither is in engine/queue.json, i.e. both are
COMPLETED-C, and neither carries any annotation). A reader does not ask "why is
this here?" about a loop's own back edge, its entry guard, or its null-check
early-out. The numeric scratchpad addresses are how PS1 code writes scratchpad RAM.

## T3 GCC-internals justification
Stated honestly, and this is the reason the FAKE annotation is now present rather
than argued away: GCC internals explain WHICH of two semantically identical
spellings this reconstruction uses. The construct's PRESENCE is explained entirely
by program logic (it is the loop); only its spelling is matching-driven. The
mechanism was measured, not guessed — from the cc1 `.loop` dump
(tmp/grind/func_80017FA0/s5/vNV.loop): the C front end emits
NOTE_INSN_LOOP_BEG/END only for for/while/do statements and loop.c analyses only
note-delimited loops; with the do-while spelling loop.c forms the three
scratchpad stores' addresses as DEST_ADDR givs of the biv `sp_inner`,
combine_givs merges them and strength_reduce hoists one biased base out of the
loop (57 insns vs the target's 61). Per do-while-zero-exception.md the mechanism
a spelling nudges is not itself disqualifying; the disqualifying set is
non-compiled bytes (regfix/pins/inline asm) and semantic-lie C, and this diff has
neither.

## T4 permuter/search provenance
Not permuter output, and it does not depend on any detector's blind spot. s5 ran
the mandated pass-attribution dump first (`cc1 -dL`), read the giv/combine/reduce
trace in vNV.loop, then read tools/gcc-2.7.2/loop.c (strength_reduce:3823,
combine_givs:5494, combine_givs_p:5457, express_from:5417) to establish that with
a numeric address on one biv, reduction is unconditional — i.e. the giv must never
be FORMED. The loop-note property was derived from that reading and confirmed by
measurement. The construct is visible, live, ordinary, and now annotated.

## T5 family check
No FORBIDDEN family matches, by shape or by analogy. Checked explicitly against
the catalog: the goto is NOT "dead-goto label-pad" (the goto is the back edge and
always executes; nothing is padded) and NOT "goto-end-with-ret-val accumulator +
shared label" (no return value, no accumulator, the label is the loop head; the
separate `goto end;` is the plain mixed-exit form sanctioned by
.claude/rules/cross-jump-store-tail-merge.md). No register-asm pin, no
hardcoded-$N asm, no scheduling barrier, no alias rename, no dead store, no
constant holder, no local array, no aggregate merge, no width-cast games, no
linker-script reorder. The construct sits in the SANCTIONED family claimed below.
The s4 volatile-scratchpad construct is BANNED for this function and is absent —
the word does not occur in the body.

## T6 naming-announces-intent
No name in the diff announces coercion intent. Names are `scr`, `ptr`, `p68`,
`ac_base`, `dp`, `i`, `j`, `sp_off`, `sp_inner`, `data_off`, `temp`, and the
labels `inner` / `end` — all describe what the object or label IS. There is no
`pad`, `dummy`, `unused`, `spill`, `slack`, `_buf` or `tail`, and every named
object is both written and read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: allowed-spelling-of-semantically-true-C (statement/control-structure
    choice) under the owner's 2026-07-06 construct-honesty ruling — the same rule
    file the 2026-08-20 03:31 layer-1 review cited as the governing line.
  SCOPE: "**ALLOWED — any spelling of semantically-TRUE C**, whatever pass it nudges: do-while(0) wraps, split/redundant arithmetic (the SOTN-wiki `+ 1 - 1` class — note this places the 2026-07-05 double-split rejection's SPELLING half under the allowed side; its cross-symbol half stays forbidden under #5), variable reuse/staging, named intermediates, statement order, mixed exit forms."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:46
  PRECEDENT: src/code6cac.c:2143
  PRECEDENT: docs/reference/sotn-construct-index.md:1015

  FAMILY: phantom-slot frame lever, producer #1 (folded loop-guard compare) — for
    construct (2), the rotated outer-loop guard. The 2026-08-20 02:54 Judge and
    the 02:40 and 03:31 layer-1 reviews all examined this lever independently and
    ruled it legitimate with no annotation owed.
  SCOPE: "**Folded loop-guard compare** — a guard comparison pseudo whose compare jump/combine fold into a bare branch, leaving the pseudo ref'd but dead. Spellings that produce it are ordinary C: `s2 = a1 - 1; if (s2 != -1)` (func_8003D9A0), the rotated-while guard `if (i < limit)` re-using the loop's own exit comparison (func_8003DBE4)."
  PRECEDENT: .claude/rules/phantom-slot-frame-lever.md:37
  PRECEDENT: src/code6cac_c2.c:1327

ANNOTATION-CONFORMANCE: one FAKE construct, annotated inline at the construct site
(the `inner:` label in candidate.c / src/code6cac.c), reading:
  /* FAKE: this inner counted loop is spelled goto-formed rather than
   * `do { ... } while (j < 2);` purely for matching, mechanism: GCC
   * 2.7.2 loop.c (strength_reduce/find_mem_givs/combine_givs) analyses
   * only NOTE_INSN_LOOP_BEG-delimited loops, which the front end emits
   * for for/while/do statements only; under the do-while spelling
   * loop.c forms the three scratchpad stores' addresses as DEST_ADDR
   * givs of the biv `sp_inner`, merges them (benefit 6 - add_cost 2)
   * and hoists one biased base, giving 57 insns against the target's
   * 61 (measured: tmp/grind/func_80017FA0/s5/vNV.loop). The loop's
   * semantics are identical either way. lever-exhaustion:
   * memory/grind/func_80017FA0/hypotheses.md (H1-H14) +
   * evidence.md - the numeric-address, extern-symbol and volatile
   * spellings of "stop the giv" are all measured dead or BANNED, and
   * the s1-s3 dead-local frame family was owner-REFUSED. */
It carries all three required parts: WHAT (goto-formed spelling of the inner
loop, chosen for matching), MECHANISM (a named GCC pass — loop.c
strength_reduce / find_mem_givs / combine_givs, gated on NOTE_INSN_LOOP_BEG),
and LEVER-EXHAUSTION (the ledger locations where the alternative levers are
recorded dead).
