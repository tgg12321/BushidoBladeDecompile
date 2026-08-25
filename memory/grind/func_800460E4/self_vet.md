# SELF-VET — func_800460E4

STATUS: PREPARED, NOT SUBMITTED. Session s4b (2026-08-25, permuter) measured
`sandbox func_800460E4 --disable all` = 0 (248/248, rules_dropped=10,
cheat_asm_stripped=0) with the body in memory/grind/func_800460E4/candidate.c,
then returned `ruling-request` rather than `candidate-ready` because construct
(3) below cannot honestly claim a sanctioned family. This vet is written out in
full so the session that carries the ruling can submit without re-deriving it.
If the ruling goes AGAINST construct (3), delete this file's construct-(3)
section along with the construct.

CONSTRUCTS: (1) pre-switch de-aliasing — the `p` / `p2` param-alias locals are
gone and the arg1 header word is read as `*(s32 *)arg1` / `((s32 *)arg1)[s3]`,
and `a0_ptr` is spelled `(s32 *)((s3 << 2) + (s32)s0)`; (2) FAKE-annotated
combine-foldable chain-extender on the default init of `s1`; (3) case 3's two
stage-header words staged in two FRESH locals `hidx` / `lidx` that are each
written TWICE — first the raw byte offset loaded from the header, then the same
variable refined in place to a word index (`hidx >>= 2;`) and consumed as
`s6 = &s0[hidx]`.

## T1 semantic purpose
(1) Yes — removing an alias local is a pure simplification; the reads are the
same reads. (2) No independent semantic purpose: it is a FAKE construct and is
declared as one; it folds to `s1 = s4` with zero emitted bytes. (3) Every
statement computes a value that is read by the next: the two loads are the
header words the function needs, the two shifts convert a byte offset to the
word index that indexes `s0`, and both indices are dereferenced. Nothing is
dead. What construct (3) does NOT have is a semantic difference from the
one-statement spelling `s6 = &s0[(u32)s0[s3 - 2] >> 2]` — that spelling is
byte-different (diffs=9) and this one is byte-exact, so the honest answer is
that the SPLIT, specifically, is byte-motivated even though every statement in
it is real. That is the exact point the ruling must decide.

## T2 human-programmer
(1) Yes — fewer names, same reads. (2) No; it is FAKE-annotated. (3) Partly.
"Read the offset, convert it to a word index, index the table" is an ordinary
two-step a human writes, and the sibling function twelve lines below in the same
file (func_800464C4, src/text1a_c2.c) already reuses one scalar `v0` to hold
successive raw header offsets across switch arms — multiply-assigned offset
scratch is this file's shipped idiom. But a reader could still ask why case 3
spells the alignment longhand while case 13, reading the SAME two words, uses
the `ALIGN4` macro. The answer is measured, not stylistic: applying the staged
spelling at case 13 too takes case 13 from matching to diffs=13 (probe8 z3), so
the divergence is byte-forced. A reviewer is entitled to weigh that as the T2
cheat smell.

## T3 GCC-internals justification
(1) No — this is a readability change, no pass is cited for it. (2) Yes, and
that is disclosed in its annotation (flow.c reg_n_refs → global.c
allocno_compare); it is a FAKE construct in the sanctioned F1 family, ruled
legitimate for this function by the 2026-08-25 03:53 layer-1 review. (3) YES,
and I am not hiding it: the mechanism is sched.c `adjust_priority` →
`birthing_insn_p`, whose boost applies only when `reg_n_sets[regno] == 1`;
writing each carrier twice clears the boost so exactly one boosted insn is ready
at block 19's reverse-cycle T-6 and both header loads issue adjacently. This is
a cheat SIGNAL under test 3 and it is why this vet does not conclude in a
submission. It is also, verbatim, the mechanism the project already sanctioned
in `.claude/rules/staged-value-reused-variable.md` — the question is whether
that family's bound 2 (borrow an EXISTING local) can be met here; measured
answer: no (probe10, existing-local carriers stop at diffs=8 / 40).

## T4 permuter/search provenance
(1) and (2) predate this session. (3) was FOUND by decomp-permuter (campaign
perm_d, output-95-1) and then generalised and re-derived by hand from the
sched.c mechanism, with the full spelling map measured (probe4/5/6/7/9/10 —
22 once-written spellings at 9, three existing-local spellings at 8/40, six
twice-written spellings at 0). It is not "passing detectors because the
detectors miss this spelling" — the vet names the mechanism explicitly. But
"found by search first" is true and is disclosed.

## T5 family check
(1) No family needed — ordinary C. (2) F1 combine-foldable chain-extender,
sanctioned by owner ruling 2026-07-01 and FAKE-annotated in place. (3) NO
FAMILY COVERS IT. named-intermediate requires once-written/once-read (this is
twice-written); staged-value-reused-variable requires borrowing a local the
function already has for another job (these are fresh). It is not a member of
any forbidden family either — no dead code, no pin, no asm, no volatile, no
alias rename, no address-form respelling, no cast trick. It is a genuine gap,
which is why the outcome is `ruling-request`.

## T6 naming-announces-intent
No construct is named `pad`/`dummy`/`unused`/`spill`/`tail`/`slack`. `hidx` /
`lidx` name what they hold (a word index into the stage header). Every one of
them is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: F1 combine-foldable chain-extender (construct 2 only)
  SCOPE: "The combine-foldable chain-extender to bump `reg_n_refs` was moved to the sanctioned F1 family by owner ruling 2026-07-01 — FAKE-annotated last-resort; un-annotated or byte-materializing instances remain FAIL."
  PRECEDENT: docs/grind/decisions.md:10710
  (Construct (3) claims NO family — see T5. Construct (1) claims none and needs none.)

ANNOTATION-CONFORMANCE:
  /* FAKE: live default init of s1 routed through a delta-rebase detour that combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare priority above the s2 pointer so allocation order matches target, lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */
  /* FAKE: the two stage-header words are staged in hidx/lidx as raw byte offsets and then refined in place to word indices, mechanism: GCC 2.7.2 sched.c adjust_priority -> birthing_insn_p (reg_n_sets[regno] == 1); the second write clears the birthing boost on both carriers so only ONE boosted insn is ready at block 19's reverse-cycle T-6 and both header loads issue adjacently, as in target, lever-exhaustion: memory/grind/func_800460E4/evidence.md [s1]-[s4b] */
  Both carry what + mechanism + lever-exhaustion. The second annotation is
  written but its FAMILY is unclaimed — an annotation is not a licence.
