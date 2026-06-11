---
name: pointer-rmw-global-sanctioned
description: User-sanctioned 2026-06-10 — the zero-displacement pointer-local RMW spelling (s32 *p = &G; cur = *p; ...; *p = cur;) in func_80077894; narrow precedent, retro-audit FAIL overridden by user decision
paths: ["src/*.c"]
---

# The zero-displacement pointer-RMW spelling — user-sanctioned (narrow)

**Status:** the retroactive adversarial audit (2026-06-10) FAILED the closing
form of `func_80077894` (commit `7b9ddcea`) on the construct:

```c
s32 *p = &D_8009BD38;
cur = *p;
/* ... mask/or work ... */
*p = cur;
```

— a zero-displacement pointer alias to a plain global, judged to fail the
semantic-purpose test (the pointer IS the global) and the GCC-internals test
(chosen because GCC otherwise materializes the lui/addiu address pair twice
where target CSEs it into `$a0`), and to fall outside the
[[defeat-combine-symbol-fold]] carve-out (which requires a DISPLACED access +
an intervening call).

**USER DECISION 2026-06-10: sanctioned.** The pointer-RMW spelling is accepted
as plausible 1990s C — read-modify-write through a pointer local is an
idiomatic form some programmers use even for plain globals, and the commit's
direction was strongly cheat-reducing (6 register pins + a do-while(0) FAKE +
1 regfix rule retired). The commit stands; `func_80077894` remains
COMPLETED-C.

## Scope — what this does and does not sanction

- Sanctioned: a pointer local to a global used for an actual
  READ-MODIFY-WRITE sequence (load through it, compute, store through it) —
  the pointer has at least one load AND one store use. The name must be
  neutral (`p`, or descriptive); the body work between load and store must be
  real program logic.
- NOT sanctioned: pointer locals with a single use (pure alias-rename
  territory, see [[inline-asm-injection]] § alias renames); pointer locals to
  defeat CSE/aliasing on SEPARATE objects; chains of pointer re-spellings of
  the same access. Each new shape gets the default-FAIL treatment.
- This is a USER-DECISION precedent (like [[do-while-zero-exception]] /
  [[proven-spelling-class-reconstruction]]), recorded so the cheat-reviewer
  treats the exact RMW shape as ALLOWED instead of re-flagging it. It does
  NOT lower the bar for other pointer-alias spellings.

## Related

- [[defeat-combine-symbol-fold]] — the displaced-pointer carve-out this shape
  falls outside of (mechanically); the user decision covers the
  zero-displacement RMW case specifically.
- [[no-new-park-categories]] — the cheats-by-any-spelling parent policy;
  posture unchanged outside this narrow precedent.
- [[review-discipline-before-commit]] — reviewer architecture; the
  retroactive-audit FAIL + user-override flow that produced this rule.
