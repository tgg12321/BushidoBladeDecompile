# Archived docs

Historical top-level docs from earlier project eras, preserved for their
durable content but no longer authoritative. Each carries an `ARCHIVED
YYYY-MM-DD` HEAD note pointing at what replaced it.

Not indexed by `docs/README.md` — this is a graveyard, not a nav
surface. If you find yourself linking here from a live doc, ask whether
the durable content should be extracted into a rule under
`.claude/rules/` or into the auto-memory system instead.

## Contents (2026-07-13)

| File | Era | Superseded by |
|---|---|---|
| [`board.md`](board.md) | GitHub Projects board / fleet | `engine/queue.json` + Grinder |
| [`asmfix_attempt_notes.md`](asmfix_attempt_notes.md) | `dc.sh next-asmfix` workflow | engine + per-function ledgers/WIP |
| [`DECOMP_QUALITY_AUDIT_2026-05-21.md`](DECOMP_QUALITY_AUDIT_2026-05-21.md) | dated one-shot quality audit | reproducible via `tools/decomp_quality_audit.py` |
| [`targeted_permuter_plan.md`](targeted_permuter_plan.md) | `bb2_permuter.py`/`bb2_retire.py` design + results | those tools removed; permuter now driven manually or via Grinder |
| [`CPU_SIDE_MOVE_DIR_4_PROGRESS.md`](CPU_SIDE_MOVE_DIR_4_PROGRESS.md) | 2026-05-30 progress snapshot | Grinder ledger `memory/grind/cpu_side_move_dir_4/` |
| [`SYSTEM_C_SIBLINGS_HANDOFF.md`](SYSTEM_C_SIBLINGS_HANDOFF.md) | 2026-05-30 sibling cluster handoff | Grinder ledger + `.claude/rules/register-alloc-pure-c.md` |

## Related

- `docs/HISTORY.md` — project-wide timeline / milestones (still live, not archived)
- `docs/superpowers/specs/` — retired-era design specs (kept alongside current specs; see each file's HEAD note)
- `docs/fleet/HANDOFF.md` — the retired fleet handoff (still in place, marked SUPERSEDED)
