# SELF-VET — CdControlF

Diff under review (vs HEAD, `git diff src/system.c`):
- `src/system.c:181` signature `s32 CdControlF(s32 a0, s32 a1)` -> `(u8 a0, s32 a1)`
  and the forward declaration at `src/system.c:1014` kept in sync at `(u8, s32)`.
- REMOVED: `register s32 result asm("s6");` (a register-asm pin — cheat-asm) and
  the inherited `int new_var = 3; unsigned long long new_var2 = new_var;
  count = new_var2;` DImode chain. The body is strictly LESS constructed than HEAD.
- `s32 *base;` local + the two-step address `base = g_cd_sector_buf;
  elem = base + idx;` (replacing `elem = &g_cd_sector_buf[idx];`).
- Init statement order is now `idx, saved, count, base, elem, result`.
- `do { ... } while (0);` region around the retry-loop body, FAKE-annotated
  inline at the construct site.
- `idx = a0;` / `CD_cw(a0, a1, 0, 1)` instead of `a0 & 0xFF` (the mask is a no-op
  on a `u8` parameter; GCC re-materialises target's two `andi` itself).

CONSTRUCTS: do-while(0) wrap around the retry-loop body; `u8` parameter type;
`base` named intermediate + two-step pointer arithmetic; init-statement order.

## T1 semantic purpose
- **do-while(0) wrap**: no semantic purpose — it is a match device, and it is
  declared as such in an inline `/* FAKE: ... */` annotation at the construct
  site. It is the ONE construct in the diff of this kind, and it is a sanctioned
  family (see SANCTIONED-FAMILY-CLAIMS). It asserts nothing false: "this body
  executes once" is true.
- **`u8 a0`**: real semantic content. The parameter IS a command byte (Sony
  libcd's `int CdControlF(u_char com, u_char *param)`), every caller passes a
  small literal, and the body's only uses are as a table index and as CD_cw's
  first argument. It replaces two explicit `& 0xFF` masks with the type that
  makes them redundant — this narrows, it does not coerce.
- **`base` + two-step**: real semantic content — a named pointer to the command
  table, then the element pointer. This is the spelling the already-matched
  sibling `CdControlB` uses at `src/system.c:237-238` in this same file.
- **init order**: statement order carries no semantics at all; every one of the
  six initialisers is a live, required statement.

## T2 human-programmer
- `u8 com`-typed parameter, a named `base` table pointer, an `elem` element
  pointer, and initialising `result` last (right before the retry loop it is
  the failure default for): every one of these is what a human writing this
  function from its specification would produce. `base`/`elem` is literally the
  in-file house style of the matched twin.
- The do-while(0) wrap is the one construct a reader would ask "why is this
  here?" about. That question is answered in-source by the FAKE annotation, and
  the construct is exactly the era-idiomatic macro-body shape (`do { ... }
  while (0)`) that the owner ruled sanctioned for this project on 2026-07-06.
  This is acknowledged, not disguised.

## T3 GCC-internals justification
- The do-while(0) wrap's justification IS a GCC-internals mechanism (flow.c
  `reg_n_refs += loop_depth` feeding `global.c:635 allocno_compare`), and I state
  that openly rather than dressing it as program logic. Under the general policy
  that is the cheat signal; under the do-while(0) rule as it stands on main it is
  explicitly permitted — the rule abolished mechanism-scoping and sanctions the
  construct "for ANY codegen effect, including register allocation", requiring
  only that the annotation name the observed effect. The annotation names both
  the effect and the pass.
- The other three constructs need NO GCC-internals justification: they are
  justified by the function's specification (a command byte; a table pointer;
  statement order). The `u8` retype is corroborated by the Sony prototype and was
  measured neutral at all five call sites.

## T4 permuter/search provenance
No permuter was run in any of the three sessions. The do-while(0) region was
DERIVED, not searched: session 3 read `tools/gcc-2.7.2/flow.c` and
`tools/gcc-2.7.2/toplev.c`, predicted the resulting seven-row priority table by
hand in `tmp/grind/CdControlF/s3/variants_b.py`'s docstring BEFORE running it,
and the measurement reproduced the predicted table exactly (count 4000 / a1 3783
/ idx 2352 / a0 2162 / saved 2162 / elem 967 / result 769). The init order was
then found by an exhaustive 240-permutation sweep of a semantics-free axis
(statement order), not by mutation search over constructs.

## T5 family check
- do-while(0) wrap: matches a SANCTIONED family, single-level (not nested), with
  the mandatory inline FAKE annotation. Claimed below with a verbatim scope
  sentence and a precedent.
- `u8` parameter type: not in any forbidden family. It is not a volatile
  coercion, not an alias rename (the symbol is the parameter itself, there is no
  second handle), not a redundant width cast (it REMOVES two casts), not a
  dead/holder local. The nearest catalogued relative is the ALLOWED
  `narrow-byte-args-packed-call` technique (declare byte-semantic params `u8`).
- `base` + two-step: an ordinary live local with a real value, read once. Not a
  dead local, not a constant holder, not a pointer-alias-to-a-global fake (it
  holds the array's decayed address and is genuinely consumed by the very next
  statement). Provenance is the matched sibling in the same file.
- init order: the frozen SOTN list names statement order / named-intermediate
  declaration order as allowed; nothing about it is a construct.
- Nothing in the diff is a register pin, hardcoded-`$N` asm, scheduling barrier,
  volatile coercion, dead store, dead conditional store, empty `if`, `if (1)`,
  dead-goto pad, DImode chain, alias rename, or regfix/asmfix touch. The diff
  DELETES a register pin and a DImode chain that HEAD carried.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack` or any
role-announcing variant. The four locals are `result`, `idx`, `saved`, `count`
plus the two pointers `base` and `elem`; every one of them is read on a live
path. There are zero declarations whose only uses are discards, address-of, or
nothing.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap (single level, FAKE-annotated)
  SCOPE: "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation.**"
  PRECEDENT: `.claude/rules/do-while-zero-exception.md:23`
  PRECEDENT: `cf3e6ce7`

  FAMILY: named-intermediate declaration order / statement order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:189`

ANNOTATION-CONFORMANCE: the single FAKE construct carries this inline annotation
at its site (`src/system.c`, immediately above the `do {`):

    /* FAKE: loop-note ref weighting seats elem in s5 and result in s6,
       mechanism: flow.c life analysis (reg_n_refs += loop_depth) feeding
       global.c allocno_compare, lever-exhaustion: memory/grind/CdControlF/
       hypotheses.md (s1 60+120 perms, s2 240 init orders + batches A-E,
       s3 batches A/B) */

It names WHAT (elem->s5, result->s6), the MECHANISM by named GCC pass (flow.c
life analysis / global.c allocno_compare), and where the LEVER-EXHAUSTION record
lives. The prerequisite for a single-level wrap under the rule is the annotation
plus "prefer natural geometry first" (exhaustion is explicitly not a hard gate
for single-level wraps); the exhaustion record is supplied anyway and is real —
three sessions, 60 + 120 + 240 + 240 permutation sweeps and five measured
variant batches, all banked in `hypotheses.md`. The wrap is not nested, so
prerequisite 3 does not apply.
