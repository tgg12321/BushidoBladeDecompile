# SELF-VET — func_80072CD4
(Written for the s5b byte-matching body banked in memory/grind/func_80072CD4/candidate.c.
The session outcome is `ruling-request`, NOT `candidate-ready`: see the last section.)

CONSTRUCTS: none — the body declares no variable of any kind. It consists only of two library
calls (SetPolyG4 / SetSemiTrans), one `if (arg0 < 4)` on the parameter, one inner `if` on a
global flag bit, twenty-four `*(u8 *)((s32)(arg1) + N) = <literal>;` field writes, one AddPrim
call and one `return`. No local, no holder, no volatile, no `__asm__`, no register pin, no
barrier, no do-while, no dead store, no self-assign, no unused declaration, no goto, no
annotation, no alias, no cast widening.

## T1 semantic purpose: every statement is a live field write executed on its own control-flow
path, with the value the game actually displays for that case. Deleting any one of them changes
what the primitive renders. The two `= 0xFC` writes to offsets 0x04 and 0x0C appear once in each
inner arm because the red component of the POLY_G4 rgb0 and rgb1 vertex colours is 0xFC on both
branches; each copy executes on its own path and stores a byte the hardware reads. Nothing in
the body is dead in the output or dead at runtime.

## T2 human-programmer: yes, and demonstrably so. arg1 is a PSX libgpu POLY_G4 primitive; the
offsets are its four vertex-colour triples (rgb0 = 0x04/0x05/0x06, rgb1 = 0x0C/0x0D/0x0E,
rgb2 = 0x14/0x15/0x16, rgb3 = 0x1C/0x1D/0x1E — the setRGB0..setRGB3 field groups). A programmer
given "set the four vertex colours, with rgb0/rgb1 depending on a flag" writes each branch's
complete colour pair, which is what this body does. The file's own COMPLETED-C sibling
func_80072BC4 (src/text1b.c:5822) is written in exactly this field order. Nothing in the body
reads as "why is this here?".

## T3 GCC-internals justification: the body is justified by the primitive's field layout, not by
a compiler pass. No pass name is load-bearing for the SHAPE of the code. (The ledger does record
what the compiler then does with it — jump2 tail-merges the arms' common tail at the join label —
but that is an explanation of the observed bytes after the fact, not the reason any statement is
written the way it is; remove the compiler from the picture and the body is still the natural
spelling.)

## T4 permuter/search provenance: none. This form was not produced by the permuter and is not
reachable from the s4/s4b search chassis (both of those searched orderings of a body that had
already lifted the red components into a shared tail). It came from reading the offsets as the
libgpu POLY_G4 colour layout and mirroring the matched sibling's field order.

## T5 family check: no sanctioned family is claimed, because no construct is present to classify.
The one shape that invites a family question is that the arms each contain a `= 0xFC` write to
0x04 and 0x0C. That is not the dead-store family (both copies are live), not the constant-holder
family (no holder exists), not the variable-reuse or named-intermediate families (no local
exists), and not a barrier/coercion of any kind. Whether it is nonetheless caught by the standing
2026-07-24 judge constraint on duplicated-into-arms store-schedule levers is exactly the question
this session escalates rather than self-answering — see below.

## T6 naming-announces-intent: no names exist in the body beyond the two parameters `arg0`/`arg1`
inherited from the surrounding file's convention. Nothing named pad/dummy/unused/spill/tail/slack.

SANCTIONED-FAMILY-CLAIMS: none.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no construct of any kind; see CONSTRUCTS).

## WHY THIS SESSION RETURNS ruling-request RATHER THAN candidate-ready
The binding judge constraint of 2026-07-24 16:38 says: do NOT respell the @4/@0xC common-tail
stores "(or any unconditional common-tail statement) as a duplicated-into-arms store-schedule
lever". This body does place `= 0xFC` writes to 0x04 and 0x0C inside both arms, so it is
textually within reach of that wording, even though it contains no lever and no construct and
was arrived at from the field layout rather than from the schedule. Submitting it as
candidate-ready would be self-approval against a binding constraint. The precise question is in
the outcome JSON's ruling_question.
