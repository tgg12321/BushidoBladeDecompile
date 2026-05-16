#!/bin/bash
# SessionStart hook. Runs `dc.sh start` and emits the briefing as
# additional context for the agent. Output goes via stdout in JSON
# format per the Claude Code hook protocol; we use additionalContext
# to prepend the briefing to the session.
#
# Also tracks "quit count" — when a session starts with .bb2_active_func
# set AND the build is unmatched, that means a previous session left this
# function unfinished. We bump a per-function counter in
# .bb2_quit_log.json and prepend a shame banner to the briefing so the
# new agent sees how many prior agents quit on this function.

set -e

# Resolve project root from cwd's git toplevel so this hook works correctly
# in git worktrees (sub-agent isolation). Fall back to $CLAUDE_PROJECT_DIR
# then to the script's own grandparent. Same pattern as active_func_guard.sh
# and grind_check.sh — keep these three in sync.
PROJECT_ROOT=""
if ROOT_FROM_CWD=$(git rev-parse --show-toplevel 2>/dev/null) && [ -d "$ROOT_FROM_CWD" ]; then
    PROJECT_ROOT="$ROOT_FROM_CWD"
elif [ -n "${CLAUDE_PROJECT_DIR:-}" ] && [ -d "$CLAUDE_PROJECT_DIR" ]; then
    PROJECT_ROOT="$CLAUDE_PROJECT_DIR"
else
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
fi

# Normalize Windows-style paths (C:/foo or C:\foo) to bash form (/c/foo).
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"
        REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

# Derive WSL mount path for the wsl bash -c invocations below.
case "$PROJECT_ROOT" in
    /[a-zA-Z]/*) WSL_ROOT="/mnt$PROJECT_ROOT" ;;
    *)           WSL_ROOT="$PROJECT_ROOT" ;;
esac

ACTIVE_FILE="$PROJECT_ROOT/.bb2_active_func"
QUIT_LOG="$PROJECT_ROOT/.bb2_quit_log.json"

# Collect the briefing. Run dc.sh start via WSL because dc.sh expects
# the WSL path layout. Strip the systemd warning line that wsl emits.
BRIEFING=$(wsl bash -c "cd '$WSL_ROOT' && bash tools/dc.sh start 2>&1" \
    | grep -v "Failed to start.*systemd" || true)

# If there's an unfinished active function, bump the quit counter and
# prepend a shame banner to the briefing.
QUIT_BANNER=""
if [ -s "$ACTIVE_FILE" ]; then
    ACTIVE=$(tr -d '[:space:]' < "$ACTIVE_FILE")
    if [ -n "$ACTIVE" ]; then
        VERIFY=$(wsl bash -c "cd '$WSL_ROOT' && bash tools/dc.sh verify $ACTIVE 2>&1" 2>/dev/null | head -1)
        case "$VERIFY" in
            *": MATCH "*)
                # Function matches; the marker is stale (would be cleared
                # by next git commit). No quit happened.
                : ;;
            *)
                # Unmatched + active marker = a previous agent left this
                # function unfinished. Bump the counter.
                COUNT=$(QUIT_LOG="$QUIT_LOG" ACTIVE="$ACTIVE" python3 - <<'PY'
import json, os, datetime
path = os.environ['QUIT_LOG']
func = os.environ['ACTIVE']
try:
    with open(path, 'r') as f:
        data = json.load(f)
except (FileNotFoundError, json.JSONDecodeError):
    data = {}
entry = data.setdefault(func, {'count': 0, 'first_seen': None, 'last_seen': None})
entry['count'] += 1
now = datetime.datetime.now().isoformat(timespec='seconds')
if not entry['first_seen']:
    entry['first_seen'] = now
entry['last_seen'] = now
with open(path, 'w') as f:
    json.dump(data, f, indent=2, sort_keys=True)
print(entry['count'])
PY
)
                if [ -n "$COUNT" ] && [ "$COUNT" != "0" ]; then
                    QUIT_BANNER="=== QUIT-COUNT TRACKER ===

This is the ${COUNT} time a session has resumed $ACTIVE without it matching.
Each prior agent left this function unfinished and the next session inherited it.
That pattern is the failure mode. Don't add to the count: finish $ACTIVE this turn.
The Stop-event hook (tools/hooks/grind_check.sh) will block any wrap-up
language while this marker is set and the build is MISMATCH.

"
                fi ;;
        esac
    fi
fi

FULL="${QUIT_BANNER}${BRIEFING}"

# Emit JSON. Claude Code reads `additionalContext` and injects it as
# system context for the session — visible to the agent on first turn.
FULL="$FULL" python3 -c "
import json, os
print(json.dumps({
    'hookSpecificOutput': {
        'hookEventName': 'SessionStart',
        'additionalContext': os.environ['FULL'],
    }
}))
"
