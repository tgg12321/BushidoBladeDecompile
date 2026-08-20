# SELF-VET — func_80057CC8

Diff surface: `src/text1b.c` only — the single line `INCLUDE_ASM("asm/funcs", func_80057CC8);`
replaced by the C body now saved at `memory/grind/func_80057CC8/candidate.c`.
Measured this session: `sandbox func_80057CC8 --disable all` = 0, target_insns 111 ==
build_insns 111, rules_dropped 0; full `build` sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
== oracle, MATCH. Zero regfix/asmfix rules, zero `__asm__`, zero register pins, zero
`volatile`, zero `/* FAKE */` constructs, zero dead locals.

CONSTRUCTS: (1) `s16 *p` — single-SET pointer local for the NEXT-neighbour vertex pair,
built as `(s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)))`;
(2) the cast-heavy sign-extend-and-scale idiom `((s32)(x << 16) >> 16) << 2` inside that
expression; (3) `{ s32 tmp = arg1 + 1; next_idx = tmp; if ((s16) tmp >= (s32)arg0[3])
next_idx = 0; }` block-scoped temp; (4) byte-offset reads of the current vertex
`cx = *(u16 *)((s32)table + arg1 * 4 + 0)` / `cy = ... + 2`; (5) direct array indexing of
the PREV-neighbour vertex pair `table[(s16) prev_idx * 2]` / `[... * 2 + 1]`.

Constructs (1)–(4) are inherited VERBATIM from the floor-3 baseline (commit 25af3133) that
already carries a layer-2 cheat-reviewer PASS in this ledger. Construct (5) is this
session's only change and it is a REMOVAL: the baseline built a second value into `p` for
the prev site; the new body reads the vertex pair directly instead. Nothing was added.
The baseline's `s16 new_var` embedded-assignment carrier is also deleted (measured
byte-inert), so the body has one fewer local than the PASSed baseline.

## T1 semantic purpose:
(1) `p` holds the address of the next-neighbour vertex pair and is dereferenced twice —
real value, real use. Deleting it is not possible without changing what is read.
(2) The shift idiom performs the s16 sign-extension of `next_idx` (declared
`unsigned short`) and the ×4 scale to a 2×s16 vertex pair; both operations are
semantically required and both appear in the target (`sll/sra/sll` at
asm/funcs/func_80057CC8.s around the second `lw`).
(3) `tmp` is read three times (assign, compare, and via `next_idx`) — real value.
(4) `cx`/`cy` are the current vertex's x/y, consumed in four arithmetic expressions.
(5) `table[(s16) prev_idx * 2]` is a plain array read whose loaded halfwords are the two
`ratan2` arguments. Every construct changes the function's output; none is byte-neutral.
The one construct a reviewer should interrogate is the SECOND read of the vertex-table
base (`*(s16 **)(arg0 + 4)` inside (1)) while `table` already holds that value: it has
observable effect and is materialised in the target — `asm/funcs/func_80057CC8.s:17`
`lw $a2, 0x4($s2)` and `:50` `lw $a0, 0x4($s2)` are two independent loads of that same
field, on either side of the first `jal ratan2`. Reusing `table` there instead is
MEASURED strictly worse and structurally impossible to match: score 30 with
build_insns 112 vs target 111 (variants banked at
`memory/grind/func_80057CC8/rejected/reload-elimination-p-derived-from-table-score30.c`
and `both-sites-indexed-no-reload-score30.c`) — GCC must otherwise hold the base live
across the call. The re-read is the original program's own shape, not a coercion.

## T2 human-programmer:
Yes to all five. This is the natural C for "read my own vertex, read my previous
neighbour's vertex, read my next neighbour's vertex, take the two angles, bisect them,
and emit a point on a circle of radius arg0[2]*40 around my vertex". Re-reading a struct
field at the point of use (construct 1) is what an ordinary 1998 C programmer writes when
the two lookups are written as independent statements; nothing in the body would make a
reader ask "why is this here?". The cast idioms (2) and (4) are forced by `arg0` being an
untyped `u8 *` in this TU — the standing decomp spelling everywhere in src/text1b.c.

## T3 GCC-internals justification:
No construct in the diff is justified by a GCC internal. The ledger DOES name a GCC
mechanism (local-alloc.c:472's `reg_n_deaths == 1` bail-out) but only as the EXPLANATION
of why the 28 prior sessions' extra `p` SET was harmful — the fix is not an insertion
aimed at that pass, it is the deletion of an unnecessary pointer construction in favour
of ordinary array indexing. Take the mechanism story away entirely and the body is still
the simpler, more natural C of the two. No allocator/scheduler/DCE/combine lever is being
steered, no barrier, no ordering trick.

## T4 permuter/search provenance:
None. No permuter run this session. The form was derived by reading the target's two
`lw 0x4($s2)` loads and asking whether the function needs a pointer local at the prev
site at all. Prior permuter output for this function (s4/s5/s13/s14/s16/s22, all
p1-alias-holder cheat classes) is banked under `rejected/` and none of it is reused here.

## T5 family check:
No sanctioned-family carve-out is claimed and none is needed — see
SANCTIONED-FAMILY-CLAIMS. Explicit check against the two BANNED constructs recorded in
`state.json` judge_constraints and the 2026-08-20 04:30 layer-1 FAIL:
  - BANNED "two source-level pointer locals both loaded with the identical expression,
    both denoting the same unchanging vertex-table base" — NOT PRESENT. This body
    declares exactly ONE base pointer local, `table`. The other pointer local, `p`,
    denotes a DIFFERENT address (base + next_idx*4), is written once, and is the same
    single-SET `p` the layer-2-PASSed floor-3 baseline already had.
  - BANNED "`nt = *(s16 **)(arg0 + 4);` as a second, independent reload of the identical
    pointer value `table` already holds in scope" — the second *load* is present (inline,
    inside `p`'s address expression) but the *construct that was banned* — a second named
    pointer local created to hold it — is NOT. That inline load is not new and is not the
    lever: it is character-for-character the baseline's, it was in place at every measured
    floor of 3, and the 3→0 delta comes from the prev-site array indexing alone. I state
    plainly that a reviewer may still want to rule on whether the ban was intended to
    reach any second read of that field; the answer the bytes give is that it cannot have
    been, because the target performs that second load itself (`:17` and `:50` above) and
    every measured single-load form is 112 instructions against a 111-instruction target,
    i.e. unmatchable. If the reviewer disagrees, the correct disposition is a ruling, not
    a respelling — there is no third spelling of "load this field again".
No other family is approached: no volatile, no alias rename, no register pin, no inline
asm, no dead store, no dead local, no constant holder, no pad array, no do-while(0), no
duplicated statement into arms, no goto/label pad, no opaque variable, no `if (1)`.

## T6 naming-announces-intent:
All identifiers are semantic: `table`, `p`, `prev_idx`, `next_idx`, `ang_prev`,
`ang_next`, `ang_mid`, `scale`, `base`, `half`, `cx`, `cy`, `tmp`. No `pad`, `dummy`,
`unused`, `spill`, `_buf`, `slack`, `new_var`-style carriers — the baseline's one such
name (`s16 new_var`) was DELETED by this session after measuring it byte-inert. Every
declared local is both written and read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C with no exception claimed. No
`/* FAKE */` construct exists in the diff, so no family scope sentence or precedent
citation is required or offered.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
