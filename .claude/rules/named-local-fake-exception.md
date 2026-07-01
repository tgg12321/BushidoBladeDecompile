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

## Related

- [[sotn-family-research-2026-07-01]] — evidence base + impact map.
- [[dead-store-fake-exception]], [[pointer-alias-fake-exception]],
  [[mmio-volatile-type-level]] — the other 2026-07-01 rulings.
- [[loop-rotation-two-shift]] — the 2026-06-02 opaque-`one` sanction
  this generalizes from.
