# Project Status (snapshot 2026-10-02)

- **Build:** `main` byte-matches the oracle (`62efab4f73f992798c43e8c730aa43baa10bb4fa`); check with
  `& tools/wteng.ps1 main verify-oracle`.
- **Worklist:** `engine/queue.json` is **empty** (reached zero 2026-10-02). Every function is complete:
  1260 COMPLETED-C + 223 COMPLETED-INLINE-ASM-CANONICAL = 1,483 functions, 0 INCOMPLETE (11 data-as-code
  symbols excluded; 1,483 = 1,437 `asm/funcs` files − 2 non-functions + 45 functions inside a
  neighbour's listing + 3 unlisted — [`naming/README.md`](naming/README.md) "The universe").
  90.8% of C-object `.text` bytes are C; the rest is canonical hand-written / PsyQ-library asm.
  Recount: `python3 tools/check_completion_integrity.py`.
- **Representation:** 135 functions remain `INCLUDE_ASM("asm/funcs", <func>);`, all listed in
  `inline_asm_canonical.txt` (hand-written original asm) — that is their final form. `asm/funcs/*.s`
  (1,437 files) is kept as the reference target listing (48 functions have no file of their own).
- **Grinder:** stopped; restart only on the owner's explicit approval.
- **Open work:** the cleanup / audit / readability phase. Recorded debt and its remaining items:
  [`grind/handoff-2026-09-30.md`](grind/handoff-2026-09-30.md) ("Open work").
- Timeline: [`HISTORY.md`](HISTORY.md). Workflow: [`../CLAUDE.md`](../CLAUDE.md) / [`../AGENTS.md`](../AGENTS.md).
