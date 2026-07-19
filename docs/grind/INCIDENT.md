# GRINDER CIRCUIT-BREAK — 2026-07-19 10:58 — RESOLVED 2026-07-19

**Reason:** queue done refused a judge-PASSed, bytes-proven candidate for tslPolyF4Init

**Root cause:** `tslPolyF4Init` carried a stale `prologue_config.json` entry from
a prior completion. A grind session found a pure-C form that byte-matches WITHOUT
prologue_fix (honest sandbox `--disable all` = 0, which strips prologue_fix), but
`engine.integrate.retire_function` only dropped regfix/asmfix rules
(`CONFIGS = [regfix, regfix_stage2, asmfix]`) — never the prologue_fix configs.
So `queue done`'s completion-integrity gate correctly refused (1 prologue_fix
entry still present), and the driver hard-circuit-broke on the refusal, halting
the whole pipeline. (The `wsl: Failed to start the systemd user session` line in
the log was incidental stderr noise; WSL was functional.)

**Fix (committed 2026-07-19):**
1. `engine/cheats.py`: `drop_prologue_fix_entries(func)` removes a function's
   entries from prologue_config.json / delay_slot_ra / frame_fix in place.
2. `engine/integrate.py`: `retire_function` now drops prologue_fix alongside
   regfix/asmfix and backs those configs up for SHA1-mismatch rollback.
3. `tools/grinder/grind.ps1`: the `Match:` commit stages every retire-touchable
   config (added regfix_stage2 + the 3 prologue configs); a `queue done` refusal
   now banks a constraint and continues instead of circuit-breaking (oracle stays
   green — a refusal is a per-function "needs more work" signal, not corruption).

Engine suite: 178 passed. See memory `retire-drops-prologue-fix`.
