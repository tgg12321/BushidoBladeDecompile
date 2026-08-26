# SELF-VET — func_80061250 (s2, 2026-08-26, applied to src/text1b.c:3271-3299)

CITATION NOTE (read first): the prior layer-1 FAIL (2026-08-26 14:56) was
citation-only — the construct itself was verified legitimate by the reviewer.
The stale citations ("src/text1b.c:3348" / "src/text1b.c:3342-3369") were
PRE-application line numbers that drifted once the candidate body was inserted
above the sibling. Every line number below was re-grepped THIS session AFTER
the candidate was applied, against the exact file state the reviewer will read:
func_80061250 body = src/text1b.c:3272-3299 (extern at 3271);
sibling func_8006156C body = src/text1b.c:3370-3397 (extern at 3369);
sibling flag test `(&D_800F1154)[1] != 0` = src/text1b.c:3375.

CONSTRUCTS: pointer local `s32 *v1 = (s32 *)&D_800F116C` (written+read, value
published to D_800A3468 and dereferenced `*v1 = 0x21000A`); block-offset flag
accesses `(&D_800F1154)[5]` / `[6]` (test, clear, and address-of publishes);
nested-if + `goto check_one_zero` / `goto end` control flow; `*p++`
walking-pointer 3-word copy tail. No volatile, no FAKE, no dead stores, no
invented locals, no asm, no regfix/asmfix rules.

## T1 semantic purpose
Every construct has observable output semantics. `v1` holds the packet-handle
address that IS stored to D_800A3468 and dereferenced to write the 0x21000A
code. The flag bytes at D_800F1154+5/+6 are tested, cleared, and their
addresses published to (D_800A3468)+0x14 / D_800F1180 — all visible stores.
The goto structure selects which packet code (0x210009 vs 0x21000A) and which
address gets published — observable divergent behavior. The `*p++` tail copies
arg0[0..2] into D_800F1140/44/48 — three visible stores. Nothing in the body
is byte-inert or removable without changing output. PASS.

## T2 human-programmer
Yes — this is a message/packet dispatch helper: publish the handle, test and
consume a two-byte flag pair, emit one of two packet codes, then copy the
3-word payload and set the color mask. A programmer maintaining this family
wrote the identical structure five times: the committed, already-integrated
sibling func_8006156C (src/text1b.c:3370-3397, verified this session) has the
SAME pointer local, SAME flag test-and-clear over (&D_800F1154)[1]/[2] (its
flag pair; ours is [5]/[6], see src/text1b.c:3375 vs 3277), SAME
publish-address arms, SAME `*p++` tail; only the data (flag offsets, packet
codes 0x210005/6 vs 0x210009/A, mask 0xFF8080 vs 0xFF0060) differs.
find_duplicates similarity 1.000 (59/59). PASS.

## T3 GCC-internals justification
The body is justified by program logic alone (above). The ledger's mechanism
note (compound-const cse anchor explaining why the D_800F1154+5 block-offset
spelling forms a base register where a plain `&D_800F1159` spelling folds
absolute) is an EXPLANATION of measurements, not the reason any statement
exists — the spelling is inherited verbatim from the committed sibling's
Judge-PASSed source shape, which spells its flags as offsets off the same
D_800F1154 block base. No construct exists solely to steer a pass. PASS.

## T4 permuter/search provenance
No permuter or auto-search output in this body. It is a hand-derived clone of
the committed sibling func_8006156C with the function's own data constants,
proven by direct sandbox measurement (spelling ladder banked in evidence.md
s1: 19 → 25 → 0). PASS.

## T5 family check
No forbidden family matches: no register pins, no asm of any kind, no
volatile (the stray chassis-era `extern volatile u8 D_800F1159;` was REMOVED
by this edit), no alias renames, no dead stores/locals/arrays, no constant
holders, no `do{}while(0)`, no padding. The pointer local is a live,
consumed value (not an unused alias); the flag accesses are ordinary C
addressing of a global byte block, identical in shape to the committed
sibling body at src/text1b.c:3370-3397. PASS.

## T6 naming-announces-intent
Locals are `v1`, `p` — dataflow names carried over from the sibling; no
pad/dummy/spill/unused naming. PASS.

SANCTIONED-FAMILY-CLAIMS: none — no construct in the diff requires a
sanctioned-exception family; every statement is ordinary semantic C whose
shape is byte-for-byte the committed COMPLETED-C sibling's (func_8006156C,
src/text1b.c:3370-3397 as of this edit; its COMPLETED-C body landed in commit
450f69d1 "Match: func_8006156C — COMPLETED-C (grinder, 1 sessions)",
2026-07-22, per the Judge PASS 2026-07-22 21:27 record in
docs/grind/decisions.md).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (none required; no
sanctioned-family claims made).

Measurements this session (s2): `sandbox func_80061250 --disable all` =
score 0, 59/59 insns, rules_dropped 0, nothing stripped from this body
(cheat_asm_stripped 169 = other functions' legacy debt, unchanged).
