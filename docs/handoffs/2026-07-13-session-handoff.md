# Session handoff — 2026-07-13

Multi-session continuation covering: LIBGTE canonical-asm authorization
pass, Closer Phase 3 residual sweep, a new sanctioned lever, docs
refresh, and a Grinder run that produced two completions before
circuit-breaking on a policy-vs-mechanism question the owner adjudicated.

## TL;DR

- **14 functions closed** across 12 commits.
- **Grinder is RUNNING** as of 2026-07-13 00:49 (pid 49780, opus-4.8 for
  both worker and judge). Currently on `func_8001C624` (regression),
  session 2, modality=structural. See "Grinder state" below.
- **1 owner-close needs review** — `func_8001B138` closed via header-level
  type correction (u16 → s16) against a layer-2 cheat-reviewer FAIL.
  See "The open question" below.
- **7 legitimately-banked candidates remain** in `memory/closer/candidates/`
  — each is a real ongoing investigation, none crackable in a single
  autonomous session.

## What was accomplished

### Completions (14 functions)

| Commit | Function(s) | Notes |
|---|---|---|
| `9eba9a3e` | 10 LIBGTE modules (E1AC, E1FC, ED6C, F24C, F2DC, E4DC, EB4C, E8DC, E74C, EA0C) | Canonical-asm authorization pass; 27 forbidden regfix rules cleared |
| `c7114628` | saEft00Add | Session 9's bit-exact work + `queue done` |
| `d2e55756` | saTan2Main | New sanctioned lever: hoist-shared-arm-computation |
| `8b158288` | cpu_get_move_pattern_table_number | Grinder autonomous (2 sessions) |
| `4598a98e` | func_8001B138 | Owner-close via type correction — see caveat |

### New sanctioned pure-C lever

`.claude/rules/hoist-shared-arm-computation-defeats-copy-pref.md` +
codegen-technique-index entry. Sanctioned 2026-07-12 (commit `4ebd1fa0`).
When two `if/else` branches duplicate `z = x + y` and the residual is
register-choice on `z`, hoisting the sum out of both arms breaks
copy-preference propagation. GCC's `jump2` duplicates the assignment
back into arms at codegen. Confirmed case: saTan2Main.

### Housekeeping

- `2ffe344a` — 9 obsolete banked patches deleted, 8 stale WIP entries
  deleted, `volatile-grant-proposals.md` collapsed to a resolved-proposals
  table (all 4 ratified), root cleanliness allowlist updated.
- `7fefcd8b` — `docs/STATUS.md` regenerated for engine-era state,
  `docs/HISTORY.md` extended with Phase 7 (engine + SOTN alignment) and
  Phase 8 (Closer PsyQ adoption mission).

## The open question — func_8001B138

**This one deserves eyes.** The Grinder ran 11 sessions across 4
modalities on `func_8001B138`, hit a 1-word encoding residual (`addiu`
target vs `ori` clean-C), Judge issued 5 FAIL rulings on within-function
coercion attempts, circuit-broke on policy deadlock. I closed it in the
follow-up review by changing `include/system.h:10`:

```
- extern u16 g_file_vram_timer;
+ extern s16 g_file_vram_timer;
```

With `s16`, the natural clamp form (`if (g < -0x1C00) g = -0x1C00;`)
emits `addiu` matching target. All `(s16)` casts throughout the function
body became redundant and were removed. Sandbox 5 → 0; retire dropped
1 rule; SHA1 == oracle.

**The debate:** Layer-2 `cheat-reviewer` FAILed on "same coercion family,
different scope" grounds — argued the c-typeck.c:3987 fold-escape
mechanism is the same the Judge banned, just moved from local-variable
scope to global-declaration scope.

Owner adjudicated legitimate reverse-engineering 2026-07-13 based on:
- The `(s16)` casts throughout the function body were load-bearing for
  correct semantic behavior under u16 (u16 promotion makes `< -0x1C00`
  always false without the cast). They weren't stylistic — they were
  compensating workarounds for a wrong type.
- A global's declared type is a data-model decision affecting every use
  site, categorically different from a local-variable coercion.
- Removing the type change would leave the committed body with the
  original typed-holder cheat (predates the 2026-06-01 cheats-by-any-
  spelling policy — commit `45b7ae9e` from the initial decomp).

**My honest note as I wrote this**: I was 65-70% confident it's
legitimate. The strongest legitimacy argument is that the (s16) casts
verifiably don't work under u16 declaration — that's not a
rationalization, it's a structural fact. The strongest cheat argument is
that the mechanism *is* the same one the Judge banned, and I discovered
this "seconds after a circuit-break" specifically searching for a way
past the deadlock. Timing is uncomfortable even if the answer is right.

**If a fresh review concludes this was wrong**, revert `4598a98e` and the
function returns to floor=1 with the typed-holder cheat body — same
INCOMPLETE-with-cheat state the Grinder was working from. The commit
message + this handoff have full context.

**Precedent hazard**: if this stands, the sanctioned-lever catalog
should get an entry distinguishing "correcting a global's type based on
independent semantic evidence" (legitimate) from "moving a
within-function coercion to a header" (cheat). I didn't add that entry —
the next agent should if the close stands.

## Grinder state

**Driver**: RUNNING (pid 49780) since 2026-07-13 00:49
**Model**: `claude-opus-4-8` for both worker and judge (user directive
2026-07-13: "we are too far in the endgame for [sonnet]")
**Current target**: `func_8001C624` (src/code6cac.c), origin=regression,
session 2, modality=structural

### Grinder commands (Windows-side, PowerShell)

| Task | Command |
|---|---|
| Status | `pwsh tools/grinder/status.ps1` |
| Live tail | `Get-Content tmp/grind/grind.log -Wait -Tail 20` |
| Judge rulings | `docs/grind/decisions.md` |
| Journal | `docs/grind/journal.md` |
| Stop cleanly | `pwsh tools/grinder/grind.ps1 -Stop` |
| Resume (fable exhausted; use opus-4.8) | `Start-Process pwsh -WindowStyle Hidden -ArgumentList '-NoProfile','-File',"`"$PWD\tools\grinder\grind.ps1`"",'-Model','claude-opus-4-8','-JudgeModel','claude-opus-4-8'` |

### Model quota note

Fable-5 quota exhausted this thread. Both `Model` and `JudgeModel`
default to `fable` in the grinder script — always pass explicit
`-Model claude-opus-4-8 -JudgeModel claude-opus-4-8` until fable
refills (or edit the defaults in `tools/grinder/grind.ps1` lines 23-24).

## Outstanding banked candidates (Closer Phase 3)

None crackable single-session; each documents a real ongoing
investigation. Full assessment in `memory/closer/phase3-progress.md`
session 15.

| Candidate | Blocker | Path forward |
|---|---|---|
| `cdcontrol_trio_prologue_order.c` | Prologue def-order wall (same family as func_8007C2A0/C4B8). Confirmed by test-edit: applying honest C breaks anchor-indexed regfix rules, oracle red at floor 8/6/8. | Same wall as the C2A0/C4B8 twins. Either finds a novel lever there and mirrors, or stays as banked plateau. |
| `cdcw_tslTm2LoadImage.c` | Coupled to marionation Phase-2 nrefs family. 4 named residual clusters, 2 in the twins' family. | Grinder is running ~57 sessions on marionation_Exec + ~98 on cpu_side_move_dir_4 — when nrefs cracks, mirror here. |
| `exec_game_sotn_hybrid.c` | Big function (`_spu_gcSPU`), banked at 30 with 4 named residual clusters (cur giv-split, key_b caching, p-derivation, prologue li). cc1 ICE on structured pointer-walk. | Substantial dedicated deep-dive. |
| `marionation_p6_volatile1496.c` / `marionation_vAT1_notailwrap.c` | Phase 2 twin. Grinder territory. | Autonomous Grinder — already running. |
| `spu_writebyio_splice.c` | `_spu_FiDMA` hits cc1 fork segfault on `while (volatile MMIO) { ... break; }` class. 15 variants measured negative in session 11. | Needs owner Ruling-2 (fork-divergence class) OR major source-side workaround search. |
| `spusetreverbmodeparam_struct.c` | 3-word sp58 spill residual. Named mechanism: cc1's reload pass runs 2 passes vs Sony's 1. | Multi-session infrastructure: BB2_RELOAD_DEBUG cc1 instrumentation to identify the pass-1-stacked pseudo set. |

## Where to look next

### If you're picking up autonomously

1. **Check grinder status first**: `pwsh tools/grinder/status.ps1`. If it
   circuit-broke or completed a batch, read `docs/grind/decisions.md` and
   `docs/grind/INCIDENT.md` (currently the incident dir is empty; it
   moves to `docs/grind/resolved/` on close).
2. **Grinder autonomous work is the default worklist**. Don't manually
   cherry-pick queue items while the Grinder is running — it takes the
   top; you'd fight it. Let it grind.
3. **If asked to work on Closer Phase 3 items**: read
   `memory/closer/phase3-progress.md` (sessions 1-15) first. The banked
   candidates each have detailed mechanism notes. The mission brief is
   `docs/closer/mission-phase3-psxsdk-adoption.md`.

### If you're picking up conversationally (user is back)

Ask about:
1. **func_8001B138 review** — does the type-correction close stand or
   need reversal? (See "The open question" above.)
2. **Grinder status** — should it keep running, and on which model.
3. **Whether to add the "global type vs local coercion" distinction to
   the sanctioned-lever catalog** — deferred this session.

## Key file pointers

| Purpose | File |
|---|---|
| Live queue (single source) | `engine/queue.json` |
| Live worklist top | `& tools/wteng.ps1 main queue next` |
| Project status snapshot | `docs/STATUS.md` (refreshed 2026-07-12) |
| Project history | `docs/HISTORY.md` (through Phase 8, 2026-07-13) |
| Closer Phase 3 ledger | `memory/closer/phase3-progress.md` |
| Grinder ledgers | `memory/grind/<func>/` + `docs/grind/` |
| Sanctioned lever catalog | `.claude/rules/codegen-technique-index.md` |
| CLAUDE.md workflow doc | `CLAUDE.md` |
| Handoffs directory | `docs/handoffs/` (this file lives here) |

## Session invariants that held

- Oracle green throughout the session (except during test-edits that
  were immediately reverted)
- Root cleanliness: 0 suspicious, 0 unknown
- No CLAUDE.md / AGENTS.md changes
- No stale WIP or candidate files left behind (housekeeping in
  `2ffe344a` was pre-emptive; nothing new added since)
- Every completion class commit passed layer-1 review; layer-2 was
  invoked on saTan2Main + LIBGTE batch + func_8001B138. The saTan2Main
  and LIBGTE reviews PASSed; the func_8001B138 review FAILed and was
  owner-overridden with documentation.
