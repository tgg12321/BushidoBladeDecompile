# Tooling-error forced-fix system

Hardens the `debugging-discipline` rule (*every tooling error happens once*)
from a soft memory note into a **hook-enforced gate**. When an agent hits a
known, permanently-fixable tooling/shell/environment failure — CRLF line
endings, WSL unavailable, a broken worktree symlink / missing linked dep, a
core tool not on PATH — the harness **forces a pause** and requires a permanent
root-cause fix, instead of letting the agent paper over it and move on.

This is the same enforcement philosophy as `grind_check.sh` (you can't quit an
unmatched function) and `main_branch_guard.sh` (you can't edit on main), applied
to tooling errors.

## Why

Agents waste real time fighting the same shell/tooling problems repeatedly:
CRLF-mangled scripts, WSL not started, worktree symlinks that didn't get
relinked. The project already had the *rule* ("fix permanently, every error
happens once") and even self-explaining error messages (`dc.sh verify-toolchain`
emits `fix:` hints) — but nothing **enforced** the rule, and there was no
*preventive* net for the #1 recurring class (CRLF). This system adds both.

## Components

| File | Role |
|---|---|
| `tools/hooks/tooling_error_guard.py` | **Detector** (PostToolUse on `Bash\|PowerShell\|Edit\|Write\|MultiEdit`). Scans command output against the signature library; for build-file edits, checks the written file for CRLF. Block-tier match → writes incident marker + exits 2 with a directive. |
| `tools/hooks/tooling_error_signatures.json` | **Signature library**. Extensible catalog of failure classes. *Adding a signature is itself a valid permanent fix.* |
| `tools/hooks/tooling_incident_stop_guard.sh` | **Forced-pause gate** (Stop / SubagentStop). Refuses to end the turn while an unresolved incident marker exists. Mirrors `grind_check.sh`'s fail-open safety model. |
| `tools/resolve_tooling_incident.py` | **Resolver** (`dc.sh fix-tooling-incident`). The only clean way to clear the marker. Verifies a real permanent guard was made; logs to the ledger. |
| `.gitattributes` | Git-level net: `eol=lf` on build-critical files. Deliberately **scoped** (C/headers, Makefile, `*.ld`, the LF-critical pipeline `.txt` files) to avoid renormalizing existing CRLF / vendored files — the live net is the detector's post-write check. |
| `docs/tooling_incidents.md` | **Ledger**. Append-only catalog of resolved/false-positive/deferred incidents — institutional memory. |
| `tools/hooks/test_tooling_error_guard.py` | Unit tests (classification + false-positive guard). |

## The loop

```
 Bash/Edit produces a known failure signature
        │
        ▼
 tooling_error_guard.py (PostToolUse)
   • writes .bb2_tooling_incident.json
   • exits 2 → directive shown to the agent: STOP, fix permanently
        │
        ▼
 agent tries to end the turn
        │
        ▼
 tooling_incident_stop_guard.sh (Stop)  ── marker present? ──► BLOCK ("decision":"block")
        │ (no marker)                                              │
        ▼                                                          ▼
      turn ends                                  agent implements a PERMANENT fix
                                                 (new hook / .gitattributes / tool fix / new signature)
                                                          │
                                                          ▼
                                       dc.sh fix-tooling-incident --fixed --guard <path> ...
                                          • verifies <path> shows a recent change
                                          • appends to docs/tooling_incidents.md
                                          • clears the marker  → turn may now end
```

## Resolving an incident

When the gate blocks you, fix the **root cause** so it can never recur, then:

```bash
# Expected path — point at the file you created/changed to PREVENT recurrence:
bash tools/dc.sh fix-tooling-incident --fixed \
     --guard <path/you/created/or/changed> \
     --root-cause "<one line>" --verify "<how you confirmed it>"

# The guard misfired (help tighten the signature):
bash tools/dc.sh fix-tooling-incident --false-positive "<why this was not a real failure>"

# Genuinely can't guard against it right now (discouraged; logged as known-unfixed):
bash tools/dc.sh fix-tooling-incident --defer "<why>"
```

`--fixed` is gated: the `--guard` file must exist **and** show a recent change
(git-dirty or committed in the last few commits). You can't satisfy it by
naming an unrelated file — the point is *prevention*, not one-off repair. If you
only repaired this occurrence (e.g. re-ran `dc.sh bootstrap`) without preventing
recurrence, either make a guard (e.g. fix the bootstrap to self-heal) or use
`--defer`.

## Adding a signature (the extension model)

A new failure mode = add an entry to `tools/hooks/tooling_error_signatures.json`.
That single edit gives every future agent instant, specific guidance — and is a
legitimate `--guard` for resolving the incident that revealed the gap.

```jsonc
{
  "id": "my-new-failure",
  "class": "environment",
  "tier": "block",                  // "block" = marker+gate; "warn" = hint only
  "tools": ["Bash", "PowerShell"],
  "substrings": ["literal text from the error (case-insensitive)"],
  "regexes": ["optional \\bregex\\b when a substring won't do"],
  "summary": "one line shown to the agent",
  "root_cause": "why it happens",
  "permanent_fix": ["step 1", "step 2"],
  "suggested_guard": "what file/change prevents recurrence"
}
```

Prefer **substrings** (no escaping, low false-positive risk) over regexes. Keep
signatures HIGH-PRECISION: they run on *every* Bash/PowerShell result, so a
false positive that blocks legitimate work is worse than a miss. After adding
one, run the test suite.

## Testing

```bash
python3 tools/hooks/test_tooling_error_guard.py     # unit: classification + false-positive guard
bash    tools/hooks/test_tooling_incident_e2e.sh    # e2e: full loop (self-cleaning, non-polluting)
```

## Safety / escape hatches

The system is built to never trap a session on a bug or a false positive:

- **Fails open.** Any internal error in the detector or gate → allow (exit 0).
  The gate only blocks on an unambiguous, present, unresolved marker.
- **Per-worktree.** The marker lives at the git toplevel of the cwd, like
  `.bb2_active_func`. Worktrees don't interfere with each other.
- **Operator off-switches** (at the project root):
  - `touch .bb2_tooling_incident_suppress_once` — allow the next stop once.
  - `touch .bb2_tooling_guard_off` — disable the whole system (detector + gate).
- **High-precision signatures** + a logged `--false-positive` path so misfires
  become signal to tighten the catalog rather than a trap.
