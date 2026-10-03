---
name: completion-bar
paths: ["src/**/*.c", "include/*.h", "memory/grind/**", ".claude/rules/*.md", "engine/queue.py", "tools/grinder/roles/*.md"]
description: "BLOCKING (owner ruling Q91, 2026-10-02): the whole completion bar, SOTN-equivalent. Byte match + the wall + honest code (every no-semantic-purpose construct /* FAKE */-annotated with its measured reason, no false claims) + existing types + simplest form. A FAIL must cite an item here; everything else is hygiene or technique."
metadata:
  type: rule
  tier: blocking
---

# The completion bar (owner ruling Q91, 2026-10-02)

The bar is what SOTN enforces: the code matches, nothing lies, and every hack is labelled.
This file is the **whole** blocking tier. A review FAIL (layer-2 `cheat-reviewer`, Grinder
Judge) must cite one of items 1-6 below; a FAIL resting only on hygiene-tier paperwork is not
a FAIL. Rule files carry `tier: blocking | hygiene | technique` in their frontmatter;
only `blocking` files can decide a completion, and every file in the Blocking list below is
blocking whether or not it is tagged yet. Other files linked here bind only through the
item that links them (e.g. [[phantom-frame-pad-family]]'s allowlist row and label).

## Items

1. **Byte match.** `sandbox --disable all` = 0 with cheat-asm stripped, and the full build's
   SHA1 == oracle (`verify-oracle --rebuild`).
2. **The wall** (Ruling 13 (D), as narrowed by Q91). No register pins; no hardcoded-`$N` or
   non-canonical `__asm__` ([[inline-asm-policy]]); no scheduling barriers; no build,
   Makefile, linker, gate or compiler change that alters bytes ([[no-compiler-divergence]]);
   no build-time assembly rewriting or prebuilt `.o` / asm substitution; no `asm("Sym")` alias
   renames — in any spelling (lowercase `asm`, `__asm`, macros, any new stage, list or
   substitution). No annotation admits any of these; the only exception is Q55 below.
3. **Honest code.** Read by a programmer, the C says what the function does.
   - A construct that exists only to shape codegen (no semantic purpose) carries
     `/* FAKE: <why> */` (or `// !FAKE`) at the site, with a one-line measured reason
     ("the literal moves `li -16` two slots; score 2"). Annotated, it is admissible. The
     pre-cleared shapes in [[no-new-park-categories]] are examples, not the boundary.
   - Comments, names, struct fields and prototypes assert nothing false: no invented object
     model, no claim the callees do not bear out. If the reason is the frame, say
     `FAKE: frame layout`. A no-purpose local may not wear a name that claims a purpose.
   - Refused even when annotated: fabricated calls or side effects the program does not
     perform (`if (0) { f(); }`); cross-symbol address derivation (one symbol's bytes reached
     through another's address; the per-function admissions in
     [[aggregate-merge-family]] stand); `volatile` outside
     its catalog — every route these files admit and nothing else: the hardware range
     ([[mmio-volatile-type-level]]); the IRQ-touched extern allowlist and the volatile-locals
     Routes A/B ([[legitimate-volatile-interrupt-touched]]); the phantom-frame pad
     ([[phantom-frame-pad-family]]); detector-stripped frame coercion
     (`(void)&local`, lost-codegen inserts, unreferenced local arrays without their
     `engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS` row) — it cannot satisfy item 1.
   - **Q55 stands (not changed by Q91):** a verified SOTN citation ([[sotn-precedent-suffices]],
     its conditions met) can still admit a construct items 2-3 refuse, as before: manual path
     only (never on a Judge PASS), `/* FAKE */`-labelled, simplest form, fresh layer-2. That
     file's conditions (1)-(4) and its `/* SOTN: */` tag are blocking for this route.
4. **Existing types.** When a header already declares the object (struct, array, typed
   extern), access it through that declaration: no second extern, alias symbol or
   raw-offset cast for the same bytes (per-function exceptions Q98/Q99:
   [[aggregate-merge-family]]). A new aggregate the function needs goes in the shared
   header, not at block scope.
5. **Simplest known form.** Of the byte-exact spellings known, land the one with the fewest
   FAKE annotations; ablate whole clusters, not single pieces (`docs/DECOMP_WORKFLOW.md` §7).
6. **One fresh adversarial review** (layer-2, recorded with `layer2 record`) judging 2-5 on
   the landed body ([[review-discipline-before-commit]]).

## The tiers

- **Blocking** — this file; the files it links for items 2, 3 and 6 ([[inline-asm-policy]],
  [[no-compiler-divergence]], [[mmio-volatile-type-level]],
  [[legitimate-volatile-interrupt-touched]], [[review-discipline-before-commit]]); and
  [[no-new-park-categories]] / [[ordinary-c-judge-decidable]] as read through this file.
- **Hygiene** — good practice, worked as codebase-wide cleanup, never a completion gate.
  Skipping one is recorded as a debt row in the landing commit body (`Hygiene debt:` line).
  Includes: lever-exhaustion dossiers and named-GCC-pass write-ups beyond the one-line FAKE
  reason; frame-math proofs, range annotations and sibling evidence for stack arrays;
  splat-symbol retirement in `named_syms.txt` / `undefined_syms_auto.txt`; per-site
  cast/offset certification; the reused-local Rulings 5-12 dossiers (a reused local with no
  semantic reading is simply a FAKE under item 3).
- **Technique** — codegen recipes and diagnosis guides ([[codegen-technique-index]]). They
  say how to reach a match, never whether it may land.

## Application

- **Direction (owner, 2026-10-03):** the project is fully decompiled; the owner is retiring
  special rules. Owner rulings and cleanup work choose the option closer to a faithful
  representation of the original source; new exceptions come only by owner ruling, as interim
  per-function labels, never precedent for a class. Not a review criterion: a FAIL still cites
  items 1-6.
- Uniform from 2026-10-02. COMPLETED functions are not re-audited against the hygiene tier;
  their existing debt (e.g. `memory/grind/judge-decl-cleanup/` disposition rows) stays there.
- **Re-opened (clause D):** per-function refusals that rested on hygiene grounds or on
  "outside the frozen list" alone, including Q45/Q77 (func_8005C8A8), Q66 (func_800770B8),
  Q84 (camera_CalcAngles) and the func_80020E74 / _SsVmFlush rotations. Re-judge them under
  items 1-6. Refusals resting on item 2 stand.
- The narrow per-function admissions Q76, Q82 and Q90 ([[ordinary-c-judge-decidable]]) are
  instances of item 3 and need no ruling of their own going forward.

Related: [[no-new-park-categories]] · [[ordinary-c-judge-decidable]] ·
[[review-discipline-before-commit]] · [[judge-sole-gate]]
