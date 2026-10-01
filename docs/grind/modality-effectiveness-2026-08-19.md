# Grinder modality effectiveness — conclusions (2026-08-19)

Data pass over `docs/grind/journal.md` (2026-07-06 → 2026-08-19, 661 logged sessions), 249 Judge
decisions, Match: commits and permuter telemetry. Trimmed 2026-10-01 to the conclusions; the full
analysis (confound tables, plateau breaking, FAIL pattern, cost side) resolves at git tag
`pre-slim-2026-10-01` (same path). Caveats: modality is collinear with session number (the ladder
is a fixed rotation), and the closing session emits no `floor=` line, so floor-drop tables are
survivor-biased; the tables below are the deconfounded cuts.

## Deconfounded: the second ladder cycle

| modality | cycle 1 (s2–s10) n / drops / rate | cycle 2+ (s11+) n / drops / rate |
|---|---|---|
| structural | 142 / 43 / **30.3%** | 77 / 0 / 0.0% |
| permuter | 89 / 11 / 12.4% | 66 / 0 / 0.0% |
| forensics | 77 / 5 / 6.5% | 61 / 1 / 1.6% |
| rederive | 48 / 3 / 6.2% | 55 / 0 / 0.0% |
| synthesis | 20 / 4 / **20.0%** | 26 / 0 / 0.0% |
| **all** | 376 / 66 / 17.6% | **285 / 1 / 0.4%** |

## Who wins (floor drops + closes)

| modality | logged sessions | floor drops | closes | total sessions | wins | win rate |
|---|---|---|---|---|---|---|
| recon | 0 | 0 | 40 | 40 | 40 | 100% |
| structural | 219 | 43 | 90 | 309 | 133 | **43.0%** |
| permuter | 155 | 11 | 13 | 168 | 24 | 14.3% |
| synthesis | 46 | 4 | 1 | 47 | 5 | 10.6% |
| forensics | 138 | 6 | 4 | 142 | 10 | 7.0% |
| rederive | 103 | 3 | 3 | 106 | 6 | 5.7% |

## Recommendations (adopted into the ladder retune, `.claude/rules/asm-until-matched.md`)

- **R1** (very high confidence) — stop the ladder cycling: cycle 2+ (s11 onward) produced 1 drop in
  285 transitions; after one flat cycle go to a distinct deep-dive mode, not the same nine rungs.
- **R2** (medium) — move synthesis from s10 to ~s6, before forensics and rederive.
- **R3** (high for 3rd+ permuter session, 0/64) — cap permuter at one session per function.
- **R4** (medium-high) — on a second Judge FAIL naming the same banned family, route away from
  resubmission.
- **R5** (high) — have the driver journal the closing session (`floor=0` line) so closers are measurable.
