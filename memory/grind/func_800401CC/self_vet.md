# SELF-VET — func_800401CC

CONSTRUCTS: (1) the two packet-link statements written as SDK bitfield
struct field assignments through the project's OTag type (include/gpu.h),
with the two pointer locals retyped to OTag pointers; (2) one added
`#include "gpu.h"` line; (3) inline global reads plus store-last statement
order, carried over from the session-1 floor form; (4) parameter reuse for
the parity flag, carried over from the pre-grind form (the target clobbers
the incoming third argument immediately).

## T1 semantic purpose: Every construct is observable program logic. The
field assignments write the low 24 bits of the packet word and of the
ordering-table slot — the exact behavior of the function (linking the new
packet into the ordering table). Nothing in the diff is dead: no unread
locals, no dead stores, no constant holders, no address-of discards. The
diff removes the previously-present coercion construct rather than adding
one; the function body now contains zero carve-out constructs.

## T2 human-programmer: Yes, directly. This is the PsyQ SDK's own idiom:
LIBGPU's P_TAG is declared as a 24/8 bitfield struct and the SDK's addPrim
macro is precisely this two-statement shape. The project already ships the
identical spelling in src/gpu.c (AddPrim, gpu.c:347-350), accepted with
the OTag type on 2026-06-11 for the sibling ordering-table routines. A
programmer writing a GPU packet writer against the SDK would naturally
write field assignments, not open-coded arithmetic.

## T3 GCC-internals justification: The construct is justified by
provenance and semantics — the SDK declares this data as a bitfield
struct, the project type exists for it, and the sibling routines in gpu.c
ship the same spelling — not by compiler behavior. This session's forensic
dump documents WHY the spelling matches where arithmetic spellings did
not (the expansion's reference counts feed the allocator differently),
but that is diagnosis of the match, not the reason the code is written
this way: the notation is the faithful reconstruction of the original
declaration-level source shape.

## T4 permuter/search provenance: Not search-derived. No campaign ran this
session; the form came from recognizing the ordering-table-link idiom and
its existing project precedent. Prior sessions' campaigns never produced
this form.

## T5 family check: Matches no forbidden family, by letter or analogy: no
invented holder locals, no staging of constants through any local, no dead
or self-assigned stores, no alias renames, no volatile coercion, no inline
asm, no wrappers, no dead arrays. It also does not merely respell any
banned form: the banned forms all held mask constants in locals, while
this form contains no mask constants in the source at all. The pointer
retyping matches the accepted sibling routines (gpu.c) rather than
introducing a novel view of the data.

## T6 naming-announces-intent: No coercion-announcing names. Locals are
the record buffer, the record-table pointer, the two texture coordinates,
and the two retyped pointers, all pre-existing names from the prior
committed form; the only new identifier in the diff is the project type
name OTag.

SANCTIONED-FAMILY-CLAIMS: none — no carve-out family is invoked. Every
construct is ordinary program logic through an existing, previously
accepted project type; there is nothing in the diff that requires a
family or an exhaustion record to defend.

ANNOTATION-CONFORMANCE: n/a — the diff contains no construct requiring an
annotation; the previously-approved annotated assignment was removed along
with the rest of the arithmetic spelling, and no new annotation-bearing
construct replaced it.
