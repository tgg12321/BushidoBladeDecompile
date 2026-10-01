---
name: header-type-correction-from-use-sites
paths: [".claude/rules/header-type-correction-from-use-sites.md"]
description: "Owner ruling 2026-07-13: a global's SIGNEDNESS may be corrected at its single canonical extern when use-site evidence proves the header mistyped. Four hard prongs (a-d), layer-2 re-verifies each."
metadata:
  type: rule
---

# Header type correction from use-site evidence

A global's declared signedness may be corrected at its single `extern` in a shared header when independent
use-site evidence proves the header was mistyped; the correction removes compensating casts that were doing
semantic work under the wrong type. Sharing the GCC-fold mechanism of banned local coercions
(`convert_for_assignment`, c-typeck.c:3987) is NOT disqualifying — the test is the four prongs.

## The four prongs (all mandatory)

- **(a) Codebase-wide consistency + positive evidence.** `grep -n <symbol>` over `src/` and `include/` shows
  every use site consistent with the new type, AND at least one site exhibits semantics dead or wrong under the
  OLD type (compare against a negative literal, sign-extending widening read, depended-on arithmetic shift, the
  `if (x < 0) x += C; x >>= k` idiom). Mere absence of contradiction fails.
- **(b) Functionally necessary compensating casts, predating the session.** Removing each cast under the OLD
  type must change runtime behaviour (per-cast WITH/WITHOUT analysis); warning-silencing or neutral casts are
  not evidence. `git blame` must show the casts predate the residual-chasing session.
- **(c) One canonical `extern` edit in a shared header.** Never an alias rename (`asm("Y")`), per-use pointer
  pun, local typedef override or macro.
- **(d) Casts eliminated at ALL use sites.** Any site still needing old-type behaviour means it's a per-site
  coercion in disguise — reject.

Layer-2 runs the grep and the cast-necessity table itself, against the actual tree.

## Scope limits

Signedness only — no width flips, no scalar ↔ struct/union/pointer changes (those need their own evidence,
e.g. the aggregate-merge entry of [[no-new-park-categories]]). Not a "flip a header type to close a byte"
license: the residual is never evidence. Grind sessions may not edit `include/*.h` without the four-prong pack.

Example: func_8001B138 — `extern u16 g_file_vram_timer;` → `s16`; the body's `if (g < -0x1C00)` clamp was dead
under `u16` without its pre-existing `(s16)` casts; all five casts removed (commit 4598a98e).

Related: [[mmio-volatile-type-level]] · [[inline-asm-policy]] · [[review-discipline-before-commit]]
