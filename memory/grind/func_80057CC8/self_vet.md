# SELF-VET — func_80057CC8

Session 29 (2026-08-20, `escalation` modality). Diff surface: `src/text1b.c` ONLY — the
single line `INCLUDE_ASM("asm/funcs", func_80057CC8);` at line 1524 replaced by a 5-field
`typedef struct` plus the function body now banked at
`memory/grind/func_80057CC8/candidate.c`. No other file touched (`git status`: `src/text1b.c`
plus the engine's own `metrics/events.jsonl` append).

Measured THIS session, this chassis:
  `sandbox func_80057CC8 --disable all` -> score 0, target_insns 111 == build_insns 111,
  rules_dropped 0.
  full `build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
Zero regfix/asmfix rules, zero `__asm__`, zero register pins, zero `volatile`, zero
`/* FAKE */` constructs, zero dead locals, zero dead stores.

CONSTRUCTS: (1) `VertRing_57CC8` — a TU-local 5-field struct type given to the function's
first parameter (`u8 *arg0` -> `VertRing_57CC8 *ring`), replacing the byte-offset casts
`*(s16 **)(arg0 + 4)` / `arg0[3]` / `arg0[2]` with `ring->verts` / `ring->nverts` /
`ring->unk2`; (2) `{ s32 tmp = arg1 + 1; next_idx = tmp; if ((s16) tmp >= (s32)ring->nverts)
next_idx = 0; }` — a block-scoped temp; (3) `u16 cx` / `u16 cy` locals holding the current
vertex; (4) direct array indexing `ring->verts[(s16) prev_idx * 2]` /
`ring->verts[(s16) next_idx * 2]` (and `+ 1`) at the two `ratan2` call sites.

Constructs (2) and (3) are inherited VERBATIM from the floor-3 baseline (commit 25af3133)
that already carries a layer-2 cheat-reviewer PASS in this ledger. Constructs (1) and (4)
are this session's change and, taken together, they are a NET REMOVAL: the two pointer
locals `table` and `p` are both DELETED, along with every cast in the body's address
arithmetic and the baseline's byte-inert `s16 new_var` carrier. The body has four fewer
declarations than the layer-2-PASSed baseline and no cast expressions at all.

## T1 semantic purpose:
(1) The struct is the parameter's real type. Every one of its named fields is read and
consumed: `verts` supplies six halfword loads, `nverts` bounds the ring wraparound in two
places, `unk2` is the radius scale. Removing the type is only possible by re-introducing
compensating casts that do the identical work — the type is not additive, it is the
absence of the casts. (2) `tmp` is read three times (assign to `next_idx`, the `(s16)`
compare, and as the surviving value). (3) `cx`/`cy` are the current vertex's coordinates,
each consumed in three arithmetic expressions. (4) The two indexed reads ARE the two
`ratan2` argument pairs. Every construct changes the function's output; none is
byte-neutral; none can be deleted without changing what the function computes.
Explicitly: there is NO construct in this diff whose removal leaves behaviour identical.

The thing a reviewer must interrogate — because the two prior layer-1 FAILs turned on it —
is the TWO loads of the vertex-table base in the target (`asm/funcs/func_80057CC8.s:17`
`lw $a2, 0x4($s2)` and `:50` `lw $a0, 0x4($s2)`). In THIS body no second read is spelled at
the source level at all: there is no `table`, no `nt`, no repeated `*(s16 **)(arg0 + 4)`
expression. The body simply references `ring->verts` at each of its use sites, and `ratan2`
intervenes between the two groups. A call clobbers memory, so GCC 2.7.2 MUST reload
`ring->verts` after it — the second `lw` is the compiler's, not the author's. The
alternative (hold the base in a callee-save across the call) is what the s29 v7/v9 variants
measured: 112 instructions against a 111-instruction target, i.e. structurally unmatchable,
banked at `rejected/reload-elimination-p-derived-from-table-score30.c` and
`rejected/both-sites-indexed-no-reload-score30.c`.

## T2 human-programmer:
Yes to all four, and this is the first form in 29 sessions where that answer is
unqualified. Given the spec — "read my own vertex, read my previous and next neighbours'
vertices around a ring of `nverts`, bisect the two angles, and emit the point at radius
`unk2 * 40` around my vertex" — the natural C is exactly a struct pointer with
`ring->verts[i * 2]` accesses. Nothing here would make a reader ask "why is this here?".
The construct a reader WOULD have asked that about (a second named pointer local holding a
value another local already holds) is precisely what this session deleted.

## T3 GCC-internals justification:
No construct in the diff is justified by a GCC internal. The ledger's mechanism story
(local-alloc.c:472's `reg_n_deaths == 1` bail-out punting the two-SET pointer pseudo to
global-alloc, where pseudo 129's copy preference pins it to `$v1`) is the EXPLANATION of
why 28 sessions of pointer-local forms failed — it is not a lever this body pulls. The fix
is a deletion, not an insertion aimed at a pass. Strike the mechanism paragraph entirely
and the body is still the simpler and more idiomatic of the two: fewer locals, no casts.
No allocator, scheduler, DCE, combine, or reorg behaviour is being steered; no barrier, no
ordering trick, no width coercion.

## T4 permuter/search provenance:
None. No permuter ran this session. The form was derived by reading the target's loads
first (`:17` / `:50`), asking whether the function needs any pointer local at all, and then
measuring three typed variants in order: local-cast-from-`u8 *` (score 10, banked at
`rejected/struct-local-cast-from-u8ptr-score10.c`), typed parameter (score 0), and the same
with conservative field names (score 0). All prior permuter output for this function
(s4/s5/s13/s14/s16/s22 — every one a `p1`-alias-holder cheat class) is banked under
`rejected/` and none of it is reused here.

## T5 family check:
No sanctioned-family carve-out is claimed and none is needed — see
SANCTIONED-FAMILY-CLAIMS. Checked against the four BANNED constructs in `state.json`
judge_constraints and the 2026-08-20 04:30 / 04:40 layer-1 FAILs:
  - BANNED "two source-level pointer locals both loaded with the identical expression, both
    denoting the same unchanging vertex-table base" — NOT PRESENT. This body declares ZERO
    pointer locals.
  - BANNED "`nt = *(s16 **)(arg0 + 4);` as a second, independent reload" — NOT PRESENT. No
    such statement, and no local exists that could be reloaded into.
  - BANNED "`p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));`
    — a second, independent inline reload while `table` is in scope" — NOT PRESENT. The
    entire `p` construction is deleted, `table` does not exist, and the expression appears
    nowhere in the diff. The class the two layer-1 FAILs objected to has been removed rather
    than respelled: there is no name and no expression left to rename.
  - BANNED "the self-vet's own T1 argument that removing the reload is 'measured strictly
    worse and structurally impossible to match'" — NOT RELIED ON as a justification here.
    The v7/v9 112-insn measurements are cited above only as banked ledger fact explaining
    why a callee-save-across-the-call shape cannot reach 111; the justification for this
    body is that it is ordinary C which spells no reload at all.
Checked against the forbidden-family catalog: no register-asm pin, no hardcoded-`$N` asm,
no regfix insert, no scheduling barrier, no volatile of any spelling, no alias rename, no
unused local array or frame pad, no dead-param assign, no dead conditional store, no empty
`if`, no `if (1)`, no dead goto/label pad, no DImode chain, no goto-end accumulator, no
opaque constant variable, no redundant width cast, no linker-script reorder.

On the struct type specifically, since it is the only novel element: it is NOT the
"per-word splat symbol -> aggregate merge" family (`.claude/rules/no-new-park-categories.md`
2026-08-17) — that family governs merging splat-invented `D_<addr>` GLOBAL scalars into a
shared-header aggregate, and this diff merges no globals and touches no splat symbol config.
It is also NOT `.claude/rules/header-type-correction-from-use-sites.md`, whose scope is a
GLOBAL's declared type at its canonical `extern` declaration in a shared header; this is a
function parameter with no `extern` declaration anywhere in the build (the only prototype in
the tree, `include/m2c_context.h:782`, is m2c decompiler context and is not compiled).
Declaring a decompiled function's pointer parameter with its actual record type is ordinary
decomp practice, and this very file already carries five TU-local typedefs of the same shape
(`src/text1b.c:658`, `:1151`, `:1184`, `:1217`, `:1225`).
The object model is supported by base-register evidence in the ORIGINAL BYTES, not by splat
naming (cf. [[splat-symbol-names-are-not-evidence]]): the target reaches offsets 3 and 4 off
ONE base register that is the incoming `$a0` (`lbu 0x3($s2)` at `asm/funcs/func_80057CC8.s:20`
and `:31`, `lw 0x4($s2)` at `:17` and `:50`), and the word at +4 is a pointer to s16 pairs
indexed by a vertex index that wraps modulo the byte at +3. Field names are claimed only
where the code proves them — `nverts` (the ring bound: `prev = n - 1` on underflow,
`next = 0` at `>= n`) and `verts` (s16 pairs consumed as x/y by `ratan2`); offsets 0-2 are
left `unk0`/`unk1`/`unk2` rather than guessed, per [[names-require-evidence]].

## T6 naming-announces-intent:
All identifiers are semantic: `ring`, `verts`, `nverts`, `unk0`/`unk1`/`unk2`, `prev_idx`,
`next_idx`, `ang_prev`, `ang_next`, `ang_mid`, `scale`, `base`, `half`, `cx`, `cy`, `tmp`.
No `pad`, `dummy`, `unused`, `spill`, `_buf`, `slack`, or `new_var`-style carrier exists —
the baseline's one such name (`s16 new_var`) was deleted after measuring it byte-inert, and
the `unk*` field names denote genuinely unidentified storage rather than filler (they are
real bytes of the record, not padding invented to move the frame). Every declared local is
both written and read; every named struct field except `unk0`/`unk1` is read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and no exception is claimed. No
`/* FAKE */` construct exists in the diff, so no family scope sentence or precedent citation
is required or offered.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
