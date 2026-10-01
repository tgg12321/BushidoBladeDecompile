---
name: do-while-zero-exception
paths: [".claude/rules/do-while-zero-exception.md"]
description: "SANCTIONED (owner 2026-07-06): `do { ... } while (0);` (any body, incl. empty) is a pure-C match device for ANY codegen effect incl. RA; mandatory inline FAKE annotation; nested wraps need a written justification."
metadata:
  type: rule
---

# do-while(0) as a match device — the construct-honesty line

**Owner ruling 2026-07-06** (supersedes the 2026-06-04 reorg.c/LABEL_OUTSIDE_LOOP_P-only scoping —
that scoping is abolished and must not be cited as a FAIL ground). Owner criterion, in substance:
materially-irrelevant minor tricks that help the compiler are OK; regfix, pins, or inline asm used
to make something appear decompiled when it isn't are NOT.

**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for
ANY codegen effect, including register allocation.** Evidence: 18+ SOTN master instances incl.
empty and one-store bodies (`w_045.c`), PR merges accepting it; SOTN ships
`// FAKE but makes register allocation work` (w_037.c). See
`pre-slim-2026-10-01:memory/project/sotn-do-while-zero-research-2026-06-04.md`, `pre-slim-2026-10-01:memory/project/sotn-family-research-2026-07-01.md`.

## The line

**FORBIDDEN — bytes or register outcomes imposed from OUTSIDE compiled C:**
1. build-time output rewriting (the retired regfix/asmfix class, any spelling)
2. register-asm pins (`register T x asm("$N")`)
3. `__asm__` in any form except the canonical hand-written-asm/GTE category ([[inline-asm-policy]])
4. compiler/toolchain divergence ([[no-compiler-divergence]])
5. **semantic-lie C**: legal C asserting FALSE program facts — cross-symbol address derivation
   (2026-07-05 ruling stands), volatile coercion / alias second-handles ([[inline-asm-policy]]).

**ALLOWED — any spelling of semantically-TRUE C**, whatever pass it nudges: do-while(0) wraps,
split/redundant arithmetic (the SOTN-wiki `+ 1 - 1` class), variable reuse/staging, named
intermediates, statement order, mixed exit forms — FAKE-marked when purely for matching, and each
still subject to its own family entry in [[no-new-park-categories]]. Other wrapper equivalents
(`for (i=0;i<1;i++)`, `while(1){...;break;}`, `if (1) {}`) are NOT sanctioned by this rule.

## Prerequisites

1. **Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site** (not file-header
   prose), naming the observed effect (e.g. "loop-note ref weighting seats tbl in s5").
2. **Prefer natural geometry first.** The wrap is a match device, not a first resort; but
   exhaustion is not a hard gate for SINGLE-LEVEL wraps.
3. **Nested wraps (`do { do { ... } while(0); } while(0)`) need a written justification** in the
   annotation or ledger that a single level was measured insufficient (no SOTN citation exists for
   nesting).

## Why it is honest

Every byte still comes from the pristine compiler consuming legal C; "this body executes once" is
true; it is the era's canonical macro-body idiom (macro expansions produced exactly these loop notes
in original binaries); it is greppable and annotated.

Example (cpu_check_same_dir_timer): a wrap around a block suppresses a reorg.c branch-sense flip.

Related: [[no-new-park-categories]] · [[review-discipline-before-commit]] · [[inline-asm-policy]]
