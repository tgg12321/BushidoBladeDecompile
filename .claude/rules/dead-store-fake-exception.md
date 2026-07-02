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
- **combine-foldable chain-extender** (scope extension, owner ruling
  2026-07-01 same-day): a LIVE store/computation routed through an
  algebraically-equivalent detour that combine folds back to the direct
  form with ZERO emitted bytes — its only surviving effect is the extra
  `reg_n_refs` count flow.c records before the fold. E.g.
  `D_800F19C0 = (void*)((u8*)tbl_125c + ((s32)&D_80016240 -
  (s32)D_800A125C));` (link-constant delta; folds to
  `D_800F19C0 = &D_80016240;`). SOTN ships the intent-class: the
  wiki-endorsed `(Random() & 3) + 1 - 1` opaque detour and the committed
  `spriteX = four + ptr[0]; ... width = spriteX - four;` round-trip
  (`src/dra/84B88.c:817-884`, `src/dra/7879C.c:3038-3044`). The
  2026-06-02 forbidden classification of this spelling is superseded.
  Extra prerequisite: verify the fold actually emits zero bytes (same
  insn count + no new address materialization) — a chain that
  MATERIALIZES is a real code change, not this lever.

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

## Confirmed closures

- **func_80078EC0** (text1b_b.c, 2026-07-01, the ruling's first application):
  16-insn I_STAT/I_MASK predicate; GCC folds `if (X) return 1; return 0;`
  (jump.c store-flag conversion: single-set 0/1 arms, condition == value)
  to `return X;`, but the target keeps the verbose
  `bnez; li v0,1(delay); move v0,zero` diamond. Closing shape — the dead
  store must be INSIDE the else arm (a two-set arm breaks the conversion's
  single-set precondition); a detached dead store before `return 0;` does
  NOT work (still folds, floor 3):
  ```c
  if ((p[0] & 1) != 0) {
      ret = 1;
  } else {
      ret = 1; /* FAKE: two-set else arm defeats jump.c store-flag fold */
      ret = 0;
  }
  return ret;
  ```
  Retired 2 pins + 5 regfix; SHA1 == oracle; layer-2 reviewer PASS.
  Reusable for any unfolded 0/1 branch-diamond tail (`bnez` + delay-slot
  `li 1` + fall-through `move 0`).

## Related

- [[sotn-family-research-2026-07-01]] — the evidence base + impact map.
- [[register-alloc-pure-c]] — try Levers A/B/C FIRST; this rule is the
  documented last resort behind them.
- [[do-while-zero-exception]] / [[legitimate-volatile-interrupt-touched]]
  — sibling carve-outs, same model (evidence-gated, annotated,
  reviewer-enforced, non-extending).
- [[named-local-fake-exception]], [[pointer-alias-fake-exception]],
  [[mmio-volatile-type-level]] — the other 2026-07-01 rulings.
