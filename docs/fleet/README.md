# docs/fleet/ — autonomous fleet records (owner-facing)

These files are written by the Autonomous Decomp Fleet (`tools/fleet/`) while it runs
unattended, and committed periodically to `main`. They are your audit trail — read
them when you return. (They live under `docs/` rather than `memory/` so they don't
need memory-system frontmatter and never trip the memory indexer / pre-commit hook.)

| File | What it holds | What you do with it |
|---|---|---|
| `log.jsonl` | full structured event stream (completions, rejects, reaudits, circuit events) | grep/scan for anything notable |
| `pending_owner.md` | **canonical-asm + novel-technique items that did NOT auto-merge** | review each; authorize (add to `inline_asm_canonical.txt` / sanction the family) or reject |
| `regressions.md` | committed functions the re-audit patrol flagged as carrying a cheat | each is being re-done cleanly; verify the clean replacement landed |
| `adjudications.md` | every Adjudicator ruling (the deep-expert decisions) | spot-check the reasoning |
| `incidents.md` | every Overseer incident + recovery decision | confirm recoveries were sound |

Nothing here is a build input — it's all documentation. The fleet's actual work
product is the `Match:` commits on `main` (each = one completed function) and the
queue/state in `tmp/fleet/` (gitignored operational state).

Design: `docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md`
Operator guide: `tools/fleet/README.md`
