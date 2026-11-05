# Project Status (snapshot 2026-11-05)

- **Build:** `main` byte-matches the oracle (`62efab4f73f992798c43e8c730aa43baa10bb4fa`); check with
  `& tools/wteng.ps1 main verify-oracle`. Five clean builds (serial and `make -j8`/`-j16`) and an engine
  rebuild gave identical objects on 2026-10-03; `verify-oracle --rebuild` matched again 2026-11-05.
  Lane gates build in a WSL mirror, so main's own `build/` goes stale: verify with `--rebuild`.
- **Worklist:** `engine/queue.json` is empty: 1260 COMPLETED-C + 224 COMPLETED-INLINE-ASM-CANONICAL =
  1,484 functions (10 data-as-code symbols excluded; 1,484 = 1,437 `asm/funcs` files − 1 non-function + 45
  functions inside a neighbour's listing + 3 unlisted — [`naming/README.md`](naming/README.md) "The
  universe"). 90.7% of C-object `.text` bytes are C; the rest is canonical hand-written / PsyQ-library asm.
  Recount: `python3 tools/check_completion_integrity.py`.
- **Representation:** 137 completed functions remain `INCLUDE_ASM("asm/funcs", <func>);`, all listed in
  `inline_asm_canonical.txt` (hand-written original asm) — that is their final form. `asm/funcs/*.s`
  (1,437 files) is kept as the reference target listing (48 functions have no file of their own).
- **Layout (Phase 1 restructure, 2026-10-03):** game code in `src/main/`, Sony library code in
  `src/main/psxsdk/<lib>/<module>.c`, headers in `include/psxsdk/`, `include/game.h`, `include/bb2.h`
  (AGENTS.md "Conventions"; old file names: `tools/tu_renames.tsv`).
- **Grinder:** stopped; restart only on the owner's explicit approval.
- **Open work:** the readability phase, run as parallel lanes (`/lanes`, `tools/lanes/`;
  backlogs `docs/naming/phase3/backlog.tsv`, `docs/foundation/backlog.tsv`). Phase 2 (types) is
  closed. Phase 3 (naming, [`naming-bar`](../.claude/rules/naming-bar.md)) is in its trace-fed tail:
  unnamed at 2026-11-03: 53 game `func_` (all in `phase3/no_name.tsv`; up to 6 aggregates reopened
  under Q134), 336 `D_8` globals, 163 members, 402 locals; phase 4 runtime traces
  (`docs/naming/runtime/`) feed it. Readability phase 5 marks the structurally neutral placeholders final;
  phase 6 (constants / enums, owner ruling Q135,
  [`constants-bar`](../.claude/rules/constants-bar.md)) is the `const` lane. 245 `/* FAKE */`
  comments remain (all meet the completion bar; the `fake` lane is paused). Plan and history: the
  latest `docs: handoff` commit body.
- Timeline: [`HISTORY.md`](HISTORY.md). Workflow: [`../CLAUDE.md`](../CLAUDE.md) / [`../AGENTS.md`](../AGENTS.md).
