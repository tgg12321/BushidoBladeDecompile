---
name: pointer-alias-fake-exception
paths: [".claude/rules/pointer-alias-fake-exception.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion. SOTN idiom incl. self-admitted `// n.b.! unused, required for PSP`. asm(\"sym\") alias-RENAMES stay forbidden."
metadata:
  type: rule
---

# C-level pointer aliases to globals — the FAKE-annotated exception

**Owner ruling 2026-07-01** (evidence base:
[[sotn-family-research-2026-07-01]]): a local pointer that provides a
second C handle to a global — where using the global directly would be
semantically identical — is a sanctioned last-resort matching lever
under the prerequisites below.

## What is sanctioned (exact scope)

- `Type* t = &g_Thing;` then `t->field` where `g_Thing.field` would do
  (the redundant-second-handle shape).
- Typed re-views: `s16 (*p)[] = &D_xxx;` array-pointer casts, and
  FakePrim-style reinterpret-cast pointer views of a global buffer.
- Pass-through aliases: `Entity* e2 = self;` used at a call site.

The alias must be ordinary C — a visible local declaration in the
function body, compiled by GCC into the emitted bytes' address
computations.

## SOTN / community evidence (verbatim, master)

- SOTN `src/boss/mar/CA94.c:23-24` — `// n.b.! unused, required for
  PSP` / `tilemap = &g_Tilemap;` (self-admitted unused alias kept to
  match)
- SOTN `src/st/no0/e_stone_rose.c:611` — `Entity* fakeEntity = self;
  // !FAKE` then `EntityExplosion(fakeEntity);`
- SOTN `src/dra/cd.c:539` — `new_var3 = &g_Cd;` used once where `g_Cd`
  would do
- SOTN repeated idiom: `Tilemap* t = &g_Tilemap;` (`src/dra/4CE2C.c:63`,
  `src/st/cen/e_chamber.c:56,72`), `pLoadFile = &g_LoadFile;`
  (`src/dra/cd.c:646`), `PlayerStatus* status = &g_Status;`
  (`src/boss/mar/clock_room.c:22`)
- SOTN `FakePrim*` reinterpret family — 10+ files incl. `#define
  FAKEPRIM ((FakePrim*)prim)` (`src/dra/84B88.c:582`)
- MGS `source/libdg/divide.c:545-548` — `//TODO: below three lines
  don't seem right but provide fake match` / `ot = (u_long *)SCRPAD_ADDR;
  ot = (u_long *)ot[0]; ot_temp = ot;`

## Strict prerequisites (cheat-reviewer FAILs if any is missing)

1. **Lever-exhaustion is documented** — the direct-global form and
   standard levers were measured-negative first (WIP ledger / commit
   body). If the alias was the first reach, FAIL.
2. **The GCC-pass interaction is named** (address-materialization
   caching, CSE shape, base-register allocation).
3. **Mandatory annotation:** `/* FAKE: <one-line reason> */` or
   `// FAKE: <reason>` at the alias declaration.
4. **Layer-1 + layer-2 cheat-reviewer** per
   [[review-discipline-before-commit]].

## What stays FORBIDDEN (non-extension)

- **`asm("Sym")` alias RENAMES** — `extern T G_v asm("G");` /
  `extern volatile T G_v asm("G");` provide a second SYMBOL-TABLE name,
  not a C-level pointer; zero community analog; unchanged per
  [[inline-asm-injection]] § alias renames.
- **Volatile-cast aliases** (`*(volatile T *)&G`) — governed by the
  volatile catalog ([[inline-asm-policy]] /
  [[legitimate-volatile-interrupt-touched]] /
  [[mmio-volatile-type-level]]), not this rule.
- Non-extension per [[no-new-park-categories]].

## Related

- [[sotn-family-research-2026-07-01]] — evidence base + impact map.
- [[dead-store-fake-exception]], [[named-local-fake-exception]],
  [[mmio-volatile-type-level]] — the other 2026-07-01 rulings.
- [[defeat-combine-symbol-fold]] — the already-legitimate displaced-
  pointer technique this borders.
