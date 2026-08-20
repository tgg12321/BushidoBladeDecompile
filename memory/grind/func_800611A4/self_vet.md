# SELF-VET — func_800611A4

CONSTRUCTS: `s32 *v1 = (s32 *) (&D_800F116C);` (pointer local to a global, dereferenced and
its value stored to D_800A3468); `s32 *p;` + `p = arg0;` + `*p++` (ordinary walking pointer
over the caller-supplied array); `u16 svec[3]` (stack buffer written then handed to the callee
via D_800F117C); byte-offset casts on the three halfword param reads. No inline asm, no
register pins, no volatile, no dead stores, no self-assigns, no constant-holder locals, no
`do {} while (0)`, no `/* FAKE */` construct of any kind.

## T1 semantic purpose: Every construct carries a real, observable value.
- `v1` holds the address of D_800F116C; that address is BOTH stored to the global D_800A3468
  and dereferenced (`*v1 = 0x21001A`). Two genuine uses of one computed address. Removing it
  does not leave the program equivalent-but-simpler — it duplicates the address computation.
- `p` walks arg0 and produces the three loaded words that are stored to D_800F1140/1144/1148.
  Every increment feeds the next load. Delete `p` and the function loses its three loads.
- `svec[3]` receives the three halfwords read from arg1 and its address is published to
  D_800F117C, which func_80060A68 reads. It is a real output buffer, not a frame pad.
- The byte-offset casts are how the source reads halfwords at +0/+2/+4 of a s32* param.
None of these is byte-neutral scaffolding; each has a consumer.

## T2 human-programmer: Yes, and demonstrably so — this is verbatim the idiom the two
ALREADY-MATCHED, already-accepted siblings in the very same source file use:
`func_8006133C` (src/text1b.c:3184) and `func_8006156C` (src/text1b.c:3248) both read
`p = a0; D_800F1140 = *p++; D_800F1144 = *p++; D_800F1148 = *p; D_800A3464 = <mask>;`.
A reader asks "why is this here?" about none of it: seed a cursor, copy three words, set a
mask word. It is the plainest possible spelling of the function's job.

## T3 GCC-internals justification: No. The reasoning for choosing this shape is
"write it the way its matched siblings in the same file are written" — a program-logic /
prior-art argument, not a pass-internals argument. No allocator, scheduler, DCE, LUID,
reg_n_refs, reg_n_deaths, allocno-priority or reorg.c mechanism is invoked to justify any
construct's PRESENCE. (The candidate.c header explains AFTER THE FACT why the previous
sessions' reused-temp decomposition allocated differently; that is a post-hoc explanation
of a dead search path, not the reason any construct here exists.)

## T4 permuter/search provenance: None. No permuter, no auto-search, no randomized
mutation produced any part of this form. It was derived by reading the two matched sibling
functions in src/text1b.c and transplanting their post-call idiom (the mandated `rederive`
modality: sibling transplant).

## T5 family check: The diff matches NO forbidden family — not by analogy either. It contains
no register pin, no hardcoded-$N asm, no regfix/asmfix rule, no scheduling barrier, no
volatile of any kind, no alias rename (`asm("Sym")`), no dead store or self-assign, no dead
conditional / empty-if / if(1) wrapper, no dead goto or label pad, no DImode chain, no
opaque constant holder, no unused local array, no redundant width cast, no linker-script
reorder. Because there is no coercion construct at all, no sanctioned-family exception is
being claimed or needed.

## T6 naming-announces-intent: No name in the diff is `pad`/`dummy`/`unused`/`spill`/`sp_*`/
`_buf`/`tail`/`slack`. `p` is a cursor, `svec` is the 3-halfword buffer, `v1` is the
carried-over sibling-idiom name for the D_800F116C pointer (same identifier the matched
siblings func_8006133C / func_8006156C / func_800613C8 use). Every one of them has real
uses; none is address-of-only, discard-only, or unused. (The previous session's `sp` array
name and its `new_var` staged temp were both REMOVED this session — measured score-neutral.)

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no construct from any sanctioned or
forbidden coercion family. It is ordinary C.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
