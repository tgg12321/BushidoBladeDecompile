# SELF-VET — func_80072CD4

Diff under vet: src/text1b.c:5865 — `INCLUDE_ASM("asm/funcs", func_80072CD4);` replaced by the
pure-C body of memory/grind/func_80072CD4/candidate.c (42 lines). Measured this session:
`sandbox func_80072CD4 --disable all` → score 0, build_insns 79 == target_insns 79,
rules_dropped 0 (tmp/grind/func_80072CD4/s5f2/sandbox_rgb_s5f2.json). No other build-pipeline
file is touched.

CONSTRUCTS: none

The body declares no variable of any kind — no local, no holder, no temporary, no alias, no
volatile, no `__asm__`, no `register ... asm()` pin, no scheduling barrier, no `do { } while (0)`,
no dead store, no self-assign, no unused declaration, no `(void)` discard, no goto, no annotation.
It is: two library calls (SetPolyG4, SetSemiTrans), an outer `if (arg0 < 4)`, an inner
`if (*(s32 *)((s32)(D_800A35C4) + 8) & 4)`, twenty-four byte field writes, an AddPrim call and a
return. Every statement in it is a live field write of the value the primitive displays on the
path it sits on. There is consequently no construct to classify and no exception family in play;
the six tests are answered below against the whole body.

## T1 semantic purpose: PASS. Every statement changes observable output. Each of the twenty-four
writes stores a colour component into the POLY_G4 primitive at arg1 (rgb0 = 0x04/0x05/0x06,
rgb1 = 0x0C/0x0D/0x0E, rgb2 = 0x14/0x15/0x16, rgb3 = 0x1C/0x1D/0x1E; the +0x24 return is the
primitive's size). Delete any one of them and the primitive is drawn with a different colour.
The two writes the historical constraint is about — `@0x04 = 0xFC` and `@0x0C = 0xFC` inside each
inner arm — are the RED components of rgb0 and rgb1 on that arm's path; they are consumed by the
GPU, not by the compiler. No statement is byte-neutral, none is a placeholder, none is dead.

## T2 human-programmer: PASS. Asked to write "vertex colours for a POLY_G4, two variants selected
by a flag", a human writes each variant's colour set as a block of complete triples in ascending
field order — which is literally what setRGB0/setRGB1 expand to. Nothing in the body would make a
reader ask "why is this here?": the answer for every line is "that is the colour of that vertex on
that path". The alternative shape (hoisting only the two red components out of the colour
assignments into a shared tail behind an `int fc_const` holder — memory/grind/func_80072CD4/
fallback_floor4.c) is the one a reader would question, and it is strictly the more artificial of
the two.

## T3 GCC-internals justification: PASS — no GCC internal is the justification for anything in the
body. The reason each arm writes its own complete rgb0/rgb1 triple is the primitive's field
layout, not a pass. This session's dump work (below, and in the candidate header) is offered ONLY
to rebut the opposite claim — that the per-arm placement was chosen to steer jump2 — and it rebuts
it: the source order inside each arm is canonical ascending 4,5,6,C,D,E, while the emitted merge
head is 4,C,E. The re-ordering is manufactured by sched2 (sched.c `schedule_block`, bottom-up:
producer-less stores sink to the arm tail, identically and independently in both arms — THEN insns
40/55/65, ELSE insns 75/90/100 in tmp/grind/func_80072CD4/dumps/text1b.sched2), and jump2 then
cross-jumps that already-scheduled common tail behind a new join label (code_label 223 / label 872
in text1b.jump2). No statement ordering available at the C level selects that order, so the
placement cannot have been, and was not, a merge-order lever. Remove the mechanism paragraph
entirely and the body is unchanged and still justified by the field layout.

## T4 permuter/search provenance: PASS. The body did not come from a search. Sessions s1–s5 ran
structural variants, two random-permuter chassis and a 15,825-iteration directed PERM_LINESWAP,
and all of them missed it, because all of them modelled arg1 as an opaque byte blob and searched
ORDERINGS of independent stores. The body came from reading the offsets as the libgpu POLY_G4
colour layout and from the COMPLETED-C sibling func_80072BC4 in the same file, which is already
written in that field order. It is a spelling derived from the data structure, not a permutation
that happened to score.

## T5 family check: PASS — no family is matched, by analogy or otherwise, because there is no
construct. Specifically checked against the two nearest historical items for this function:
(a) rejected/dup4_0xc_into_arms.c — an `int fc_const` holder plus mid-arm injection of
`@4 = fc_const; @0xC = fc_const` in a merge-order-driven sequence (5,6,D,4,C,E), self-annotated by
its author as duplicated for jump2's merge order: that construct is BANNED and is not present here
in any spelling — there is no holder, no injected statement, no non-canonical ordering, and the
red writes are the arm's own colour data, not copies of a shared temporary;
(b) the layer-1 concern that the body "re-spells" (a) — answered on the record by the Judge ruling
of 2026-08-20 06:09, docs/grind/decisions.md:8456 (on main as a8d7ee5f), verbatim: "The banned
lever is the fc_const-holder mid-arm injection (rejected/dup4_0xc_into_arms.c) and stays banned;
that ban does not reach a body with no holder and no hoist." That ruling is a Judge disposition on
this exact body, filed by a separate session and committed before this one began; this session is
not self-approving anything and does not rely on the 2026-08-20 05:46 entry, which the driver has
listed as a banned citation and which is not cited here or in candidate.c.

## T6 naming-announces-intent: PASS. There are no names to audit — the body introduces no
identifier at all. The only symbols it references are the pre-existing externs SetPolyG4,
SetSemiTrans, AddPrim, D_800A35C4, D_800A374C and its own parameters arg0/arg1.

SANCTIONED-FAMILY-CLAIMS: none

No exception family is claimed or spent, because the diff contains no construct requiring one.
Supporting in-repo precedent for the SPELLING (offered as corroboration, not as a family grant):
the COMPLETED-C sibling func_80072BC4 carries the same un-hoisted cross-arm duplicate store
`*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;` in both arms at src/text1b.c:5840 and src/text1b.c:5843,
with zero rules and absent from engine/queue.json.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
