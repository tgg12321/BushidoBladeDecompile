---
name: dead-store-fake-exception
paths: [".claude/rules/dead-store-fake-exception.md"]
description: "SANCTIONED LAST RESORT (owner 2026-07-01): a dead store / self-assignment to a LOCAL or PARAMETER, annotated /* FAKE: ... */, after documented lever exhaustion. Un-annotated dead stores and pins stay forbidden."
metadata:
  type: rule
---

# Dead stores / self-assignments — the FAKE-annotated exception

**Owner ruling 2026-07-01** (`pre-slim-2026-10-01:memory/project/sotn-family-research-2026-07-01.md`): a dead store or self-assignment
written as ordinary C, targeting a **local variable or function parameter**, is a sanctioned
last-resort matching lever — under the strict prerequisites below.

## What is sanctioned (exact scope)

Ordinary-C assignment statements inside a function body whose stored value is never read (GCC DCEs
the store; its existence influences RA / scheduling / flow analysis upstream of DCE):

- self-assignment: `x = x;`
- dead store to a local: `dest = val1;` where the STORED VALUE is never read — **deadness is
  STORE-level, not variable-level** (owner ruling 2026-08-31, [[ordinary-c-judge-decidable]]
  Ruling 2): the store is in scope even if `dest` is later re-assigned and read (defensive-init
  shape `x = a; ... x = b;` with every read reached only by the second store). This does not by
  itself legalize any previously-FAILed instance; each is adjudicated fresh with every
  prerequisite verified.
- dead conditional store: `if (c) { v = e; } ...; v = e;` (inner store dead)
- dead param assign: `arg0 = 0;` / `param = param;` never read after
- **combine-foldable chain-extender**: a LIVE store/computation routed through an
  algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted
  bytes (only effect: the extra `reg_n_refs` flow.c counts before the fold), e.g.
  `D_800F19C0 = (void*)((u8*)tbl_125c + ((s32)&D_80016240 - (s32)D_800A125C));`. Extra
  prerequisite: verify the fold emits zero bytes (same insn count, no new address
  materialization) — a chain that materializes is a real code change, not this lever.

SOTN evidence: `src/dra/66590.c:390` `dest = val1; // fake`; `src/dra/alu_anim.c:174-175`
`idxSub = idxSub;`; oot `rtile = rtile; // Fake match?`.

## Strict prerequisites (cheat-reviewer FAILs if any is missing)

1. **Lever-exhaustion is documented.** The ledger (or commit body) shows the standard pure-C levers
   were tried and measured-negative: [[register-alloc-pure-c]] Levers A/B/C, structural rewrites,
   permuter from a clean base. If this construct was the FIRST lever reached for, **FAIL**.
2. **The GCC-pass interaction is named** (allocno priority / nrefs, scheduling, DCE-flow) and why
   the natural form can't reach it. No articulated mechanism = noise, not a lever.
3. **Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the
   statement. Un-annotated instances remain forbidden — the detectors (`find_dead_param_assigns`,
   `find_dead_conditional_stores`) skip ONLY annotated instances.
4. **Layer-1 + layer-2 cheat-reviewer** per [[review-discipline-before-commit]], verifying 1-3
   against the actual source and ledger, default FAIL.

## What stays FORBIDDEN

- **Register-asm pins** — a dead store paired with a pin still FAILs on the pin.
- **Hardcoded-`$N` `__asm__` injection, scheduling barriers, build-time rewriting**
  ([[inline-asm-policy]]).
- **Unused/oversized-array frame coercion** — [[dead-vars-local-array]]; this rule covers STORES,
  not declarations ([[named-local-fake-exception]] covers scalar locals).
- **Dead stores to globals** — observable behaviour, out of scope.
- No relaxation of "cheats by any spelling" for any other construct ([[no-new-park-categories]]).

## Example (func_80078EC0)

GCC folds `if (X) return 1; return 0;` (jump.c store-flag conversion) to `return X;`; target keeps
the `bnez; li v0,1; move v0,zero` diamond. A two-set else arm breaks the single-set precondition (a
detached dead store before `return 0;` does NOT work):

```c
if ((p[0] & 1) != 0) {
    ret = 1;
} else {
    ret = 1; /* FAKE: two-set else arm defeats jump.c store-flag fold */
    ret = 0;
}
return ret;
```

Related: [[register-alloc-pure-c]] (try first) · [[do-while-zero-exception]] ·
[[named-local-fake-exception]] · [[pointer-alias-fake-exception]]
