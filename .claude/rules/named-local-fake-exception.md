---
name: named-local-fake-exception
paths: [".claude/rules/named-local-fake-exception.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): constant-holder locals (a local existing only to carry a constant in a register across calls) and dead/unused SCALAR locals that bias RA are allowed as LAST-RESORT levers with `/* FAKE: ... */` annotation + documented lever-exhaustion. SOTN ships `s16 three = 3;`, `s32 zero = 0; // needed for PSP`, `fake = 8;`, `new_var` in 9 files. Arrays/frame coercion stay forbidden."
metadata:
  type: rule
---

# Constant-holder / dead scalar locals — the FAKE-annotated exception

**Owner ruling 2026-07-01** (evidence base:
[[sotn-family-research-2026-07-01]]): a local variable whose only
purpose is codegen influence — holding a constant in a register across
calls/statements, or existing as a declaration that biases register
allocation — is a sanctioned last-resort matching lever under the
prerequisites below. This extends the 2026-06-02 sanctions (variable
reuse, opaque `s32 one = 1;`, named intermediates) to the two shapes
the 2026-06/07 re-audit flagged: **constant-holders across calls** and
**dead scalar declarations**.

## What is sanctioned (exact scope)

- **Constant-holder local:** `s32 k = 1; f(); g_a = k; ...; g_b = k;` —
  a local initialized to a constant, kept live across intervening calls
  so the constant sits in a callee-save register instead of being
  re-materialized. (The `DispSamnailWindow` shape.)
- **Dead / unused SCALAR local declaration** whose presence shifts RA
  (`int new_var;` — the `func_8006E49C` shape, proven load-bearing
  0→35).
- **m2c-artifact / generic names** (`new_var`, `temp`) are acceptable
  AS NAMES when the construct itself passes; prefer renaming to
  something meaningful, but the name alone is not a FAIL trigger
  (SOTN ships `new_var` in 9 committed files).

## SOTN / community evidence (verbatim, master branches)

- SOTN `src/dra/84B88.c:817-860` — `s16 three = 3; s16 one = 1; ...
  unk7E.val += three; unk7C += one;` (named constant-carriers)
- SOTN `src/dra/7879C.c:2067` — `s32 zero = 0; // needed for PSP`
- SOTN `src/st/cat/e_hellfire_beast.c:615,819-823` — `s32 fake; ...
  // FAKE! PSX requires some trickery here, was not able to resolve` /
  `fake = 8; prim->drawMode = fake;` (constant-holder literally named
  `fake`)
- SOTN `src/dra/42398.c:280` — `sprite->u0 = (D_801362B4 & 1) <<
  (new_var = 7);`
- SOTN `src/dra/cd.c:520-552` — `int new_var2; s32 new_var; CdThing*
  new_var3;`, `new_var2 = 6` used once as a shift amount
- SOTN 65+ `pad`/`dummy`/`unused` dead-local declaration sites across
  44+ files (e.g. `src/dra/704D0.c:53` `s32 unused = 1; // Meaningless,
  PSP-only`)
- papermario `src/world/world.c` — match-temp kept: `// TODO:
  Potentially a fake match? Difficult to not set the temp in the for
  conditional.`
- oot `z_en_elf.c:1478-1479` — `// temp probably fake match` /
  `unk2C7 = this->unk_2C7;`

## Strict prerequisites (cheat-reviewer FAILs if any is missing)

1. **Lever-exhaustion is documented** (WIP ledger / commit body): the
   natural inline-literal / direct-expression forms and the
   [[register-alloc-pure-c]] A/B/C levers were measured-negative first.
2. **The GCC-pass interaction is named** (e.g. "keeps the constant in
   a callee-save across the jal instead of re-materializing `li 1`").
3. **Mandatory annotation:** `/* FAKE: <one-line reason> */` or
   `// FAKE: <reason>` at the declaration (or the holding assignment).
4. **Layer-1 + layer-2 cheat-reviewer** per
   [[review-discipline-before-commit]].

## What stays FORBIDDEN (non-extension)

- **Unused ARRAYS / oversized arrays / frame-size coercion** —
  [[dead-vars-local-array]] unchanged. The sanction is SCALAR locals
  whose mechanism is RA/scheduling; anything whose mechanism is "reserve
  frame bytes" stays forbidden pending its own evidence request.
- **Address-coerced locals** (`(void)&local;`, `&local` passed nowhere)
  — unchanged.
- **Register pins, asm injection, barriers** — unchanged.
- Non-extension per [[no-new-park-categories]]: other spellings need
  their own evidence.
- **Dummy constant locals used as array subscripts (owner ruling
  2026-09-27, thirteenth batch, Q22).** The question put to the owner,
  verbatim, is recorded in docs/grind/owner-rulings-2026-09-26.md
  (batch 13); it asked whether `s32 zero = 0; s32 one = 1; /* FAKE */`
  used as `score[zero]` / `score[one]`, so that the compiler treats the
  subscript as unknown and emits direct accesses, counts as an allowed
  construct. Owner (Trenton) chose, verbatim: **"Doesn't count
  (Recommended)"**, whose text is: "Dummy constant locals used only as
  array indices are outside the sanctioned FAKE-local scope. The per-file
  declarations stay (no FAKE constructs anywhere). I fix the ledger proof
  and the file comments to cover this trick, then re-run layer-2." The
  author's narrowing: a DUMMY local is one whose value at EVERY one of
  its reads is fixed at compile time, i.e. the same on every feasible path
  reaching that read (branch conditions jointly satisfiable, as in Ruling
  5 2(c)); the fixed values may differ from read to read, and the reads
  include those made by the local's own updates (`i++`, `i += 1`). This
  holds whatever its name, its annotation or the spelling of its writes (a
  literal, a constant expression, an increment of a fixed value, or an
  expression over other locals or parameters that is fixed at that read,
  e.g. `one - 1` where `one` always holds 1). So `s32 zero = 0;`, `one -
  1`, and a stepped cursor whose every read is fixed (`s32 i = 1; a[i -
  1]; i++; a[i - 1];`) are all dummy locals. A read of a dummy local that is an array
  subscript or a pointer offset, alone or inside a larger index or offset
  expression (`a[k]`, `a[i + k]`, `a[k][j]`, `&a[k]`, `*(a + k)`,
  `p + k`, `(u8 *)p + k`), is REFUSED. A "pointer offset" here is an
  integer added to or subtracted from an operand of pointer or array type
  (C pointer arithmetic); integer-typed address arithmetic (e.g.
  func_8006E49C's `s32 base2; ... base2 + tail`) is not a pointer offset
  and is outside this refusal. Such a local is not a constant-holder under
  this entry and not an opaque arithmetic variable, and no other FAKE-local
  family admits it for a subscript or pointer offset. (The opaque-arithmetic
  entry's frozen-list text names bit-test transforms; its existing
  admissions for other effects, e.g. func_8006E49C `tail`, decisions.md
  2026-07-17, are unaffected.) The author's addition, beyond the owner's "only": the
  refusal applies whatever the dummy local's OTHER reads are. A dummy
  local whose other reads are plain value reads of that same fixed value
  (e.g. `D_800A377C[round] = zero;`), or that is admitted for another
  mechanism (a constant-holder across calls, an opaque `one`), gets
  nothing for such a subscript or offset read. That read is not a
  sanctioned mechanism, cannot be the GCC-pass interaction named under
  prerequisite 2 of any FAKE-local family, and is spelled with the
  literal constant. For the per-file-declaration exception's condition
  (1) in [[no-new-park-categories]] (aggregate-merge entry), a
  single-declaration spelling that relies on a subscript or
  pointer-offset read of a dummy local is set aside like any other
  refused construct. A local with at least one
  read whose value is chosen at run time (it can differ between feasible
  paths or executions) is not a dummy local and is outside this refusal:
  for example PsyQ SetDrawEnv's packet cursor (`var_a3 = 7; if
  (r->flag) { o[var_a3++] = ...; ... }`, then `var_a3 - 1`, which is 7
  or 10 depending on `r->flag`; src/display.c, COMPLETED-C). A local that
  is fixed at its subscript reads and is later given an unrelated
  run-time value (a reused local) is judged, unchanged, under the
  reused-variable rulings (ordinary-c-judge-decidable Rulings 5-12),
  where a value whose writes are all literal constants fails Ruling 11
  (C)(3) unless its per-branch-constants clause applies; its subscript
  reads are reads of a non-dummy local and are outside this refusal. A
  subscript variable whose value at
  the subscript read can differ between feasible paths or between
  executions (read from state, computed from such a value, a loop counter
  that takes two or more values at that read, or different constants on
  different feasible paths in Ruling 11 (C)(3)'s per-branch sense) is
  ordinary C and is unaffected. A value that is computed but is the same
  on every feasible path is not chosen at run time and falls under the
  refusal above. Record: docs/grind/decisions.md
  2026-09-27 OWNER RULING — dummy constant locals as array subscripts.

## Related

- [[sotn-family-research-2026-07-01]] — evidence base + impact map.
- [[dead-store-fake-exception]], [[pointer-alias-fake-exception]],
  [[mmio-volatile-type-level]] — the other 2026-07-01 rulings.
- [[loop-rotation-two-shift]] — the 2026-06-02 opaque-`one` sanction
  this generalizes from.
