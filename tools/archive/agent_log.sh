#!/usr/bin/env bash
# tools/agent_log.sh -- worker progress log for parallel orchestration.
#
# Workers in subagent worktrees call this to record key events. The
# coordinator (parent agent on main) reads the resulting logs at any
# time via `bash tools/dc.sh agent-status`.
#
# Logs are written to <MAIN_REPO>/tmp/parallel_logs/<func>.log -- one
# file per function, no concurrent writes (each worker owns its own
# function), no locking needed. Worker discovers MAIN_REPO via
# `git rev-parse --git-common-dir` (worktrees share the main gitdir).
#
# Usage:
#   bash tools/agent_log.sh <event> [details...]
#
# Examples:
#   bash tools/agent_log.sh setup-done
#   bash tools/agent_log.sh classified standard size=73
#   bash tools/agent_log.sh attempt NEAR_MISS score=42
#   bash tools/agent_log.sh diff ins=2 del=1 reord=4 reg=3
#   bash tools/agent_log.sh regfix-add subst @28
#   bash tools/agent_log.sh permuter-start budget=600s
#   bash tools/agent_log.sh permuter-done best=12 elapsed=540s
#   bash tools/agent_log.sh build OK
#   bash tools/agent_log.sh verify MATCH
#   bash tools/agent_log.sh matched recipe="LICM unhoist"
#   bash tools/agent_log.sh stuck reason="cycle persists" best=8
#
# Granularity: log MAJOR transitions and outcomes, NOT every shell
# call. ~10-30 events per worker is the right shape.
#
# Exits 0 on success and on most errors -- never block worker progress
# because logging failed.

set -uo pipefail

EVENT="${1:-status}"
shift || true
DETAILS="$*"

# Find main repo root (works from worktree or main). The git common dir
# is shared across all worktrees, so its parent is always main's root.
MAIN_GITDIR=$(git rev-parse --git-common-dir 2>/dev/null || true)
if [ -z "$MAIN_GITDIR" ]; then
    exit 0
fi
MAIN_GITDIR=$(cd "$MAIN_GITDIR" 2>/dev/null && pwd) || exit 0
MAIN_ROOT=$(cd "$(dirname "$MAIN_GITDIR")" 2>/dev/null && pwd) || exit 0

# Function name from active marker in current worktree
FUNC="unknown"
if [ -s ".bb2_active_func" ]; then
    FUNC=$(tr -d '[:space:]' < .bb2_active_func)
fi

# Worker ID = short suffix of branch name (worktree-agent-XXXXXXXX -> XXXXXXXX)
BRANCH=$(git symbolic-ref --short HEAD 2>/dev/null || echo "?")
WID="${BRANCH#worktree-agent-}"
WID="${WID:0:8}"
[ "$WID" = "?" ] && WID="main"

LOG_DIR="$MAIN_ROOT/tmp/parallel_logs"
mkdir -p "$LOG_DIR" 2>/dev/null || exit 0
LOG_FILE="$LOG_DIR/$FUNC.log"

TS=$(date +%H:%M:%S)
printf '%s\t%s\t%s\t%s\n' "$TS" "$WID" "$EVENT" "$DETAILS" >> "$LOG_FILE" 2>/dev/null || true

# Token budget self-check (audit-driven). After the 50th event for a
# function, emit a warning to stderr so the worker sees it on stdout
# capture. ~50 events is a rough proxy for 200K+ tokens spent.
EVENT_COUNT=$(wc -l < "$LOG_FILE" 2>/dev/null || echo 0)
if [ "${EVENT_COUNT:-0}" -ge 50 ] && [ "${EVENT_COUNT:-0}" -lt 60 ]; then
    if ! grep -q "TOKEN_BUDGET_WARNING" "$LOG_FILE" 2>/dev/null; then
        printf '%s\t%s\tTOKEN_BUDGET_WARNING\tevent_count=%s -- consider returning STUCK if not close to match\n' \
            "$TS" "$WID" "$EVENT_COUNT" >> "$LOG_FILE" 2>/dev/null
        echo "[agent_log] TOKEN BUDGET WARNING: $EVENT_COUNT events logged for $FUNC. Consider STUCK if not close to match." >&2
    fi
fi

exit 0
