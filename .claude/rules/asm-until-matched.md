---
name: asm-until-matched
paths: ["src/*.c", "tools/grinder/**", "engine/queue.py"]
description: "Owner ruling 2026-08-19: no cheat on main in any form; an unfinished function is committed as INCLUDE_ASM(\"asm/funcs\", <func>); C lands once, at COMPLETED-C; candidates live in memory/grind/<func>/."
metadata:
  type: rule
---

# ASM-until-matched (owner ruling 2026-08-19)

1. **Main carries no cheats, in any spelling, for any INCOMPLETE function** — no pins, no cheat-asm, no
   coercion constructs, no draft C. "Not yet decompiled" is committed as exactly
   `INCLUDE_ASM("asm/funcs", <func>);`.
2. **C lands on main once per function, at COMPLETED-C**, through the normal gates (sandbox 0 → layer-1 /
   Judge or layer-2 → verify-oracle → `queue done`). No intermediate committed state.
3. **In-progress work lives in the ledger** `memory/grind/<func>/` (`candidate.c` = working frontier;
   `migration_pin.json` = honest floor used for queue ordering while the body is INCLUDE_ASM).
4. **Build-time assembly rewriting is gone** (regfix/asmfix removed 2026-08-30) and may not be reintroduced.

Ladder retune adopted with this ruling: one synthesis pass at s6; at most 2 permuter sessions per function;
the driver records the closing session's modality in the journal and the Match commit. Exhaustion handling is
now [[rotation-not-foreclosure]].

Related: [[judge-sole-gate]] ·
[[rotation-not-foreclosure]]
