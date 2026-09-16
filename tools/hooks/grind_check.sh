#!/bin/bash
# Stop hook for GRINDER sessions — the semantic stop-gate the agent registry's
# resilience-judge was always meant to sit behind (the earlier keyword filter was
# trivially paraphrased around; this is a deterministic registry check instead).
#
# THE BUG IT CLOSES: a one-shot headless grind session launches a permuter
# campaign via tools/permuter_campaign.py (which DETACHES it, start_new_session),
# then ends its turn "waiting" for the campaign — orphaning it. Because the turn
# is over, no outcome JSON is written, and the driver discards the ENTIRE session
# as invalid. The role text forbids this in prose, but capable models paraphrase
# around prose ("waiting for monitor events", 2026-07-20). This hook makes the
# rule mechanical: you may not end your turn while a campaign you launched is
# still registered active.
#
# ARMING: no-op unless GRIND_FUNC is set in the environment. Only the grinder's
# spawn sets it (tools/grinder/grind.ps1), so interactive/operator sessions and
# the grind session's own subagents (this is wired Stop-only, not SubagentStop)
# are never affected.
#
# INVARIANT it relies on: the grinder zeroes every registry 'active' flag at each
# session boundary (permuter_campaign.py deactivate-all in Reap-PermuterOrphans),
# so an entry that is func==GRIND_FUNC AND active==true at Stop time was orphaned
# by THIS session — never a stale leftover. `harvest --stop` clears active; a
# session that properly stops its campaigns passes this gate.
#
# Hook protocol: read JSON on stdin. Emit {"decision":"block","reason":"..."} on
# stdout to keep the agent going; emit nothing + exit 0 to allow the stop.
#
# SAFETY: FAIL OPEN. Any error, missing file, unparseable JSON, or no active
# campaign for this func -> allow the stop. We only block on an unambiguous,
# still-active campaign for the current grind function.

INPUT=$(cat)

# Don't recurse.
HOOK_ACTIVE=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('true' if d.get('stop_hook_active') else 'false')" 2>/dev/null || echo "false")
[ "$HOOK_ACTIVE" = "true" ] && exit 0

# Only arm for grinder sessions.
[ -z "$GRIND_FUNC" ] && exit 0

# Project root STRICTLY from cwd's git toplevel.
PROJECT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)
{ [ -z "$PROJECT_ROOT" ] || [ ! -d "$PROJECT_ROOT" ]; } && exit 0
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"; REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

# Operator escape hatch (mirrors the tooling guard).
[ -f "$PROJECT_ROOT/.bb2_grind_guard_off" ] && exit 0

# Emit a block decision and leave. Bash cannot be trusted to escape a
# multi-line reason into JSON, so python does it.
emit_block() {
    REASON="$1" python3 -c "
import json, os
print(json.dumps({'decision':'block','reason':os.environ['REASON'].replace(chr(13),'')}))
" 2>/dev/null
    exit 0
}

# ── GATE 1: an orphaned permuter campaign (a LIVE process) ──────────────────
REGISTRY="$PROJECT_ROOT/tmp/permuter_campaigns.json"
ACTIVE_DIRS=""
if [ -s "$REGISTRY" ]; then
# Find campaigns still active for GRIND_FUNC. Prints their dirs, one per line;
# prints nothing (rc!=0 or empty) when there is nothing to block on.
ACTIVE_DIRS=$(REGISTRY="$REGISTRY" FUNC="$GRIND_FUNC" python3 <<'PY' 2>/dev/null
import json, os, sys
try:
    reg = json.load(open(os.environ["REGISTRY"], encoding="utf-8"))
except Exception:
    sys.exit(0)  # unreadable -> fail open (no output)
func = os.environ["FUNC"]
dirs = [d for d, info in reg.items()
        if info.get("active") and info.get("func") == func]
for d in dirs:
    print(d)
PY
)
fi

if [ -n "$ACTIVE_DIRS" ]; then
# Build the harvest command list for the reason text.
HARVEST_LINES=$(echo "$ACTIVE_DIRS" | while IFS= read -r d; do
    [ -n "$d" ] && echo "  python tools/permuter_campaign.py harvest --dir \"$d\" --stop --reason \"session end\""
done)

REASON="STOP BLOCKED -- you launched a permuter campaign for $GRIND_FUNC and are
ending your turn while it is still registered ACTIVE. A grind session is a
ONE-SHOT process: the instant you stop, your process exits, the detached
campaign is orphaned, and because no outcome JSON is on disk your ENTIRE session
is discarded as if it never ran (this is the exact failure the role warns about).

You must finish the campaign IN THIS TURN before stopping:
$HARVEST_LINES
Then bank any finds (candidate.c / rejected/*.c), record the measured result,
and WRITE YOUR OUTCOME JSON to the path in your brief. Only then end your turn.

A 0-find harvest is a valid data point -- harvest --stop it and report the
KILLED hypothesis. Never end your turn 'waiting' on a campaign; nothing will
wake you.

(Operator override: touch .bb2_grind_guard_off to disable this gate.)"

    emit_block "$REASON"
fi

# ── GATES 2 & 3: outcome-schema validity and file scope ─────────────────────
# The driver runs these same predicates the moment the turn ends (grind.ps1
# steps 5 and 6), where the only verdict left is DISCARD — the whole session,
# work included. Running them HERE, while the agent is still alive, converts a
# lost session into an in-session repair. Measured 2026-09-08..16: 61 of 423
# sessions discarded, 34 outcome-schema + 17 scope.
#
# stopgate.py prints a reason or nothing, and always exits 0 (fail open); it
# caps itself at 3 blocks per session so a stuck agent can never loop.
GATE_REASON=$(cd "$PROJECT_ROOT" && python3 tools/grinder/stopgate.py "$PROJECT_ROOT" 2>/dev/null)
[ -n "$GATE_REASON" ] && emit_block "$GATE_REASON

(Operator override: touch .bb2_grind_guard_off to disable this gate.)"

exit 0
