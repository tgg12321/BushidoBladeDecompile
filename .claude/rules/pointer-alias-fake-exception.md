---
name: pointer-alias-fake-exception
paths: [".claude/rules/pointer-alias-fake-exception.md"]
description: "SANCTIONED LAST-RESORT (owner 2026-07-01): a C-level local pointer alias to a global (or one forward-order param alias, 2026-07-17) with /* FAKE */ + exhaustion; pointer-RMW on a global allowed. asm(\"sym\") renames forbidden."
metadata:
  type: rule
---

# C-level pointer aliases — the FAKE-annotated exception

**Owner ruling 2026-07-01** (evidence: `pre-slim-2026-10-01:memory/project/sotn-family-research-2026-07-01.md`): a local pointer that provides a
second C handle to a global — where using the global directly would be semantically identical — is a
sanctioned last-resort matching lever under the prerequisites below.

## What is sanctioned (exact scope)

- `Type* t = &g_Thing;` then `t->field` where `g_Thing.field` would do (the redundant-second-handle shape).
- Typed re-views: `s16 (*p)[] = &D_xxx;` array-pointer casts, and FakePrim-style reinterpret-cast pointer
  views of a global buffer.
- Pass-through aliases: `Entity* e2 = self;` used at a call site.

The alias must be ordinary C — a visible local declaration in the function body, compiled by GCC into the
emitted bytes' address computations. SOTN precedent: `src/boss/mar/CA94.c:23-24` (`// n.b.! unused,
required for PSP` / `tilemap = &g_Tilemap;`), `src/st/no0/e_stone_rose.c:611` (`fakeEntity = self;
// !FAKE`), `src/dra/cd.c:539`, FakePrim family (`src/dra/84B88.c:582`).

## Strict prerequisites (cheat-reviewer FAILs if any is missing)

1. **Lever-exhaustion is documented** — the direct-global form and standard levers were measured-negative
   first (ledger / commit body). If the alias was the first reach, FAIL.
2. **The GCC-pass interaction is named** (address-materialization caching, CSE shape, base-register
   allocation).
3. **Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` at the alias
   declaration.
4. **Layer-1 + layer-2 cheat-reviewer** per [[review-discipline-before-commit]].

## Parameter aliases (owner ruling 2026-07-17, hirahira_w_frie)

- **SANCTIONED (last resort, per-use dossier):** a SINGLE forward-order param alias with a mandatory
  `/* FAKE */` annotation, e.g. `s32 *b = base; /* FAKE: prologue pair order — see ledger */`, whose sole
  mechanism is combine merging the now-single-use parameter's entry copy into the alias init at its later
  position. Prerequisites per use, none waivable: (1) measured mechanism-level lever exhaustion — every
  sanctioned axis individually measured dead (cc1 `expand_function_start` param-order emission, sched1
  `rank_for_schedule` LUID tie-break, combine merge as the only C-level relocation); (2) the GCC pass named,
  not asserted; (3) the `/* FAKE */` annotation; (4) default-FAIL Judge + layer-2. Reaching for it without
  the dossier is the ordinary cheat it always was.
- **FORBIDDEN:** literal param renames declared in target's pair order to flip the prologue save+def pair
  order (`Rect *_r = r; s32 *_out = out;` with the body using the aliases). The declaration order is a freely
  tunable prologue-ordering knob; no dossier can sanction it.

## Pointer read-modify-write on a global (user decision 2026-06-10, func_80077894)

Allowed without the FAKE dossier as plausible 1990s C: `s32 *p = &G; cur = *p; /* real work */ *p = cur;` —
a pointer local to a global used for an actual READ-MODIFY-WRITE (at least one load AND one store through
it), neutral or descriptive name, real program logic between load and store. NOT covered: single-use
pointer locals (alias-rename territory), pointers used to defeat CSE/aliasing on SEPARATE objects, chains of
pointer re-spellings of one access. Each new shape gets default-FAIL treatment.

## What stays FORBIDDEN

- **`asm("Sym")` alias RENAMES** — `extern T G_v asm("G");` provide a second SYMBOL-TABLE name, not a
  C-level pointer; zero community analog ([[inline-asm-policy]]).
- **Volatile-cast aliases** (`*(volatile T *)&G`) — governed by the volatile catalog ([[inline-asm-policy]]
  / [[legitimate-volatile-interrupt-touched]] / [[mmio-volatile-type-level]]), not this rule.
- Non-extension per [[no-new-park-categories]].

## Related

[[dead-store-fake-exception]] · [[named-local-fake-exception]] · [[defeat-combine-symbol-fold]] (the
displaced-pointer technique this borders)
