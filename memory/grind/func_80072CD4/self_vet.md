# SELF-VET — func_80072CD4 (session s17, 2026-09-04)

The body in src/text1b.c is byte-identical (comments/whitespace aside) to
memory/grind/func_80072CD4/candidate.c and to
tmp/grind/func_80072CD4/s16/A2_dup_arms_ascending.c, which carries a Judge PASS
clearance dated 2026-09-04 01:32 (docs/grind/decisions.md:22248). Per the driver's
body-keyed review rule this body was submitted EXACTLY as cleared; nothing was
respelled. Measured this session with the body in place:
`sandbox func_80072CD4 --disable all` = 0, build_insns 79 == target_insns 79,
rules_dropped 0; `verify-oracle` = ok true / build_matches true (full-build SHA1 ==
oracle).

CONSTRUCTS: (1) local `int red` holding the shared red-channel value 0xFC, written
once and read four times; (2) each arm of the inner `if` writes its own complete
vertex-0 and vertex-1 RGB triple at offsets 4,5,6,0xC,0xD,0xE in ascending field
order, so the two @4 stores and the two @0xC stores carry the same value on
mutually exclusive paths. No holder with an intent name, no do-while(0), no
invented multi-write carrier, no annotation-owing device, no volatile, no asm,
no dead store, no pad, no alias.

## T1 semantic purpose
`red`: names the red channel that both branch outcomes share; every read consumes
it as the value stored. Removing it would require literally repeating 0xFC four
times — the variable is the ordinary spelling, not an effect-free addition.
Per-arm triples: every one of the six stores in each arm is a real, live store
executed on that path; the primitive's vertex-0/vertex-1 colours are fully
written by whichever arm runs. Nothing here is byte-identical-with-or-without:
delete any store and the primitive's colour is wrong.

## T2 human-programmer
Yes. Each arm sets the two vertices' colours completely, in ascending field
order — textually the same shape as the outer `else` arm ten lines below, which
writes exactly those six offsets in exactly that order, and the same shape as the
COMPLETED-C sibling func_80072BC4 already on main in this same file. A reader
asks no "why is this here?" question about any line: the answer is "that vertex
gets that colour on this path".

## T3 GCC-internals justification
The body's presence is not justified by a GCC pass. The reason the code is
written this way is that both branch outcomes assign a full RGB pair and the
shared red channel is named once. GCC internals appear in the ledger only as the
EXPLANATION of why the alternative spellings (hoisting the common stores into
the join) diverge — jump.c cross_jump merges the per-arm copies to the single
`.L80072D64` tail the target ships (asm/funcs/func_80072CD4.s lines 40-42) — but
that is a description of the compiler's behaviour, not the semantic reason the
statements exist. Per .claude/rules/ordinary-c-judge-decidable.md:57,
"'scheduling-motivated respelling' is not a FAIL ground when the spelling is
semantically truthful."

## T4 permuter/search provenance
No permuter, no auto-search. The form was derived by hand from the target's own
control flow and confirmed by the sched_solver run banked at
tmp/grind/func_80072CD4/s16/perturb_naturalE_depth2.txt. It survives detection
not by evading a detector but because there is no device in it to detect.

## T5 family check
Honest statement, not a denial: the two @4 stores and the two @0xC stores DO
carry the same value in both arms, and jump2's cross_jump merges them, so they
cost zero final bytes. That shape was previously banned for this function under
the 2026-07-24 premise that it was a "store-SCHEDULING-order duplication" with an
`int fc_const` holder injecting a bare `@4=fc_const; @0xC=fc_const;` pair
(rejected/dup4_0xc_into_arms.c). This body has no holder and no injected pair:
the stores are members of a complete per-arm triple. The Judge ruled on exactly
this question and this body on 2026-09-04 01:32 — "banned_constructs entry 5 /
judge_constraints entry 1 are narrowed: they do NOT reach a device-free,
unannotated, ascending-order per-arm triple body" — so the ban does not reach
this diff. I am not re-declaring a banned construct and I am not claiming the
body "duplicates nothing".

## T6 naming-announces-intent
`red` names the colour channel it holds. No `pad`, `dummy`, `unused`, `spill`,
`tmp`, `holder`, `fc_const`, `_buf`, `slack`. Every identifier survives a neutral
rename; `red` is read four times, never merely declared or address-taken.

SANCTIONED-FAMILY-CLAIMS: none — this body is ordinary C and claims no exception
family. Governing disposition:
.claude/rules/ordinary-c-judge-decidable.md:57 (Ruling 1(3), the rename test)
and the Judge PASS at docs/grind/decisions.md:22248, which states verbatim that
"NO /* FAKE */ annotation is owed here (this is ordinary C, not an
exception-family use); adding one would be a defect."

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. No claimed family mandates an
annotation, and the governing Judge ruling explicitly forbids adding one to this
body.
