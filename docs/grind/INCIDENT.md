# GRINDER CIRCUIT-BREAK — 2026-07-13 20:47

**Reason:** 3 consecutive invalid sessions on func_8003D7B4

git HEAD: eb791bdb
git status:
```
M  metrics/events.jsonl
?? memory/grind/func_8003D7B4/candidate.c
?? memory/grind/func_8003D7B4/rejected/

```
Last 20 log lines:
```
[grind 2026-07-13 19:43:57] grinder stopped.
[grind 2026-07-13 19:43:58] grinder starting (pid 52668, model fable, judge fable)
[grind 2026-07-13 19:43:59] pre-flight: oracle green.
[grind 2026-07-13 19:44:00] func_80035480: session 2 starting, modality=structural
[grind 2026-07-13 19:44:00] func_80035480: SCOPE VIOLATION —  M regfix.txt — session discarded.
[grind 2026-07-13 19:44:01] grinder stopped.
[grind 2026-07-13 19:44:10] grinder starting (pid 40232, model fable, judge fable)
[grind 2026-07-13 19:44:11] pre-flight: oracle green.
[grind 2026-07-13 19:44:12] func_80035480: session 2 starting, modality=structural
[grind 2026-07-13 19:57:45] func_80035480: MERGED — COMPLETED-C.
[grind 2026-07-13 19:57:46] cpu_side_move_dir_2: session 2 starting, modality=structural
[grind 2026-07-13 20:04:02] cpu_side_move_dir_2: MERGED — COMPLETED-C.
[grind 2026-07-13 20:04:03] func_8003C42C: session 2 starting, modality=structural
[grind 2026-07-13 20:18:19] func_8003C42C: MERGED — COMPLETED-C.
[grind 2026-07-13 20:18:22] func_8003D7B4: session 2 starting, modality=structural
[grind 2026-07-13 20:39:41] func_8003D7B4: SCOPE VIOLATION —  M engine/cheats.py |  M engine/queue.py |  M tools/check_completion_integrity.py |  M tools/hooks/no_new_regfix_guard.py | ?? .claude/rules/maspsx-gate-lists.md — session discarded.
[grind 2026-07-13 20:39:43] func_8003D7B4: session 2 starting, modality=structural
[grind 2026-07-13 20:43:08] func_8003D7B4: SCOPE VIOLATION — ?? .claude/rules/maspsx-gate-lists.md — session discarded.
[grind 2026-07-13 20:43:30] func_8003D7B4: session 2 starting, modality=structural
[grind 2026-07-13 20:47:00] func_8003D7B4: SCOPE VIOLATION — A  .claude/rules/maspsx-gate-lists.md | M  engine/cheats.py | M  engine/queue.py | M  tools/check_completion_integrity.py | M  tools/hooks/no_new_regfix_guard.py — session discarded.
```
