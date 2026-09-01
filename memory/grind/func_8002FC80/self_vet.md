# SELF-VET — func_8002FC80

CONSTRUCTS: VECTOR-typed parameters whose components are read as ->vx / ->vy / ->vz; six member
stores through a VECTOR pointer at the two fixed scratchpad addresses 0x1F800360 and 0x1F800370;
one named VECTOR pointer local bound to 0x1F800380 and referenced four times; four cop2 inline-asm
islands character-for-character identical to the already-authorized sibling at src/code6cac_b.c:1311;
one added header include (include/gte.h) supplying the VECTOR type.

## T1 semantic purpose

- **VECTOR-typed parameters, member reads.** Yes, observable purpose independent of codegen. The
  function's three arguments are 3-D points; it forms two difference vectors from them, hands those
  to the GTE, and returns an angle. `VECTOR` (include/gte.h:25) is precisely the PsyQ type for a
  32-bit 3-component point, and it is the type the four cop2 macros in this body consume. Declaring
  the parameters as that type is a description of the data, and it removes casts rather than adding
  them: there is not a single cast on the read side of this body.
- **Member stores at the two fixed scratchpad addresses.** Yes. The two destinations are the
  scratchpad slots the very next statements read back as vectors — the gte_SetRotMatrix island reads
  three consecutive words at 0x1F800360 and the gte_ldlvl island reads three consecutive words at
  0x1F800370. Naming those two 3-word slots with the same type they are consumed as is the truthful
  description of the layout. A cast is unavoidable when the destination is a fixed hardware address;
  the only question is which type it names, and the type the hardware macros demand is the honest
  answer.
- **The named pointer local for 0x1F800380.** Yes, and it is load-bearing: the address has four uses
  (the gte_stlvnl operand plus the three component reads) and one meaning (the GTE output vector).
  Removing it is not byte-neutral — measured 76 instructions against the target's 75, with the
  address re-materialized for the ratan2 argument (banked at
  rejected/s2r3_vector_params_no_p_local_76insns.c). So it is not a discardable holder.
- **The four cop2 islands.** Yes — cop2 has no C analogue; these are the SDK macro bodies, byte-
  verified against the target and identical to the sibling function already shipping on main.
- **The include.** Mechanically required: the translation unit did not previously reference VECTOR.

## T2 human-programmer

Yes to all of it. Given only the specification — "subtract point a0 from points a1 and a2, put the
two differences in the scratchpad slots the GTE macros read, cross them, return the angle of the
result with 180 degrees added when its Y component is positive" — a PsyQ programmer writes exactly
this: VECTOR in, VECTOR out, `->vx/->vy/->vz` throughout, the SDK macros unchanged. Nothing in the
body invites the question "why is this here?". The construct a reader would actually stumble on is
the *alternative*: describing three 3-D points as loose integer arrays and then casting them back.

## T3 GCC-internals justification

Stated in full, because the honest answer has two halves and omitting either would be a
misrepresentation.

(a) **What the compiler analysis established, and it is not hidden.** This function's failure mode is
documented in this project's own ledger: with integer-array-typed operands, `true_dependence`
(tools/gcc-2.7.2/sched.c:812-840) discards the store-to-load dependence whenever exactly one side of
the pair is `MEM_IN_STRUCT_P` at a varying address and the other is a plain memref at a fixed
address, and sched1 then sinks all six stores past all twelve loads. That is why the integer-array
spelling measures 34 and not 0. When both the sources and the destinations are struct-typed, neither
exemption clause's `! MEM_IN_STRUCT_P (...)` conjunct can hold, the dependence survives, and the
six blocks stay in source order. I read that out of the compiler source this session rather than
guessing it, and it is written up in evidence.md.

(b) **Why that is nevertheless not the justification for the construct, which is the test this
checklist actually asks.** The mechanism above is an *explanation of a past failure*, not the reason
this body is spelled the way it is. The reason is the program logic: these five quantities are 3-D
vectors, and the body says so. The distinguishing evidence is that the mechanism is satisfied here
as a side effect of uniform honest typing, not by an asymmetry aimed at the flag — there is no
construct in this diff that exists on one side of an assignment but not the other, no cast that has
no semantic reading, and nothing that would be deleted if the scheduler behaved differently. The
`p` local is the one construct whose *position* was tuned by measurement (assigning it at its use
site rather than with the other declarations), and that is an ordinary statement-placement choice a
programmer makes freely — the statement is real, it is consumed immediately by the island below it,
and no reader would ask why a pointer is initialized on the line before its first use.

## T4 permuter/search provenance

This session ran in permuter modality but launched no randomized campaign; none is running now. The
prior run had already exhausted the integer-array-parameter basin with a full campaign (four
distinct proposals, all banked in rejected/), and fresh-seed discipline calls for a structurally
different chassis rather than a re-seed. The structurally different chassis was the type system, and
it was chosen by reasoning about what the data IS, not by sampling. The form was then confirmed by
direct measurement (objdump-identical at 75/75, then sandbox 0 at 74/74 in src, then oracle SHA1).
Nothing here is a construct that "only a search would find" — the search tooling was used as a
differ, and the winning shape is the one that reads most naturally.

## T5 family check

No forbidden family, and specifically not by analogy:

- This is not a scheduling barrier: nothing is inserted between statements, no ordering primitive is
  present, and deleting any construct here changes what the function *means*, not merely when things
  are emitted.
- This is not volatile coercion: there is no volatile in the diff outside the four cop2 asm islands,
  where it is the SDK's own spelling.
- This is not a dead store, constant holder, dead local, dead array, alias rename, register pin,
  hardcoded-register asm injection, or a redundant width cast. Every declaration is read; every
  store is read back by the island immediately following it; every read feeds the return value.
- It is not the whole-body assembly form that is banned for this function, and it does not cite any
  grant as authorizing one: the body is C, and the only assembly is the four cop2 islands whose
  legitimacy the layer-1 reviewer has already affirmed in writing.
- Nor is it a respelling of either construct the reviewer previously FAILed. Both of those turned on
  a type appearing on ONE side of the six assignments while the other side stayed an integer array;
  that asymmetry was the whole basis of the finding. Here the typing is uniform across every
  3-component quantity the function touches, the read side carries no cast at all, and the resulting
  body is shorter and plainer than either predecessor. If the reviewer disagrees and reads uniform
  vector typing as the same thing, that is a ruling I will take — but it is a different diff, not a
  different spelling of the same one.

## T6 naming-announces-intent

No name in the diff announces coercion intent. The identifiers are `a0`/`a1`/`a2` (the pre-existing
auto-generated parameter names, unchanged), `p` (the GTE output vector, four uses), and `ret`. There
is no `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, or `_buf`. No declaration in the body is
write-only, address-of-only, or discarded.

SANCTIONED-FAMILY-CLAIMS: none — this diff is ordinary C plus the four already-legitimate cop2
islands, so no frozen-list exception is claimed or needed.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
