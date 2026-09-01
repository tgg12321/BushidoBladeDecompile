# SELF-VET — func_80062020

Session s11, modality `annotation-fix`. Diff: `src/text1b.c` only — the
`INCLUDE_ASM("asm/funcs", func_80062020);` line at 3932 is replaced by the pure-C body
plus one descriptive block comment. No other file in the build surface is touched.

Measured THIS session on the live chassis, with the edit in place:
  `sandbox func_80062020 --disable all` -> score 0, target_insns 38, build_insns 38,
    rules_dropped 0, cheat_asm_stripped 166 (all from OTHER functions in text1b.c).
  `verify-oracle` -> ok true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.

SCOPE NOTE (annotation-fix contract): the C body is BYTE-FOR-BYTE IDENTICAL to the body
the Judge ruled on 2026-08-31 22:24 (banked at rejected/judge-fail-0831-2224.c) — same
statements, same order, same identifier names (`i`, `ofs`, `t`, `row`). The ONLY changes
are comments: the inline FAKE block before the column-0 store is DELETED, and the
pre-function comment block that cited proven-spelling-class-reconstruction and owner
ruling 6a with the four-point claim is REPLACED by a plain description of what the function
does. That is exactly the Judge's stated defect and nothing else.

CONSTRUCTS: none. Concretely: no dead store, no self-assign, no constant holder, no
written-never-read local, no local array, no volatile of any kind, no pointer alias to a
global declared for codegen purposes, no `do { } while (0)` wrap, no dead goto/label pad, no
duplicated statement into arms, no opaque arithmetic variable, no asm, no pin, no pragma, no
gate-list or build change. Every declared local is written and read; every statement performs
a store or an address computation that appears in the target. The one thing a reviewer will
notice is a SPELLING choice, not an added construct: the terminator row's column-0 store is
written `*(s32 *)((u8 *)&D_800F1198 + ofs) = 0;` while columns 1 and 2 are written
`row[2] = 0; row[1] = 0;` through the row pointer. That statement is required by the
function's semantics and writes an lvalue the target writes; only its spelling was chosen.

## T1 semantic purpose
Every construct answers yes. The three epilogue stores write three DIFFERENT lvalues
(row+0, row+4, row+8) and all three appear in the target's bytes. `row` is read twice.
`ofs`, `i` and `t` are each read after being written. Delete any one of them and a store
or an address the target performs disappears. There is no construct in this diff whose
removal is byte-neutral, which is the definition the test applies — so nothing here is a
no-semantic-purpose construct, and Ruling 1 criterion 2 (frozen-list membership) does not
engage at all.

## T2 human-programmer
Yes. D_800F1198/119C/11A0 is a 3-column row table; bit 0 of column 0 is the list-terminator
flag (established s6 from the consumer func_800620B8's arithmetic). The copy loop directly
above writes column 0 as `*(s32 *)((u8 *)&D_800F1198 + ofs) = t;`, and all six sites in the
table's only consumer address the columns the same per-column symbol+byte-offset way. A
programmer clearing the terminator row — zero the two tail columns through the row pointer
already in hand, then write the flag column with the same expression the loop just used for
that column — writes exactly this text. Applying the rename test of
`.claude/rules/ordinary-c-judge-decidable.md` Ruling 1 sec.3: rename `row` to `q` and `ofs`
to `k` and the code still reads truthfully as "clear the three columns of the terminator
row"; no name and no statement exists to serve a codegen purpose rather than a semantic one.

## T3 GCC-internals justification
No GCC internal is offered as the justification for any construct, and the source text now
makes NO mechanism claim at all — the inline mechanism comment was deleted per the Judge's
instruction. The semantic justification is T2 and stands alone. For the ledger's benefit
(not as a defence of a construct): sessions s3–s9 established that a uniform all-row-pointer
epilogue compiles to 35 instructions and a uniform all-symbol-relative epilogue to 39, while
the target is 38 — which is WHY the mixed spelling is the truthful preimage rather than an
arbitrary preference. Ruling 1 sec.3 is explicit that a semantically-truthful spelling
chosen after observing codegen is not a FAIL ground.

## T4 permuter/search provenance
The distance-0 point was first surfaced by an s2 hand probe and independently re-found by
the s4 permuter campaign (~46k iterations, two fresh-seed basins), but it is not accepted on
search provenance: it is the form a reader derives from the target's own two address forms,
and s9 measured both uniform poles to show no single-spelling body can reach 38 insns. There
is no construct here that passes detectors "only because of its spelling" — there is no
no-semantic-purpose construct for a detector to have an opinion about.

## T5 family check
Walked the forbidden catalog line by line: not a register-asm pin, not hardcoded-`$N` asm,
not a scheduling barrier, not INLINE_MOVE_ALIASING, not volatile coercion in any spelling,
not an unused-local-array frame coercion, not a dead-param assign, not a dead conditional
store, not an empty-body `if`, not an `if (1)` wrap, not a dead-goto label pad, not a DImode
chain, not a goto-end accumulator, not a param-local alias declaration-order trick, not an
`s32 one = 1;` opaque variable, not a lowercase asm block, not build-time assembly
rewriting, not an `asm("sym")` alias rename, not a redundant width cast, not a `bb2.ld`
rodata reorder. The nearest forbidden neighbour by shape is "same-lvalue dual spelling" —
and it does not apply on the facts: row+0 is written exactly ONCE in the epilogue. `row[0]`
appears nowhere in the function; there is no pair of stores to one lvalue, no discard, and
no redundant write. Two different lvalues reached through two different expressions is not
dual spelling of one lvalue.
No sanctioned family is CLAIMED either — see the claims section — because none is needed: a
family claim is how a no-semantic-purpose construct is admitted, and this diff has none.

## T6 naming-announces-intent
Names are `i` (row count), `ofs` (byte offset into the table), `t` (the word being copied),
`row` (pointer to the terminator row). None of `pad`, `_pad`, `dummy`, `unused`, `spill`,
`sp_*`, `_buf`, `tail`, `slack`, `_frame_pad` appears. Every name denotes a value that is
read, and each survives renaming without the code losing its meaning.

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no no-semantic-purpose construct, so no
family entry is invoked and none is needed. The governing authority for adjudicating this
candidate is the mechanical checklist of `.claude/rules/ordinary-c-judge-decidable.md`
Ruling 1: (1) zero non-C mechanisms — confirmed, the diff is 100% ordinary compilable C;
(2) construct-class membership — vacuous, there is no such construct to place;
(3) the rename test — answered in T2/T6, every construct has a truthful semantic reading and
neutral names survive; (4) simplest-known-form — of the byte-exact forms known for this
function this is the only one at distance 0, and it carries zero no-semantic-purpose
constructs, so it is trivially the simplest.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. No FAKE marker is present anywhere in the
diff (the s10 submission's marker was deleted this session per the Judge's stated defect),
no family requiring a FAKE annotation is claimed, and the source's only comment is a plain
description of the function's behaviour that makes no rule, ruling, mechanism or exhaustion
claim.
