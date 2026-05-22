#!/bin/bash
# Stop / SubagentStop hook. Fires when the agent tries to end its turn.
#
# Purpose: enforce the debugging-discipline rule. If a KNOWN tooling failure
# (CRLF, WSL-unavailable, broken worktree symlink, missing dep, ...) was
# detected this turn by tools/hooks/tooling_error_guard.py (PostToolUse), an
# incident marker (.bb2_tooling_incident.json) was written. The agent may NOT
# end the turn by papering over a tooling error -- it must root-cause and fix
# it PERMANENTLY, then clear the marker via `dc.sh fix-tooling-incident`.
#
# Hook protocol: read JSON on stdin {stop_hook_active, ...}. Emit
# {"decision":"block","reason":"..."} on stdout to keep the agent going; emit
# nothing + exit 0 to allow the stop.
#
# SAFETY (mirrors grind_check.sh): we FAIL OPEN. If we can't resolve the root,
# the marker is absent/empty/already-resolved, or anything goes wrong, we allow
# the stop. We only block when there is an unambiguous, unresolved incident.
# Operator escape hatches: .bb2_tooling_guard_off (permanent) and
# .bb2_tooling_incident_suppress_once (one-shot).

INPUT=$(cat)

# Don't recurse: if this Stop was itself triggered while a stop hook is active.
HOOK_ACTIVE=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('true' if d.get('stop_hook_active') else 'false')" 2>/dev/null || echo "false")
[ "$HOOK_ACTIVE" = "true" ] && exit 0

# Project root STRICTLY from cwd's git toplevel (per-worktree marker).
PROJECT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)
{ [ -z "$PROJECT_ROOT" ] || [ ! -d "$PROJECT_ROOT" ]; } && exit 0
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"; REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

# Operator overrides.
[ -f "$PROJECT_ROOT/.bb2_tooling_guard_off" ] && exit 0
if [ -f "$PROJECT_ROOT/.bb2_tooling_incident_suppress_once" ]; then
    rm -f "$PROJECT_ROOT/.bb2_tooling_incident_suppress_once"
    exit 0
fi

MARKER="$PROJECT_ROOT/.bb2_tooling_incident.json"
[ ! -s "$MARKER" ] && exit 0

# Build the block reason from the marker. If python3 can't parse it (and the
# marker is non-empty), fail CLOSED with a generic reason -- a present marker
# means an unresolved incident regardless of our ability to pretty-print it.
REASON=$(MARKER="$MARKER" python3 <<'PY' 2>/dev/null
import json, os, sys
try:
    d = json.load(open(os.environ["MARKER"], encoding="utf-8"))
except Exception:
    sys.exit(3)
if d.get("status") != "unresolved":
    sys.exit(4)  # already resolved -> allow stop
incs = d.get("incidents") or []
if not incs:
    sys.exit(4)
latest = incs[-1]
fix = "\n".join(f"  {i+1}. {s}" for i, s in enumerate(latest.get("permanent_fix", [])))
print(f"""STOP BLOCKED -- unresolved tooling incident ({d.get('count', len(incs))} occurrence(s)).

You hit a KNOWN, permanently-fixable tooling failure this turn and have not
resolved it. Per the debugging-discipline rule, you may NOT end the turn by
working around a tooling error -- fix the ROOT CAUSE so no future agent hits it.

Latest: {latest.get('class')}/{latest.get('signature')}
  {latest.get('summary')}
  Root cause: {latest.get('root_cause')}

Fix it permanently:
{fix}
  Suggested guard: {latest.get('suggested_guard')}

Then clear the marker (this is the ONLY clean way to end the turn):
  bash tools/dc.sh fix-tooling-incident --fixed \\
       --guard <path/you/created/or/changed> \\
       --root-cause "<one line>" --verify "<how you confirmed it>"

Misfire?              bash tools/dc.sh fix-tooling-incident --false-positive "<why>"
Truly unfixable now?  bash tools/dc.sh fix-tooling-incident --defer "<why>"

Operator overrides: touch .bb2_tooling_incident_suppress_once (allow this one stop)
                    touch .bb2_tooling_guard_off               (disable the guard)
Full record: .bb2_tooling_incident.json""")
PY
)
RC=$?

# RC 4 = marker resolved/empty -> allow stop. RC 3 / other parse failure -> the
# marker exists but is unparseable: fail CLOSED with a generic reason.
if [ "$RC" -eq 4 ]; then
    exit 0
fi
if [ -z "$REASON" ]; then
    REASON="STOP BLOCKED -- an unresolved tooling incident marker exists (.bb2_tooling_incident.json).
Fix the root cause permanently, then run: bash tools/dc.sh fix-tooling-incident --fixed --guard <path> --root-cause \"...\" --verify \"...\"
(Override: touch .bb2_tooling_incident_suppress_once)"
fi

REASON="$REASON" python3 -c "
import json, os
# strip CRs that Windows text-mode stdout may have injected into the captured reason
print(json.dumps({'decision':'block','reason':os.environ['REASON'].replace(chr(13),'')}))
" 2>/dev/null
exit 0
