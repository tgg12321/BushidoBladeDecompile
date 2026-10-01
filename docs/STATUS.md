# Project Status (snapshot 2026-10-01)

- **Build:** `main` byte-matches the oracle (`62efab4f73f992798c43e8c730aa43baa10bb4fa`); check with
  `& tools/wteng.ps1 main verify-oracle`.
- **Worklist:** `engine/queue.json` held 27 INCOMPLETE items (15 active, 12 rotated) on 2026-10-01, three of
  them re-queued by the 2026-10-01 inline-asm audit ([`audits/INLINE-ASM-AUDIT-2026-10-01.md`](audits/INLINE-ASM-AUDIT-2026-10-01.md)).
  Live: `queue status` / `queue next`. Completion counts: `python3 tools/check_completion_integrity.py`.
- **Grinder:** stopped; restart only on the owner's explicit approval.
- **Open work and debt:** [`grind/handoff-2026-09-30.md`](grind/handoff-2026-09-30.md).
- **Representation:** an INCOMPLETE function is `INCLUDE_ASM("asm/funcs", <func>);` (asm-until-matched,
  2026-08-19); candidates live in `memory/grind/<func>/`. No post-pass rule machinery exists.
- Timeline: [`HISTORY.md`](HISTORY.md). Workflow: [`../CLAUDE.md`](../CLAUDE.md) / [`../AGENTS.md`](../AGENTS.md).
