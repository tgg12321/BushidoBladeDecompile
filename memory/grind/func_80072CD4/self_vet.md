# SELF-VET — func_80072CD4

Written by grind session s6 (synthesis modality, 2026-08-20) against the diff actually in
src/text1b.c right now: `INCLUDE_ASM("asm/funcs", func_80072CD4);` at src/text1b.c:5865 replaced
by the body banked in memory/grind/func_80072CD4/candidate.c. Nothing else in the tree is touched.

MEASUREMENT: `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all` = **score 0**,
build_insns 79 == target_insns 79, scorable true, rules_dropped 0. Re-measured THIS session on the
current chassis with the edit in place; artifact tmp/grind/func_80072CD4/s6/sandbox_s6_reverify.json.

CLASSIFICATION STATUS: the one open question this body ever carried — whether the standing
2026-07-24 16:38 judge constraint on duplicated-into-arms store-schedule levers reaches it — was
ruled on and **PASSED** on 2026-08-20 05:46 (docs/grind/decisions.md:8448). The ruling: "The
2026-07-24 constraint does NOT reach candidate.c. It banned a LEVER ... candidate.c has no local,
no holder, no annotation, no intent-named symbol ... Declining to hoist a common live store is
ordinary C, not an exception family." That ruling is committed on main (6e80ffdf). This vet is
therefore a candidate-ready vet, not a ruling request.

CONSTRUCTS: none — the body declares no variable of any kind. It consists only of two library
calls (SetPolyG4 / SetSemiTrans), one `if (arg0 < 4)` on the parameter, one inner `if` on a
global flag bit, twenty-four `*(u8 *)((s32)(arg1) + N) = <literal>;` field writes, one AddPrim
call and one `return`. No local, no holder, no volatile, no `__asm__`, no register pin, no
barrier, no do-while, no dead store, no self-assign, no unused declaration, no goto, no
annotation, no alias, no cast widening, no `asm("sym")` rename, no type-level volatile.

## T1 semantic purpose: every statement is a live field write executed on its own control-flow
path, with the value the game actually displays for that case. Deleting any one of them changes
what the primitive renders. The two `= 0xFC` writes to offsets 0x04 and 0x0C appear once in each
inner arm because the red component of the POLY_G4 rgb0 and rgb1 vertex colours is 0xFC on both
branches; each copy executes on its own path and stores a byte the hardware reads. Nothing in
the body is dead in the output or dead at runtime. Removing the construct is not even definable
here — there is no construct to remove, only the choice of whether to hoist a store that is live
on both paths, and both spellings have identical observable behaviour by construction.

## T2 human-programmer: yes, and demonstrably so. arg1 is a PSX libgpu POLY_G4 primitive; the
offsets are its four vertex-colour triples (rgb0 = 0x04/0x05/0x06, rgb1 = 0x0C/0x0D/0x0E,
rgb2 = 0x14/0x15/0x16, rgb3 = 0x1C/0x1D/0x1E — the setRGB0..setRGB3 field groups). A programmer
given "set the four vertex colours, with rgb0/rgb1 depending on a flag" writes each branch's
complete colour pair, which is what this body does. The file's own COMPLETED-C sibling
func_80072BC4 (src/text1b.c:5822) is written in exactly this field order and carries the same
shape of un-hoisted cross-arm duplicate (`*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;` at
src/text1b.c:5840 and :5843). Nothing in the body reads as "why is this here?".

## T3 GCC-internals justification: the body is justified by the primitive's field layout, not by
a compiler pass. No pass name is load-bearing for the SHAPE of the code. The ledger does record
what the compiler then does with it — jump2 tail-merges the arms' common tail at the join label —
but that is an after-the-fact explanation of the observed bytes, not the reason any statement is
written the way it is. Remove the compiler from the picture and the body is still the natural
spelling; indeed it is strictly the LESS compiler-aware of the two candidates, since the rejected
floor-4 alternative is the one whose shape was chosen to steer the merge block.

## T4 permuter/search provenance: none. This form was not produced by the permuter and was not
reachable from the s4/s4b search chassis (both of those searched orderings of a body that had
already lifted the red components into a shared tail). It came from reading the offsets as the
libgpu POLY_G4 colour layout and mirroring the matched sibling's field order.

## T5 family check: no sanctioned family is claimed, because no construct is present to classify.
The one shape that invites a family question is that the arms each contain a `= 0xFC` write to
0x04 and 0x0C. That is not the dead-store family (both copies are live), not the constant-holder
family (no holder exists), not the variable-reuse or named-intermediate families (no local
exists), not duplicated-statement-into-arms as a lever (nothing was moved INTO the arms — the
arms were written whole and nothing was ever hoisted out), and not a barrier or coercion of any
kind. Whether the standing 2026-07-24 constraint nonetheless reached it was escalated rather than
self-answered by session s5b, and the ruling of 2026-08-20 05:46 (docs/grind/decisions.md:8448)
is **PASS**: the constraint does not reach a body with no lever construct.

## T6 naming-announces-intent: no names exist in the body beyond the two parameters `arg0`/`arg1`
inherited from the surrounding file's convention. Nothing named pad/dummy/unused/spill/tail/slack,
and no symbol carries an intent name.

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no construct requiring a family, so no scope
sentence or exception precedent is being spent. (Ordinary-C precedent for the spelling, offered as
corroboration only and not as a family claim: src/text1b.c:5840 and src/text1b.c:5843, the
COMPLETED-C sibling func_80072BC4's own un-hoisted cross-arm duplicate store.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no construct of any kind; see CONSTRUCTS).
