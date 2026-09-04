# GRINDER CIRCUIT-BREAK — 2026-09-03 09:47

**Reason:** 3 consecutive invalid sessions on func_80017848

git HEAD: e613e71a
git status:
```
MM metrics/events.jsonl

```
Last 20 log lines:
```
[grind 2026-09-03 09:37:04] pre-flight: oracle green.
[grind 2026-09-03 09:37:06] pre-flight: WSL bridge enabled (default; BB2_WSL_BRIDGE=0 disables). make stays on the direct path by design.
[grind 2026-09-03 09:37:08] func_80017848: session 30 starting, modality=rederive, model=claude-opus-5[1m]
[grind 2026-09-03 09:37:09] func_80017848: INVALID session output (progress requires >=1 hypothesis with verdict CONFIRMED/KILLED and a numeric measurement in result) — discarded, src reverted, respawning.
[grind 2026-09-03 09:37:09] grinder stopped.
[grind 2026-09-03 09:37:09] grinder starting (pid 42284, model claude-opus-5[1m], judge claude-opus-5[1m])
[grind 2026-09-03 09:37:10] pre-flight: oracle green.
[grind 2026-09-03 09:37:12] pre-flight: WSL bridge enabled (default; BB2_WSL_BRIDGE=0 disables). make stays on the direct path by design.
[grind 2026-09-03 09:37:14] func_80017848: session 30 starting, modality=rederive, model=claude-opus-5[1m]
[grind 2026-09-03 09:37:15] func_80017848: SCOPE VIOLATION —  M Makefile — session discarded.
[grind 2026-09-03 09:37:15] grinder stopped.
[grind 2026-09-03 09:37:25] grinder starting (pid 6548, model claude-opus-5[1m], judge claude-opus-5[1m])
[grind 2026-09-03 09:37:26] pre-flight: oracle green.
[grind 2026-09-03 09:37:28] pre-flight: WSL bridge enabled (default; BB2_WSL_BRIDGE=0 disables). make stays on the direct path by design.
[grind 2026-09-03 09:37:30] func_80017848: session 30 starting, modality=rederive, model=claude-opus-5[1m]
[grind 2026-09-03 09:40:43] func_80017848: INVALID session output (no outcome file / unparseable JSON) — discarded, src reverted, respawning.
[grind 2026-09-03 09:40:45] func_80017848: session 30 starting, modality=rederive, model=claude-opus-5[1m]
[grind 2026-09-03 09:44:01] func_80017848: INVALID session output (no outcome file / unparseable JSON) — discarded, src reverted, respawning.
[grind 2026-09-03 09:44:04] func_80017848: session 30 starting, modality=rederive, model=claude-opus-5[1m]
[grind 2026-09-03 09:47:37] func_80017848: INVALID session output (no outcome file / unparseable JSON) — discarded, src reverted, respawning.
```

---
**RESOLVED 2026-09-04 (owner ruling, decisions.md "foreclosed-bucket re-evaluation", finding 5 / Ruling B.3).** Both 2026-09-03 breaks (08:32, 09:47) were API weather: metrics/events.jsonl `grind-agent-usage` rows for the six discarded s30 spawns show one `API Error: 500` after 30 turns and five `API Error: 529 Overloaded` at one turn / zero tokens, each 190-266 s. `Test-AgentApiError` keyed on `api_error_status`, which the CLI leaves null, so deaths past the 120 s spawn window were miscounted as invalid sessions. The classifier now also matches the status code in the result text. No session work was lost: the first attempt's forms survived in tmp/grind/func_80017848/s30/ and the successful 12:04 s30 re-measured them.
