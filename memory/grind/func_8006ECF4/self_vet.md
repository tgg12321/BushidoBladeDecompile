# SELF-VET — func_8006ECF4 (completion)

CONSTRUCTS: none. The landed body is ordinary C and contains no dead stores,
unused locals, volatile tricks, register pins, inline assembly, or padding.

## T1 semantic purpose
Every declaration, statement, and branch represents behavior visible in the
original assembly. In particular, both `s.p0 = s0 + sel * 12` statements are
semantically required: the switch default uses one, while the other implements
the `i == 0` overwrite after explicit switch cases.

## T2 human-programmer
The body is a direct transcription of the loop, table selection, image upload,
shared pointer dispatch, and renderer call. The labels express the assembly's
real shared control-flow tails.

## T3 GCC-internals justification
No semantically inert construct was added for compiler behavior. The two
source-level `s.p0` assignments reach different real paths; GCC's later tail
merge merely restores the shared machine block present in the target.

## T4 permuter/search provenance
The rejected permuter dead-store family is not present. The landed index
staging uses consumed values (`b2` and `c12`) and was selected only after the
semantically incorrect score-11 chassis was replaced.

## T5 family check
No sanctioned fake family is claimed. The shared-label/goto topology and the
two path-required assignments are ordinary control-flow C.

## T6 naming announces intent
Names describe the represented values (`b2`, `c12`, `sel`, `rectbuf`) and no
identifier disguises a code-generation-only purpose.

SANCTIONED-FAMILY-CLAIMS: none.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct exists.

BYTE PROOF: `tools/wteng.ps1 main build` produced the oracle SHA1
`62efab4f73f992798c43e8c730aa43baa10bb4fa` on 2026-09-20 after moving the
intervening rodata objects into `text1b.o`.
