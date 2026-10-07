# Project Status (snapshot 2026-10-07)

- **Build:** `main` byte-matches the oracle (`62efab4f73f992798c43e8c730aa43baa10bb4fa`); check with
  `& tools/wteng.ps1 main verify-oracle`. Five clean builds (serial and `make -j8`/`-j16`) and an engine
  rebuild gave identical objects on 2026-10-03.
- **Worklist:** `engine/queue.json` is empty: 1260 COMPLETED-C + 224 COMPLETED-INLINE-ASM-CANONICAL =
  1,484 functions (10 data-as-code symbols excluded; 1,484 = 1,438 `asm/funcs` files − 2 non-functions + 45
  functions inside a neighbour's listing + 3 unlisted — [`naming/README.md`](naming/README.md) "The
  universe"). 90.7% of C-object `.text` bytes are C; the rest is canonical hand-written / PsyQ-library asm.
  Recount: `python3 tools/check_completion_integrity.py`.
- **Representation:** 137 completed functions remain `INCLUDE_ASM("asm/funcs", <func>);`, all listed in
  `inline_asm_canonical.txt` (hand-written original asm) — that is their final form. `asm/funcs/*.s`
  (1,438 files) is kept as the reference target listing (48 functions have no file of their own).
- **Layout (Phase 1 restructure, 2026-10-03):** game code in `src/main/`, Sony library code in
  `src/main/psxsdk/<lib>/<module>.c`, headers in `include/psxsdk/`, `include/game.h`, `include/bb2.h`
  (AGENTS.md "Conventions"; old file names: `tools/tu_renames.tsv`).
- **Grinder:** stopped; restart only on the owner's explicit approval.
- **Open work:** the readability phase. Phase 2 (types) is closed: what remains is ruled or debt rows.
  Phase 3 (naming) is in progress under [`.claude/rules/naming-bar.md`](../.claude/rules/naming-bar.md)
  ([`grind/handoff-2026-10-03-restructure.md`](grind/handoff-2026-10-03-restructure.md) item 3); older
  recorded debt: [`grind/handoff-2026-09-30.md`](grind/handoff-2026-09-30.md) "Open work".
- Timeline: [`HISTORY.md`](HISTORY.md). Workflow: [`../CLAUDE.md`](../CLAUDE.md) / [`../AGENTS.md`](../AGENTS.md).
