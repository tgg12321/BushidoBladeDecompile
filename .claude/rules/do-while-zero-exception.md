---
name: do-while-zero-exception
paths: [".claude/rules/do-while-zero-exception.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification. Hard line unchanged: no regfix/pins/inline-asm/semantic-lie C."
metadata:
  type: rule
---

# do-while(0) as a match device — the construct-honesty line

## Owner ruling 2026-07-06 (final; supersedes the 2026-06-04 mechanism-scoping
## and the same-day "Declined extension" interim ruling)

The owner's criterion, verbatim in substance: *materially-irrelevant minor
tricks that help the compiler are OK; what is NOT OK is regfix, pins, or
inline asm used to make something appear decompiled when it really isn't.*

Applied to this construct and grounded in the full SOTN evidence base
([[sotn-do-while-zero-research-2026-06-04]], [[sotn-borderline-research-2026-06-02]],
[[sotn-family-research-2026-07-01]]):

**`do { <any body> } while (0);` — including empty bodies — is a sanctioned
pure-C match device for ANY codegen effect, including register allocation.**
The former scoping to the reorg.c label-note mechanism is abolished: it had
no SOTN basis (SOTN's acceptance is construct-level; their reviewers merge
wraps on sight without asking which GCC pass they bend, and SOTN ships
`// FAKE but makes register allocation work` in master), and the mechanism
a wrap influences is invisible at source level anyway.

## The line (project-wide restatement)

**FORBIDDEN — bytes or register outcomes imposed from OUTSIDE compiled C**
("appears decompiled when it really isn't"):
1. regfix/asmfix rules (bytes from rule text)
2. register-asm pins (`register T x asm("$N")`)
3. `__asm__` in any form except the canonical hand-written-asm/GTE category
   ([[inline-asm-allowed]])
4. compiler/toolchain divergence ([[no-compiler-divergence]])
5. **semantic-lie C**: legal C that asserts FALSE program facts — cross-symbol
   address derivation (2026-07-05 ruling STANDS), volatile coercion / alias
   second-handles ([[inline-asm-policy]] catalog; a deliberate
   stricter-than-SOTN BB2 choice, unchanged). These compile, but they make the
   decompiled source a false document.

**ALLOWED — any spelling of semantically-TRUE C**, whatever pass it nudges:
do-while(0) wraps, split/redundant arithmetic (the SOTN-wiki `+ 1 - 1` class —
note this places the 2026-07-05 double-split rejection's SPELLING half under
the allowed side; its cross-symbol half stays forbidden under #5), variable
reuse/staging, named intermediates, statement order, mixed exit forms.
Marked with the FAKE convention (below) when purely-for-matching.

## Prerequisites for do-while(0) use

1. **Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site**
   (not file-header prose), naming the observed effect (e.g. "loop-note ref
   weighting seats tbl in s5"). Effect, not internals, is enough — SOTN's own
   annotations are effect-level.
2. **Prefer natural geometry first.** The wrap is still a match device, not a
   first resort; a natural statement-placement close-out is always the better
   form. But exhaustion is no longer a hard gate for SINGLE-LEVEL wraps —
   SOTN ships them routinely, and forcing multi-session grinds before
   permitting a device the reference project uses freely was mis-calibrated.
3. **Nested wraps (`do { do { ... } while(0); } while(0)`) need a written
   justification** in the annotation or WIP notes that a single level was
   measured insufficient (nesting is a weight multiplier; no direct SOTN
   citation exists for nested forms — the sanction rests on the owner's
   construct-honesty criterion plus the macro-idiom plausibility argument,
   so it carries the extra documentation duty).

## Why the construct is on the honest side of the line

- Every byte still comes from the pristine compiler consuming legal C; the
  committed source IS a true compilable origin of the binary.
- The wrap asserts nothing false — "this body executes once" is true.
- It is the canonical C macro-body idiom of the era; PsyQ-period codebases
  are full of `do { ... } while (0)` macro expansions, and macro expansions
  produced EXACTLY these loop notes in original binaries. A wrap in a
  reconstruction is indistinguishable from an original macro.
- It is transparent: greppable, annotated, never disguised as program logic.

## Evidence summary (see the three memos for citations)

- 18+ SOTN master instances incl. EMPTY bodies and one-line-store bodies
  (`do { var = 0; } while (0);` — w_045.c); merged with comments like
  "I saw that it is in other parts of the repo as well".
- `// FAKE but makes register allocation work` ships verbatim (w_037.c) —
  SOTN accepts RA-purposed coercion C by name.
- Full-tree census (2026-07-01): ~34 genuine coercion-FAKE sites; ZERO
  non-compiled-bytes mechanisms anywhere — the universal community line is
  exactly the owner's line.
- Nested `do { do {` : no SOTN hits found (gh search 2026-07-06) — hence
  prerequisite 3.

## Confirmed applications

- **`cpu_check_same_dir_timer`** (cf3e6ce7, 2026-06-04) — reorg.c
  branch-sense suppression; the original narrow use case.
- **`marionation_Exec`** (2026-07-06, work/marion) — RA-weighting wraps
  (do_timeout→tbl/s5, poll→i1494/i1495, nested clear→i1496, tail
  label-prediction wrap); the case that prompted this ruling. The masked-4
  candidate was reviewer-FAILed under the old mechanism-scoping, measured
  honest under the construct line (wraps = the whole masked 30→4), and
  reinstated by the owner ruling.

## Related

- [[no-new-park-categories]] — the "cheats by any spelling" policy; its
  target is now precisely the FORBIDDEN list above (non-compiled bytes +
  semantic lies), not spelling-diversity of true C.
- [[sotn-do-while-zero-research-2026-06-04]] /
  [[sotn-borderline-research-2026-06-02]] /
  [[sotn-family-research-2026-07-01]] — the evidence base.
- [[review-discipline-before-commit]] — reviewer architecture; family checks
  should apply THIS rule's line to wrap constructs.
- [[inline-asm-policy]] / [[inline-asm-allowed]] — the unchanged hard bans.
