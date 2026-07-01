---
name: dead-store-fake-exception
paths: [".claude/rules/dead-store-fake-exception.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a dead store / self-assignment to a LOCAL or PARAMETER, annotated `/* FAKE: ... */`, is allowed as a LAST-RESORT codegen lever after documented lever-exhaustion. SOTN ships `dest = val1; // fake`, `idxSub = idxSub;`; oot ships `rtile = rtile; // Fake match?`. Un-annotated dead stores remain forbidden; register pins remain forbidden."
metadata:
  type: rule
---

# Dead stores / self-assignments — the FAKE-annotated exception (Lever D unsealed, narrowly)

**Owner ruling 2026-07-01** (evidence base:
[[sotn-family-research-2026-07-01]]): a dead store or self-assignment
written as ordinary C, targeting a **local variable or function
parameter**, is a sanctioned last-resort matching lever — under the
strict prerequisites below. This supersedes the 2026-05-31 / 2026-06-01
blanket prohibition recorded in [[register-alloc-pure-c]] Lever D and
[[no-new-park-categories]] (whose "SOTN's bar rejects them" rationale
the 2026-07-01 census disproved: SOTN master ships the construct,
annotated).

## What is sanctioned (exact scope)

Ordinary-C assignment statements inside a function body whose stored
value is never read (GCC DCEs the store; its existence influences RA /
scheduling / flow analysis upstream of DCE):

- self-assignment: `x = x;`
- dead store to a local: `dest = val1;` where `dest` is never read
- dead conditional store: `if (c) { v = e; } ...; v = e;` (inner store dead)
- dead param assign: `arg0 = 0;` / `param = param;` never read after

## SOTN / community evidence (verbatim, master branches)

- SOTN `src/dra/66590.c:390` — `dest = val1; // fake` (never read)
- SOTN `src/dra/alu_anim.c:174-175` — `// Probably fake. Makes PSP
  registers match.` / `idxSub = idxSub;`
- SOTN `src/dra/menu.c:1993,2009,2017` — `j = menu->unk1D; // FAKE?`
- SOTN `src/weapon/w_049.c:293` — `i = 0; // FAKE? why here?`
- SOTN `src/st/no0/clock_room.c:309` — `primIndex = g_Player.status; // FAKE`
- zeldaret/oot `src/code/PreRender.c:493,500` — `rtile = rtile; // Fake match?`
- MGS `source/weapon/socom.c:89-91` — `// TODO: Fake match/hack no reason
  to assign these again but no match if we don't`

## Strict prerequisites (cheat-reviewer FAILs if any is missing)

1. **Lever-exhaustion is documented.** The WIP entry's `rejected_forms`
   / `sessions[]` ledger (or the commit body for single-session work)
   shows the standard pure-C levers were tried and measured-negative:
   [[register-alloc-pure-c]] Levers A/B/C, structural rewrites, permuter
   from a clean base. If this construct was the FIRST lever reached for,
   **FAIL**.
2. **The GCC-pass interaction is named.** The worker states which
   analysis the dead store influences (allocno priority / nrefs,
   scheduling, DCE-flow) and why the natural form can't reach it. A
   dead store with no articulated mechanism is noise, not a lever.
3. **Mandatory annotation:** `/* FAKE: <one-line reason> */` or
   `// FAKE: <reason>` ON the statement. Un-annotated instances remain
   forbidden — the engine detectors (`find_dead_param_assigns`,
   `find_dead_conditional_stores`) skip ONLY annotated instances, and
   the annotation is the transparency artifact the SOTN convention
   requires.
4. **Layer-1 + layer-2 cheat-reviewer** per
   [[review-discipline-before-commit]] — the reviewer verifies 1-3
   against the actual source and ledger, defaulting to FAIL.

## What stays FORBIDDEN (non-extension)

- **Register-asm pins** (`register T x asm("$N")`) — zero community
  precedent (re-verified 2026-07-01 across SOTN/oot/papermario/MGS/esa).
  A dead store PAIRED with a pin (the func_80078EC0 "Variant F" shape)
  still FAILs on the pin.
- **Hardcoded-`$N` `__asm__` injection, scheduling barriers, regfix
  inserts** — unchanged ([[inline-asm-injection]],
  [[lost-codegen-insert-cheat]]).
- **Unused-array / oversized-array frame coercion** —
  [[dead-vars-local-array]] unchanged; this rule covers STORES, not
  declarations (see [[named-local-fake-exception]] for the scalar-local
  scope).
- **Dead stores to globals** — a store to a global is observable
  program behavior, not a dead store; out of scope.
- This exception does NOT relax "cheats by any spelling" for any other
  construct. Next borderline construct → default FAIL + its own SOTN
  evidence bar per [[no-new-park-categories]].

## Related

- [[sotn-family-research-2026-07-01]] — the evidence base + impact map.
- [[register-alloc-pure-c]] — try Levers A/B/C FIRST; this rule is the
  documented last resort behind them.
- [[do-while-zero-exception]] / [[legitimate-volatile-interrupt-touched]]
  — sibling carve-outs, same model (evidence-gated, annotated,
  reviewer-enforced, non-extending).
- [[named-local-fake-exception]], [[pointer-alias-fake-exception]],
  [[mmio-volatile-type-level]] — the other 2026-07-01 rulings.
