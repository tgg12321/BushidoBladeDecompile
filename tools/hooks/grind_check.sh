#!/bin/bash
# Stop-event hook. Fires when the agent finishes its turn.
#
# Purpose: catch voluntary-stop attempts. Per the user's directive, the agent
# may NOT decide to end work on an active function. Stopping is allowed only
# when:
#   - dc.sh verify <active> reports MATCH
#   - the classifier returned permanently_blocked:* (already gated out by
#     dc.sh next), or
#   - the user explicitly told the agent to stop in this session.
#
# This hook reads the transcript, extracts the assistant's last turn, and
# checks for stop-language patterns ("next session", "wrap up", "preserved
# state", etc.). If detected AND .bb2_active_func is set AND the build is
# MISMATCH, it returns decision=block with a reason that forces the agent
# to keep working.
#
# Hook protocol: read JSON on stdin with {session_id, transcript_path,
# stop_hook_active}. If stop_hook_active=true, exit 0 (we're already in a
# hook-response loop; don't re-trigger). Otherwise emit JSON on stdout with
# {decision: "block", reason: "..."} to keep the agent going.

set -e

INPUT=$(cat)

# If we're already inside a stop-hook-triggered response, do nothing.
# Otherwise we'd loop forever.
HOOK_ACTIVE=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('true' if d.get('stop_hook_active') else 'false')" 2>/dev/null || echo "false")
if [ "$HOOK_ACTIVE" = "true" ]; then
    exit 0
fi

TRANSCRIPT=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('transcript_path',''))" 2>/dev/null || echo "")
if [ -z "$TRANSCRIPT" ] || [ ! -f "$TRANSCRIPT" ]; then
    exit 0
fi

# Resolve project root the same way active_func_guard.sh does.
PROJECT_ROOT=""
if ROOT_FROM_CWD=$(git rev-parse --show-toplevel 2>/dev/null) && [ -d "$ROOT_FROM_CWD" ]; then
    PROJECT_ROOT="$ROOT_FROM_CWD"
elif [ -n "${CLAUDE_PROJECT_DIR:-}" ] && [ -d "$CLAUDE_PROJECT_DIR" ]; then
    PROJECT_ROOT="$CLAUDE_PROJECT_DIR"
else
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
fi

case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"
        REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

ACTIVE_FILE="$PROJECT_ROOT/.bb2_active_func"

# Fast path: no active marker, allow stop.
if [ ! -s "$ACTIVE_FILE" ]; then
    exit 0
fi

ACTIVE=$(tr -d '[:space:]' < "$ACTIVE_FILE")
if [ -z "$ACTIVE" ]; then
    exit 0
fi

# Check whether the function actually matches. If MATCH, the marker is stale
# (would have been cleared by the next git commit anyway) — allow stop.
case "$PROJECT_ROOT" in
    /[a-zA-Z]/*) WSL_ROOT="/mnt$PROJECT_ROOT" ;;
    *)           WSL_ROOT="$PROJECT_ROOT" ;;
esac

VERIFY=$(wsl bash -c "cd '$WSL_ROOT' && bash tools/dc.sh verify $ACTIVE 2>&1" 2>/dev/null | head -1)
case "$VERIFY" in
    *": MATCH "*) exit 0 ;;
esac

# Function is unmatched. Now check the assistant's last message for
# stop-language. Only block if we see it; if the agent's last turn ended
# silently or just asked a clarifying question, that's a different problem.
#
# Extract the last assistant text from the transcript JSONL. Each line is
# one JSON event. Assistant text events have type="assistant" and a content
# array containing text blocks.
LAST_TEXT=$(python3 - "$TRANSCRIPT" <<'PY' 2>/dev/null || true
import json, sys
path = sys.argv[1]
chunks = []
try:
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                ev = json.loads(line)
            except Exception:
                continue
            if ev.get('type') != 'assistant':
                continue
            msg = ev.get('message') or {}
            content = msg.get('content') or []
            for block in content:
                if isinstance(block, dict) and block.get('type') == 'text':
                    chunks.append(block.get('text', ''))
except Exception:
    pass
# Only inspect the last ~3 assistant text blocks (the recent turn).
print('\n'.join(chunks[-3:]).lower())
PY
)

if [ -z "$LAST_TEXT" ]; then
    # No recent assistant text to scan — don't block on silence.
    exit 0
fi

# Stop-language patterns. These are phrases that agents say when they're
# wrapping up voluntarily. Add to this list as new patterns are observed.
PATTERNS=(
    "next session"
    "future session"
    "partial state"
    "best partial"
    "preserved state"
    "preserve.* state"
    "preserved in (the )?working tree"
    "leaving (this|the|it) (in )?(the )?working"
    "wrap.up"
    "wrap up"
    "wrapping up"
    "diminishing returns"
    "good place to stop"
    "good stopping point"
    "natural pause"
    "let me leave"
    "let me commit"
    "leave (it|this) (here|for|in)"
    "i'?ve (made )?(substantial|significant|good|major) progress"
    "i'?ve done (a lot|enough|significant)"
    "progress summary"
    "final summary"
    "session-end summary"
    "stop here"
    "calling (it|this) (a session|done|for now)"
    "tap out"
    "for now"
    "for this session"
    "this iteration"
    "let the next session"
    "the next session can"
    "next session can"
    "pick up from here"
    "pick up where i left"
    "continued in the next"
    "given the volume"
    "given the (depth|complexity)"
    "more.{0,30}work needed"
    "more.{0,30}rules.{0,30}needed"
    "more (regfix|substs|surgery)"
    "[0-9]+\+? more rules"
    "is enough for one session"
    "this is enough"
    "i'?ve exhausted"
    "exhausted .* angle"
    "session ends? before"
    "ending this session"
    "out of (time|energy|reasonable)"
    "i'?ve (been )?at this for"
    "many hours"
    "1\.5 hours"
    "[0-9]+\+? hours"
    "[0-9]+ iterations"
    "diminishing"
    # Rationalization patterns observed in func_80070C70 STUCK returns:
    "tractable bound"
    "exceeds the tool"
    "cannot be coerced"
    "structural .{0,30}cannot be"
    "regalloc cannot"
    "register allocation cannot"
    "[0-9]+ attempts.{0,40}none"
    "reached -[0-9]+"
    "still requires ~?[0-9]+"
    "unreachable via"
    "path [a-z] exhausted"
    "[0-9]+ different .{0,20}variants"
    "tried [0-9]+ "
    "consumed.{0,20}attempts"
    "no improvement after"
    "reached the limit"
    "given.{0,40}directive"
    "ground hard"
    "grinded hard"
    "i'?m going to be honest"
    "honestly,? at this point"
    "let me make.{0,30}final"
    "one more (push|attempt|try)"
    "one final"
    "final attempt"
    "to wrap"
)

# Build a single grep -E pattern from the array (joined with |).
JOINED=$(IFS='|'; echo "${PATTERNS[*]}")

# grep -E -- exit 0 if any pattern matched. The text was already lowercased.
if echo "$LAST_TEXT" | grep -qE "$JOINED"; then
    # Block — emit JSON on stdout per Claude Code Stop-hook protocol.
    REASON="STOP-HOOK BLOCKED: you wrote quit-language while $ACTIVE is unmatched and active.

Build: dc.sh verify $ACTIVE -> $VERIFY

Per the user's directive (2026-05-08): a 'genuine blocker' is *literally
impossible to decompile to C*. Time, difficulty, regfix volume, and
'diminishing returns' are NOT blockers. The HARD RULE in
feedback_workflow_rules.md still applies.

The only valid reasons to stop work on an active function are:
  (a) dc.sh verify <active> reports MATCH
  (b) classifier returns permanently_blocked:* (already gated out by the queue)
  (c) the user explicitly told you to stop in THIS session
  (d) WSL/repo physically broken (catastrophic), or
  (e) the platform auto-compacts your context (you don't decide that).

You don't get to declare 'session end' yourself. 'Multi-session OK' in
CLAUDE.md is a fallback for context-exhaustion, not permission to quit.

Continue working: switch *technique*, not target. Tool calls only — no
more progress summaries. Use dc.sh diff/diff-align/regfix-suggest, write
the next regfix rule, build, verify. Repeat until verify reports MATCH."

    REASON="$REASON" python3 -c "
import json, os
print(json.dumps({
    'decision': 'block',
    'reason': os.environ['REASON'],
}))
"
    exit 0
fi

# No stop-language detected — allow the stop.
exit 0
