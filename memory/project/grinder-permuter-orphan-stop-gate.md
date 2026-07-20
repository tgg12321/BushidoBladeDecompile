---
name: grinder-permuter-orphan-stop-gate
description: "permuter_campaign.py launch DETACHES the campaign, so a one-shot grind session that ends its turn 'waiting' for it orphans the campaign + writes no outcome JSON → driver discards the whole session as invalid (two s4 sessions on gnd_init_80041688, 2026-07-20). The role forbids it in prose but models paraphrase around it. Fixed: tools/hooks/grind_check.sh Stop-gate BLOCKs a stop while a campaign for GRIND_FUNC is still registry-active."
metadata:
  type: project
---

# Permuter campaigns orphaned by turn-end; grind_check.sh Stop-gate (fixed 2026-07-20)

## Mechanism
`tools/permuter_campaign.py launch` runs the campaign with
`Popen(..., start_new_session=True)` and returns immediately. A grind session
is a one-shot `claude -p`: ending the turn ends the process. If the agent
launches a campaign and then ends its turn "waiting" for it (e.g. "waiting for
monitor events"), the detached campaign is orphaned and — because no outcome
JSON is on disk at that instant — the driver discards the ENTIRE session as
invalid (`no outcome file / unparseable JSON`), reverts src, and respawns. Three
in a row → circuit-break.

`tools/grinder/roles/grind-session.md` forbids this in prose (with a cited
2026-07-18 incident), but capable models paraphrase around prose warnings. On
2026-07-20 two consecutive s4 permuter sessions on `gnd_init_80041688` were lost
this way (opus-4-7, "Baseline confirmed... permuter campaign running. Waiting for
monitor events."). The semantic Stop-gate the agent registry's resilience-judge
was supposed to sit behind — `tools/hooks/grind_check.sh` — did not exist.

## The gate (deterministic, not keyword-based)
`tools/hooks/grind_check.sh` (wired into `.claude/settings.json` **Stop** only,
not SubagentStop):
- **Arms only when `GRIND_FUNC` is set.** The grinder's spawn sets it
  (`grind.ps1` Invoke-GrindAgent job); interactive/operator sessions and the
  grind session's own subagents are unaffected.
- **BLOCKs the stop** if `tmp/permuter_campaigns.json` still has an entry with
  `func == GRIND_FUNC` and `active == true`, quoting the exact
  `harvest --dir <d> --stop` command to run first.
- **Fails open** on any error / missing file / no active entry.
- Operator override: `touch .bb2_grind_guard_off`.

**Load-bearing invariant:** `harvest --stop` clears `active`; a session that
properly stops its campaigns passes. To keep "active at Stop ⇒ orphaned by THIS
session" exact, `Reap-PermuterOrphans` (every session boundary) now calls
`permuter_campaign.py deactivate-all` — reaping killed orphan *processes* but
left their registry `active` flags set, which would false-positive the next
session of the same func.

## Why not just strengthen the role text
It was already strong and cited a prior identical incident; prose is
paraphrase-able. The fix mirrors [[grinder-nonmatchings-scope-break]] and the
tooling-incident Stop guard: make the rule mechanical.

## Related
- [[grinder-pipeline]] — the Grinder overall
- [[grinder-nonmatchings-scope-break]] — sibling permuter-lane circuit-break
- [[permuter-fresh-seed-discipline]] — the campaign discipline the gate enforces
