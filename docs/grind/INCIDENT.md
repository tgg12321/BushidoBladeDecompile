# GRINDER CIRCUIT-BREAK — 2026-07-21 16:14 — RESOLVED 2026-07-21

**Reason:** judge unreachable/invalid after 5 attempts for cpu_get_dist

**Root cause:** plan usage limit — every judge spawn died instantly with
`429 "You've hit your session limit · resets 4pm (America/Chicago)"`
(tmp/grind/judge_cpu_get_dist.json.agent.log). Environmental, not a pipeline
defect; the driver's backoff (60s→960s) correctly outlasted only ~32 min, all
inside the limit window. The s2 candidate was bytes-proven mid-flight (src +
regfix retire state left dirty by the break, per design: candidate waits).

**Recovery (operator, 2026-07-21):** fable probe OK post-4pm-reset; s2 closing
form banked in memory/grind/cpu_get_dist/candidate.c (committed); src/regfix
reverted to HEAD + verify-oracle green; relaunched. s3 inherits candidate.c and
re-runs the candidate path with the judge reachable.

git HEAD: 17377cb3
git status:
```
 M memory/grind/cpu_get_dist/candidate.c
 M metrics/events.jsonl
 M regfix.txt
 M src/code6cac_b.c
?? memory/grind/cpu_get_dist/rejected/staged-splitinit-all-variants-27-33.c
?? memory/grind/cpu_get_dist/rejected/store-stage-dead-var-reuse-26.c
?? memory/grind/cpu_get_dist/rejected/wrap-alone-without-dead-store-27.c

```
Last 20 log lines:
```
[grind 2026-07-21 13:40:38] func_8007B844: session 5 starting, modality=permuter
[grind 2026-07-21 13:40:38] func_8007B844: SCOPE VIOLATION —  M regfix.txt — session discarded.
[grind 2026-07-21 13:40:38] grinder stopped.
[grind 2026-07-21 13:40:39] grinder starting (pid 12556, model fable, judge fable)
[grind 2026-07-21 13:40:40] pre-flight: oracle green.
[grind 2026-07-21 13:40:40] func_8007B844: session 5 starting, modality=permuter
[grind 2026-07-21 14:13:27] func_8007B844: progress applied — floor=6, 's5 permuter: floor 6 holds; 3 fresh-seed campaigns on never-randomized chassis (172k iters, all stopped) confirm the permuter axis dead across 5 lifetime chassis geometries — no sub-135 find has ever occurred; frontier is exclusively F2 sched-forensics + F3 cross-project research'
[grind 2026-07-21 14:13:51] func_8007B844: session 6 starting, modality=forensics
[grind 2026-07-21 14:40:07] func_8007B844: judge ruling PASS recorded.
[grind 2026-07-21 14:40:30] func_8007B844: session 6 starting, modality=forensics
[grind 2026-07-21 14:54:07] func_8007B844: MERGED — COMPLETED-C.
[grind 2026-07-21 14:54:08] cpu_get_dist: seeded ledger from memory/wip checkpoint.
[grind 2026-07-21 14:54:08] cpu_get_dist: session 1 starting, modality=recon
[grind 2026-07-21 15:04:56] cpu_get_dist: progress applied — floor=15, 'Recon: floor 15 re-confirmed on main; duplicate lead killed (func_8003032C is a same-address stale twin); m2c = natural rx-first shape; fresh 15-form RTL dumps banked; rz-addend-lead swap measured KILLED at 25.'
[grind 2026-07-21 15:05:20] cpu_get_dist: session 2 starting, modality=structural
[grind 2026-07-21 15:42:58] judge attempt 1 returned no valid verdict; backing off 60s.
[grind 2026-07-21 15:44:01] judge attempt 2 returned no valid verdict; backing off 120s.
[grind 2026-07-21 15:46:04] judge attempt 3 returned no valid verdict; backing off 240s.
[grind 2026-07-21 15:50:08] judge attempt 4 returned no valid verdict; backing off 480s.
[grind 2026-07-21 15:58:12] judge attempt 5 returned no valid verdict; backing off 960s.
```
