# SELF-VET — func_80060A68  (session 29, 2026-09-05)

STATUS: the body in memory/grind/func_80060A68/candidate.c — now applied verbatim at
src/text1b.c:3138 — is the body a Judge PASS ruling already cleared
(docs/grind/decisions.md:23184, 2026-09-05 01:20; clearance hash 5f78d844c1029e78 in
state.json). It is submitted EXACTLY as cleared, unrespelled. Measured THIS session:
`sandbox func_80060A68 --disable all` = score 0, build_insns 66 / target_insns 66;
`verify-oracle` = build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
original_sha1_locked, build_matches true.

CONSTRUCTS: (1) a block-scope struct/union type declaration `struct Ob` giving the object
D_800A3468 points at its shape, with `extern struct Ob *D_800A3468;`; (2) array-typed
declarations for the three tables (D_8009BA60, D_800F10D0,
chractar_use_pset_combo_id_table); (3) one local, `result`, holding the dispatch call's
return value; (4) the function-pointer call through the combo-id table.

## T1 semantic purpose
(1) A type declaration IS the semantic statement of what the storage is; it is not an
added construct with no observable effect — remove it and the program does not compile.
(2) Same: the tables are indexed, so declaring them as arrays is the ordinary spelling of
the access. (3) `result` holds the call's value between the call and the byte store — a
value that is genuinely produced and genuinely consumed; it is not dead, not
self-assigned, not multiply-written. (4) The call is the function's purpose. No construct
in the body is byte-identical-with-or-without: there is no construct in the body that
could be removed while leaving the program's meaning intact.

## T2 human-programmer
Yes. A programmer with the original headers would write member references and array
subscripts, not casts through integers. The struct reports the model the file's own
committed, MATCHED code already asserts: src/text1b.c:3461 declares
`extern s32 *D_800A3468;` (pointer type, in the matched sibling func_80061064); sixteen
matched sites store callee-returned pointers into it; the +0x14 member always receives a
byte-buffer pointer and is stored through with `sb`; offset 0 is written whole at five
matched sites as single 32-bit constants whose low halfword is this function's `lhu` index
and whose bit 21 is the flag this function tests. Nothing here would make a reader ask
"why is this here?".

## T3 GCC-internals justification
No construct in the body is justified by a GCC internal. Nothing in this diff is a lever:
there is no borrowed local, no staged value, no separator, no pad, no reordering-for-effect.
The header comment's numbered notes are an after-the-fact pass-attribution record of what
cse and sched did with the resulting stream, explicitly labelled in the file as
"observation, recorded for the next reader - not the reason any construct is here"; the
Judge ruled (decisions.md:23184, point 3) that an accurate after-the-fact
pass-attribution note is evidence, never a cheat ground, for a semantically truthful
spelling, and voided the paired banned_constructs entry on that paragraph. The
justification for every construct above is the object model evidenced from matched C.

## T4 permuter/search provenance
No permuter output is in this body. It was derived from the file's own matched sibling
declarations, not from a score sweep. Robustness check recorded in the ledger: FOUR
structurally distinct faithful spellings of the SAME model all measure 0/66 (this body;
tables through the address of their first word; offset 0 as two u16 members with a cast
flag read; a file-scope struct with every member renamed) — the three alternates are on
disk as memory/grind/func_80060A68/alt-s29-score0-*.c. The bytes track the model, not one
swept spelling.

## T5 family check
No forbidden family is present, by analogy or otherwise: no register-asm pin, no `__asm__`
of any kind, no scheduling barrier, no volatile, no alias rename, no unused/dead local, no
local array, no dead store or self-assign, no dead conditional or empty-body if, no
always-true wrapper, no goto, no DImode chain, no duplicated statement, no redundant width
cast, no linker/rodata change. No local is written twice. The five standing
banned_constructs on the staged/multiply-assigned carrier (`src`, `temp2` in all its
roles) are not present in any spelling — the body has exactly one local and it is written
once. The banned `(s32)D_800A3468 + 0x14` re-read of the raw global is NOT present: the
tail store is `*D_800A3468->p14 = result;`, a member reference like every other access.

## T6 naming-announces-intent
The only local is `result`, named for the value it holds and read on the next line. Member
names describe offsets in an object with no header declaration (DATA MODEL block: none of
D_800A3468's members has one). No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack` or
`_buf` name appears, and no local is address-taken, discarded, or declaration-only.

SANCTIONED-FAMILY-CLAIMS: none. This body claims no exception family: it is ordinary C.
The governing ruling is docs/grind/decisions.md:23184 ("(1) ORDINARY C. Declaring a
global's type is a semantic act, not a no-semantic-purpose construct, so the frozen family
list is not the governing surface. ... (2) N/A - no family grant needed."), which also
cleared this exact body for submission and unbanned the derived struct-rewrite entry.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The body contains zero FAKE constructs;
no claimed family mandates an annotation, because no family is claimed.
