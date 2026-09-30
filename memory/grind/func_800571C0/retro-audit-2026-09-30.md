# func_800571C0 — retro-audit CONCERN (owner Q49), analysis 2026-09-30 (laneG) — open

Archived landing ledger: memory/grind/_completed/func_800571C0/. Concern (retro-audit 2026-09-29 batch_01,
ecfdc52a6): `s8 nr` holds the right-side clear-step count, then is reused as the chosen-side flag
(`nr = 0` / `nr = 1`, read by `if (nr != 0)` in the waypoint loop); the landing said "no FAKE constructs" and
claimed no ruling. Undisclosed reuse = FAIL class (lane brief lesson 3).

Measured (sandbox --disable all, probes-2026-09-30/, landed body 0/287): fresh flag replacing the reuse
`u8 toL` 68 (295/287), `s32 toL` 72 (294/287), `s8 toL` declared before `nl` 47 (295/287) — the auditor's 47.
The reuse saves 8 instructions, so it is load-bearing.

Routes (none executed yet): (1) an ordinary spelling that needs no reuse (e.g. derive the side from the counts
without a separate flag write); (2) a Q51 SOTN citation of a count reused as a direction flag; (3) Ruling 11
package (dumps naming the pass, honest generic name — `nr` fails (E) — annotation, layer-2). If none closes,
the Q37 fallback (revert to INCLUDE_ASM + reopen) goes to the orchestrator first.
