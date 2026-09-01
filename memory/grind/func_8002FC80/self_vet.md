# SELF-VET — func_8002FC80  (session s2, 2026-09-01, structural modality)

Diff scope: `src/code6cac_b.c` only — the single line `INCLUDE_ASM("asm/funcs",
func_8002FC80);` replaced by a 71-line C body. No other file in the build surface is
touched. Measured this session with the edits in place:
`sandbox func_8002FC80 --disable all` → **score 0, target_insns 74, build_insns 74,
rules_dropped 0, cheat_asm_stripped 37**; `verify-oracle` → ok, build_sha1
`62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, build_matches true.

CONSTRUCTS: (1) parameters typed `u8 *a0, *a1, *a2` (untyped record bases) with
widening reads `*(s32 *)(a1 + 4)`; (2) six plain fixed-address scratchpad stores
`*(s32 *)0x1F8003xx = v1 - v2;`; (3) a named pointer local `s32 *p` holding the GTE
output-vector address 0x1F800380, used as the store macro's register operand and then
read as p[0]/p[1]/p[2]; (4) three cop2/GTE inline-asm islands transcribing the PsyQ
libgte macros gte_SetRotMatrix / gte_ldlvl / gte_stlvnl plus the raw OP command word;
(5) ordinary scalar locals v1, v2, ret.

## T1 semantic purpose
(1) Each read produces one of the nine coordinate words the function subtracts; the
`(s32 *)` cast is *required* by the type system — dereferencing a `u8 *` without it
reads one byte, not the word, so removing the construct changes the program's meaning,
not just its bytes. (2) Each store is the function's observable output: the two
difference vectors the GTE preamble then consumes. (3) `p` names the address the GTE
store macro writes and the three MAC components are read back from; all four uses are
live reads of a real value. (4) The islands compute the cross product and load/store
cop2 registers — no C form exists for cop2 (project policy, `inline-asm-allowed`).
(5) v1/v2 carry the two operands of each subtraction; ret carries the returned angle.
Nothing in the diff is dead, discarded, written-never-read, or removable without
changing what the function computes.

## T2 human-programmer test
A programmer given only the spec ("subtract the base point from the other two, stage
both differences in the scratchpad the GTE macros use, take the cross product, return
its atan2 with a 180° flip when MAC2 is positive") writes exactly this: three record
pointers, nine word reads, six subtract-and-store steps, the PsyQ macro sequence, and
one named pointer to the macro's output vector. No line invites the question "why is
this here?" on semantic grounds. The `u8 *` signature is what an author writes for a
record base of unproven layout — this file's own accepted code does it
(`src/code6cac_b.c:860` `u8 *obj`, read at `:896` / `:898` as `*(s32 *)(obj + 0x100)`).

## T3 GCC-internals justification test
No construct in this diff is justified by a compiler pass. The reasoning for every line
above is program logic (what value is produced, what the hardware macro requires).
Prior sessions on this function DID reason from `sched.c` — that reasoning is not what
this body rests on and is not reproduced here. The one codegen fact I record is a
negative measurement kept as ledger data, not as a justification: deleting the named
pointer `p` and repeating the literal address at each of its four uses measures 5 @
75/74 (`tmp/grind/func_8002FC80/s2/sandbox_no_p_local.txt`,
`rejected/s2_no_named_output_pointer.c`). I am not claiming that measurement as the
*reason* `p` exists — `p` exists because the value has four uses and one meaning.

## T4 permuter / search provenance
None. No permuter run contributed to this body. Every element was written by hand from
the function's semantics and from the accepted spelling its byte-identical sibling
`func_8002FDB0` ships in this same file for the same six scratchpad slots. The one
change this session made over the inherited form was a *simplification* (uniform
p[0]/p[1]/p[2] reads replacing two repeated literal addresses), not a search find.

## T5 family check
I checked the diff line by line against the driver's ban list for this function and
against the forbidden-family catalog. There is no register pin, no numbered-register
asm injection, no barrier, no volatile anywhere, no alias rename, no unused local, no
array, no self-assign, no dead conditional, no always-true wrapper, no goto pad, no
DImode chain, no opaque constant, no build-time rewriting, and no redundant width cast
(each cast changes the accessed width and is required to read a word).

Of the three bans standing for this function: the whole-function assembly transcription
is **absent** — the head here is C and only the cop2 macro islands are assembly; the
misuse of the cluster grant to cover a whole body is **absent** — the grant at
`inline_asm_canonical.txt:365` is cited only for the four cop2 islands, exactly as its
sibling entry covers `func_8002D320`'s islands at `src/code6cac_b.c:867/876`; and the
third ban (discarding a proven body over a spelling objection) is honoured — this
session kept the proven body and refined its spelling rather than abandoning it.

The two spelling classes that were previously banned were cleared by the owner ruling
of 2026-08-31 (`ordinary-c-judge-decidable`, commit `73bee8f8`; migration entry
`docs/grind/decisions.md:17002`), which returns this item to active and delegates the
spelling choice to the Judge on semantic-truthfulness and simplest-known-form grounds.
This body is the load-side class in its simplest measured spelling: a single required
cast per read on an untyped base, versus the previously-tried double cast over an
already-typed pointer. The aggregate-typed store class is not used at all.

## T6 naming-announces-intent
Names in the diff: `a0`/`a1`/`a2` (splat-convention parameter names), `v1`/`v2` (the
two subtraction operands), `p` (the GTE output vector), `ret` (the return value). None
is a pad/dummy/unused/spill/slack-class name, and every one has live reads.

SANCTIONED-FAMILY-CLAIMS: none. Every construct is ordinary C whose meaning is the
reason it is written, adjudicated under the amended policy in
`.claude/rules/ordinary-c-judge-decidable.md`. No frozen-list family is invoked and
none is needed. The cop2 assembly islands are not a family claim either — they run on
the standing canonical-asm grant recorded at `inline_asm_canonical.txt:365` and
`tools/grinder/owner_cluster_grants.txt:18`, and are character-for-character the same
macro transcriptions the already-accepted `func_8002FDB0` and `func_8002D320` ship in
this file.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff is a coercion,
so no `/* FAKE: */` annotation is present or required. The in-body comments are
semantic documentation only (what the scratchpad layout is, which PsyQ macro each
island transcribes, what the return value means) and assert no family, no criterion,
and no codegen motive.
