# Contributing to the BB2 Decompilation

The goal is C source that compiles, through the pinned GCC 2.7.2 + maspsx toolchain, to a
**byte-identical** copy of `SLUS_006.63` (SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`).
Most contributions are one function at a time.

Read, in order:

1. [`BUILD.md`](BUILD.md) — get a green build (`OK: bb2 matches!`). Nothing else matters until the oracle passes.
2. [`docs/DECOMP_WORKFLOW.md`](docs/DECOMP_WORKFLOW.md) — the operating manual: the three function states
   (INCOMPLETE / COMPLETED-C / COMPLETED-INLINE-ASM-CANONICAL), the per-function engine loop, what counts as a
   cheat, and the mandatory adversarial review before any completion commit.
3. [`AGENTS.md`](AGENTS.md) — toolchain and build facts, file-edit conventions (build files are LF).
4. [`docs/MATCHING.md`](docs/MATCHING.md) and `.claude/rules/codegen-technique-index.md` — techniques.

The essentials:

- **One worklist:** `engine/queue.json`, pre-ordered easiest-first. Take the top item
  (`& tools/wteng.ps1 main queue next`); no cherry-picking, no deferral — a stuck item changes modality,
  never target.
- **Done means** pure C (or owner-authorized canonical asm), zero cheats, and full-build SHA1 == oracle.
  Register pins, injected `__asm__`, scheduling barriers and compiler patches are never a completion.
- **Every completion** passes a fresh default-FAIL `cheat-reviewer` (manual path) or the Grinder's Judge.
- **Commits** follow [`docs/COMMIT_CONVENTIONS.md`](docs/COMMIT_CONVENTIONS.md); multi-line messages via
  `git commit -F <file>`.
- **Don't break the substrate:** cc1, maspsx, `prologue_fix`, `multu_pad`, splat, the Makefile, `bb2.ld`
  (hand-maintained — never run `make setup` on an existing tree), `asm/`, `include/`, the `*.txt` configs.
