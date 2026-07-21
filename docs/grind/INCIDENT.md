# GRINDER CIRCUIT-BREAK — 2026-07-21 01:48 — RESOLVED 2026-07-21

**Reason:** 3 consecutive invalid sessions on saSeMain_80045600 (all
`SCOPE VIOLATION — ?? "]\357\200\242"`)

**Root cause:** a zero-byte root file named `]` + U+F022 (mangled redirect
artifact, created once at 01:08 during the first s2 attempt; no generating
script survives — one-off). The scope-violation cleanup only did
`git checkout -- .` (tracked files) + `git clean -fd -- tmp` (skips gitignored
tmp), so a root-level UNTRACKED junk file survived the discard and re-tripped
the scope check of every respawn: sessions 2 and 3 were discarded for dirt they
did not create. The third discarded session had CLOSED saSeMain_80045600 to
sandbox 0 (candidate-ready) — its outcome survived on disk and was banked to
the ledger by the operator (s2 apply, 2026-07-21); the closing form re-derives
from the ledger + tmp/grind/saSeMain_80045600/s2/p1_guard_ge_induction.c.

**Fix (committed 2026-07-21):** the scope-violation path now also runs
`git clean -fdq -e memory -e docs -e src -e include`, purging any untracked
out-of-surface dirt it just flagged so junk can never poison respawns. Sibling
of the nonmatchings/ scope-break (2026-07-19) — same sticky-untracked-dirt
class, now closed for ALL junk paths, not just known ones.

git HEAD: aeb77dd6
git status:
```
M  metrics/events.jsonl
?? "]\357\200\242"
?? memory/grind/saSeMain_80045600/rejected/dead-index-use-dce-before-reload-vars0.c
?? memory/grind/saSeMain_80045600/rejected/rotated-top-test-loop-fires-phantom-but-score-11.c
?? memory/grind/saSeMain_80045600/rejected/s16-derivation-respellings-13-forms-all-memfold-vars0.c

```
Last 20 log lines:
```
[grind 2026-07-20 21:11:24] func_80033550: session 4 starting, modality=permuter
[grind 2026-07-20 22:37:24] func_80033550: progress applied — floor=4, 's4 permuter: floor holds at 4; four campaigns (~104k iters, 4 basins incl. the v07-flip neighborhood) all converge to the score-20 ptr=a1 attractor with zero sub-20 finds — permuter whole-function axis measured near-dead, corroborating the s2/s3 structural closure'
[grind 2026-07-20 22:37:47] func_80033550: session 5 starting, modality=permuter
[grind 2026-07-20 23:19:33] func_80033550: progress applied — floor=4, 's5 permuter: floor holds at 4; two fresh basins (DImode-pair 17k iters, walker-pointer 17k iters) both converge to the score-20 ptr=a1 attractor — permuter modality now fully dead (6 basins, ~138k cumulative); NEW fact: uninit-read pseudo occupies $a3 byte-free (first zero-byte occupant ever observed)'
[grind 2026-07-20 23:19:56] func_80033550: session 6 starting, modality=forensics
[grind 2026-07-20 23:39:49] func_80033550: progress applied — floor=4, 's6 forensics: floor holds at 4; cc1psx is instruction-identical (fork-divergence KILLED), and source-level closure theorem proven — no valid C with this 34-insn shape can home ptr in a3 (all byte-free a1/a2-occupancy channels closed: DImode even-pair rule mips.c:3447, empty/unreachable preferences, coalesce=same-reg-only final.c:1800, dead-defs die pre-RA flow.c:1479, uninit-use invalid)'
[grind 2026-07-20 23:40:13] func_80033550: session 7 starting, modality=forensics
[grind 2026-07-21 00:19:44] func_80033550: judge ruling FAIL recorded.
[grind 2026-07-21 00:20:07] func_80033550: session 7 starting, modality=forensics
[grind 2026-07-21 00:27:30] func_80033550: OWNER-GATED — parked pending owner ruling (docs/grind/decisions.md — '2026-07-21 — func_80033550 (src/code6cac_b.c) — OWNER-ESCALATION' (filed by grind s8 forensics per the 2026-07-21 00:19 Judge FAIL disposition and the standing 2026-07-20 endgame-lock-disposition rule)).
[grind 2026-07-21 00:27:32] func_8007C2A0: session 1 starting, modality=recon
[grind 2026-07-21 00:38:52] func_8007C2A0: MERGED — COMPLETED-C.
[grind 2026-07-21 00:38:53] saSeMain_80045600: session 1 starting, modality=recon
[grind 2026-07-21 01:01:07] saSeMain_80045600: progress applied — floor=4, 'Residual is purely the +8-byte phantom frame slot; phantom CONFIRMED reachable here (positive control vars=8) and two zero-cost in-tree witnesses found; 6 spellings measured (5 killed) via a new vars= probe harness; src cheats (volatile pad + v0 pin) removed at no cost'
[grind 2026-07-21 01:01:31] saSeMain_80045600: session 2 starting, modality=structural
[grind 2026-07-21 01:17:14] saSeMain_80045600: SCOPE VIOLATION — ?? "]\357\200\242" — session discarded.
[grind 2026-07-21 01:17:37] saSeMain_80045600: session 2 starting, modality=structural
[grind 2026-07-21 01:38:23] saSeMain_80045600: SCOPE VIOLATION — ?? "]\357\200\242" — session discarded.
[grind 2026-07-21 01:38:46] saSeMain_80045600: session 2 starting, modality=structural
[grind 2026-07-21 01:48:41] saSeMain_80045600: SCOPE VIOLATION — ?? "]\357\200\242" — session discarded.
```
