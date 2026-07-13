# Session handoff — 2026-07-13 (session 2)

Continuation of the 2026-07-13 session handoff (`62e86923`). This session
focused on non-Grinder wrap-up and infrastructure while the Grinder ran
autonomously overnight. 64 commits landed: 15 Grinder Match completions +
14 docs/rules refreshes + audit reports + policy ruling paperwork.

## TL;DR

- **Grinder landed 15 COMPLETED-C matches overnight**, unattended, on
  `claude-opus-4-8` (both worker and judge).
- **Ruling-2 (fork-divergence-inline-asm) sanctioned** as a new narrow
  authorized-inline-asm sub-class. Paperwork + evidence pack + rule doc
  landed. Src-level close for `_spu_FiDMA` attempted but reverted (outer
  scheduling residual outside ruling scope; deferred as critical decision).
- **Header-type-correction rule sanctioned** as a codegen technique after
  independent adversarial review of the `func_8001B138` close (`4598a98e`).
  One MEDIUM candidate flagged for owner review: `D_8010277C u8→s8`.
- **Doc corpus refreshed** for the engine + Grinder workflow. CLAUDE.md
  259→165, CONTRIBUTING.md 281→200, retired-workflow refs (dc.sh /
  WORK_QUEUE.md / bb2_*) swept from 15 docs + 7 tools + 6 rules. Six docs
  archived to new `docs/history/` with head notes.
- **Naming DB audited**: 976 rows resolved, 23 low-conf still pending.
- **Grinder is RUNNING** as of handoff time on `func_80037540` in
  `src/code6cac_b2_post.c` (session 2, judge FAIL then retry ongoing).

## The Ruling-2 open question

`_spu_FiDMA` in `src/main.c` (DispUpdateStatusMessage splice) has been:
- Granted Ruling-2 sanction (rule at `.claude/rules/fork-divergence-inline-asm.md`)
- Attempted for src close: wait-loop asm island closed byte-exact against
  target's delay-slot+comp region, but 4 residual insns in outer structure
  (bios_DeliverEvent jal delay slot + speculative constant preload)
- Reverted to paperwork-only state (oracle green); function stays `active`

**Options for owner return:**
1. **Widen Ruling-2 scope** to allow region-scoped asm for scheduling-class
   divergences (needs its own evidence gate — currently the four-gate bar
   requires a cc1 crash, which the outer scheduling doesn't have)
2. **Grinder search work** — the outer 4-insn residual is ordinary
   scheduling; may or may not yield to a novel C form. The wait-loop asm
   island lever is validated; when Grinder reaches DispUpdateStatusMessage
   it inherits this state.

## The header-type-correction candidate

`docs/naming/header-type-audit-2026-07-13.md` details:

- **MEDIUM: `D_8010277C` (code6cac.h:401) u8 → s8** — 11 compensating
  `(s8)` scalar casts across code6cac.c / code6cac_b.c / code6cac_c_ab.c
  as signed-array-index sites. Prong (b) hinges on build verification
  (would need to confirm target uses `lb` not `lbu`); the sweep couldn't
  run build against the Grinder's live state. Reviewer note: casts
  predate any recent session (blame verified).
- Plus 3 LOW candidates (weaker evidence).

The rule (`.claude/rules/header-type-correction-from-use-sites.md`)
requires layer-2 cheat-reviewer verification against the tree, not the
audit summary. If you decide to attempt the D_8010277C flip, follow the
rule's four-prong protocol.

## Grinder overnight summary

15 Match commits between 03:13 (overnight restart) and this handoff:
`func_8001FAE4, func_8001FB34, func_800233AC, se_data_set, DispSamnailWindow,
func_8003047C, func_80030BA8, func_80032064, func_80033498, func_800342A0,
func_800343F0, mottest_disp`, plus prior day's `cpu_get_move_pattern_table_number,
camera_SetMatrix_8001DBE4, func_8001B138 (owner-close)`.

Notable Judge behavior:
- Rejected a proposed u16 retype on `func_80033498` (2026-07-13 09:26) using
  the new `header-type-correction-from-use-sites` rule — the four prongs
  are being enforced correctly.
- Circuit-breaker on `func_80037540` current session — 2 FAILs on structural
  modality; retrying.

Grinder state at handoff: `pid 9408`, session `2` on `func_80037540` in
`src/code6cac_b2_post.c`, modality=structural.

## What landed this session

### Rules & policy (new)
- `.claude/rules/header-type-correction-from-use-sites.md` (`f1de3289`) — the
  four-prong rule for header signedness corrections; codegen-index entry
- `.claude/rules/fork-divergence-inline-asm.md` (`c5f8fde3`) — Ruling-2
  sanction with four-gate evidence bar; codegen-index entry
- `.claude/rules/decomp-loop.md` (`421124a1`) — manual-path per-function loop
  extracted from CLAUDE.md

### Ruling paperwork & evidence
- `docs/escalations/closer-cc1-fork-divergence.md` — updated with the
  Ruling-2 grant + attempted-close note
- `docs/escalations/spu3-fork-crash-evidence/` — 8-probe grid, cc1psx
  counter-exhibit, results table

### Doc corpus refresh
- CLAUDE.md 259→165 (`421124a1`), CONTRIBUTING.md 281→200 (`0465af8b`)
- BUILD.md dc.sh → engine CLI (`15129315`)
- README.md status counts + retired refs (`9816e5d5`)
- `docs/naming/README.md` refreshed for post-audit state (`0e2d278a`)
- 8 more docs swept for retired refs (`dc521bbb`)
- 6 rules swept for retired dc.sh command examples (`33a20114`)
- 7 tools' docstrings + allowlist cleaned (`9e796d6c`)
- 3 fleet-era superpowers specs RETIRED head notes (`407b1424`)
- `IMMUTABLE_PLATEAUS.md` refreshed to engine syntax + historical note (`abbe22e5`)
- Format docs cross-checked against inspectors; STAGE_BIN.md field-table
  bug fixed (`b8e83a76`)

### Doc archival
- 4 dc.sh-era docs → `docs/history/` (`c7b9ce8a`, `f07350ec`)
- 2 dated snapshots → `docs/history/` (`abbe22e5`)
- New `docs/history/README.md` index (`aed821fe`)

### Audits / candidate reports
- `docs/naming/proposals_audit_2026-07-13.md` — DB audit (7c4347d4)
- `docs/naming/header-type-audit-2026-07-13.md` — type candidates for owner (`d67ffa00`)
- `docs/naming/proposals_resolved.csv` — 976 resolved rows catalog
- `memory/project/` freshness audit — 10/11 notes still current (`16b64d79`)

### Grinder autonomous
- 15 Match: commits + close ledger + judge ruling commits
- SCOPE VIOLATION safety mechanism validated (grinder discards + respawns
  cleanly when parallel agents edit off-target files)

## Where to look next

If you're picking up conversationally:

1. **`D_8010277C` u8→s8 header flip** — worth a call. Report at
   `docs/naming/header-type-audit-2026-07-13.md`. Layer-2 cheat-reviewer
   verification protocol is in the rule.

2. **Ruling-2 scope decision for `_spu_FiDMA` outer scheduling** — 4
   insns short via ordinary scheduling divergence (not a crash class).
   Widen the ruling or let Grinder grind.

3. **Grinder status** — `pwsh tools/grinder/status.ps1`. If it's still
   circuit-breaking on `func_80037540` after a few more sessions, the
   modality ladder should surface a natural change. No intervention needed
   short of a stop.

If you're picking up autonomously:

- The Grinder is the default worklist. Don't cherry-pick.
- Both open questions above are owner decisions; defer.

## Key file pointers (unchanged from prior handoff except where noted)

| Purpose | File |
|---|---|
| Live queue | `& tools/wteng.ps1 main queue next` (also `queue status`) |
| Project status | `docs/STATUS.md` |
| Grinder ledgers | `memory/grind/<func>/` |
| Judge decisions | `docs/grind/decisions.md` |
| Grinder journal | `docs/grind/journal.md` |
| Escalations | `docs/escalations/` |
| Sanctioned lever catalog | `.claude/rules/codegen-technique-index.md` |
| Handoffs directory | `docs/handoffs/` |
| Archived docs | `docs/history/` (NEW this session) |

## Session invariants that held

- Oracle green throughout (except during test edits, immediately reverted)
- Grinder autonomous progress uninterrupted (11 Match commits during doc
  cleanup phase; SCOPE VIOLATION safety triggered once and cleanly recovered)
- No CLAUDE.md/AGENTS.md drift (both fresh, CLAUDE.md now under 200-line hygiene target)
- No src/*.c modifications by non-Grinder agents at commit time
- Every completion-class Grinder commit passed the default-FAIL Judge
