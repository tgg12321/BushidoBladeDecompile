# SELF-VET — func_80061250

CONSTRUCTS: block-offset flag access `(&D_800F1154)[5]` / `(&D_800F1154)[6]`; pointer
local `s32 *v1 = (s32 *)&D_800F116C;`; store through published handle
`*(s32 *)((s32)D_800A3468 + 0x14) = ...`; goto-structured flag state machine
(`goto check_one_zero;` / `goto end;`); walking-pointer copy tail
(`p = arg0; D_800F1140 = *p++; ...`); `extern u8 D_800F1154;` declaration replacing the
stray unused `extern volatile u8 D_800F1159;` (chassis debt, no live uses removed).

## T1 semantic purpose
Every statement has an observable effect present in the target bytes: the flag test/clear
sequence on bytes 5 and 6 of the D_800F1154 flag block (target's lbu/sb run), publishing
the packet handle to D_800A3468 and D_800F1178, storing the flag address into field +0x14
of the packet (or D_800F1180 in the other arm), writing the per-arm command codes 0x210009
/ 0x21000A, calling func_80060A68, copying the 3-word coordinate block arg0[0..2] to
D_800F1140/44/48, and storing the 0xFF0060 color mask to D_800A3464. Zero dead stores,
zero unused declarations, zero constructs whose removal leaves behavior byte-identical.
PASS for all constructs.

## T2 human-programmer
Yes for every construct. The function is one member of a six-sibling family
(func_8006133C / 800613C8 / 80061454 / 800614E0 / 8006156C / 80061250) that all publish a
command packet and index the same flag block; a programmer who owns a multi-byte flag
block naturally indexes it from its base (`(&D_800F1154)[5]`) exactly as the
Judge-PASSed sibling func_8006156C does at src/text1b.c:3348 (`(&D_800F1154)[1]`). The
pointer local v1 is genuinely used (its value is published to D_800A3468 and written
through); the walking pointer is the ordinary idiom for copying three consecutive words.
PASS.

## T3 GCC-internals justification
No construct is justified by a GCC pass. The block-offset spelling is the program's data
model (one flag block, byte-indexed — the same model the accepted sibling ships); the
ledger's s1 notes about cse compound-constant anchors are diagnosis of why OTHER
spellings failed, not the reason this one exists. The body is a statement-for-statement
mirror of the semantic structure visible in m2c/target. PASS.

## T4 permuter/search provenance
No permuter or auto-search produced any part of this body. It was derived from the
COMPLETED-C sibling func_8006156C (find_duplicates similarity 1.000) by substituting this
function's own data (byte offsets 5/6, codes 0x210009/0x21000A, mask 0xFF0060). PASS.

## T5 family check
No forbidden family matches, directly or by analogy: no pins, no asm, no volatile
coercion (the diff REMOVES a stray unused volatile extern), no dead
stores/locals/arrays, no alias renames, no constant holders, no dead gotos (both labels
are reached). No sanctioned-FAKE family is used or needed — every construct is ordinary
semantic C with a committed, Judge-PASSed in-repo precedent: src/text1b.c:3342-3369
(func_8006156C, Judge PASS 2026-07-22). PASS.

## T6 naming-announces-intent
Names are v1, p, check_one_zero, end — descriptive control-flow/temporary names mirroring
the sibling; no pad/dummy/unused/spill/slack naming. PASS.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
