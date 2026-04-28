# Archived tmp/ scripts (2026-04-27)

The 109 ad-hoc `add_regfix*.py` scripts were the per-function manual workflow
for appending regfix rules. They are superseded by `tools/add_regfix.py`
(invokable as `dc.sh add-regfix <func> <op> <args>`), which validates indices
against live pipeline output and rolls back failed appends.

The `_tmp_*` scripts were exploratory analyses (callgraph, near-miss filter,
affinity audit, find_pairs, etc.). The most useful ones should be promoted
to first-class `tools/` files; the rest are reference only.

## Files

- `add_regfix*.py` (variants) -- replaced by `tools/add_regfix.py`
- `add_*regfix*.py` (variants) -- same
- `_tmp_*.py` -- exploratory analyses
- `agent99_*` -- agent session logs (snapshot, not a tool)

## Promotion candidates

Any of these would be useful as proper tools after refactoring:

- `_tmp_callgraph.py` -- function call graph; promote as `tools/callgraph.py`
- `_tmp_affinity_audit.py` -- function affinity scoring
- `_tmp_near_miss_clean.py` -- near-miss filter for permuter results
- `_tmp_find_pairs.py` -- sibling/related function pairing
- `_tmp_find_unmatched.py` -- list remaining backlog (now: `psyq_stdlib_scan.py` + grep)
