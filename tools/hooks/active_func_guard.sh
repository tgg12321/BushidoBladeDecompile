#!/bin/bash
# PreToolUse hook for Bash. Enforces THE HARD RULE: once a function is
# selected (via `dc.sh next`, `next-structural`, or `next-asmfix`), the
# agent must finish it before:
#   - committing code (git commit)
#   - reverting build files (git checkout/restore on src/*.c, regfix.txt, etc.)
#   - selecting another function (dc.sh next / next-structural / next-asmfix)
#
# State file: .bb2_active_func at project root.
#   - Written by `dc.sh next` and its alternate queue pullers
#   - Cleared by `dc.sh refresh-queue` (post-match) or `dc.sh release` (manual)
#
# Hook protocol: read JSON on stdin, exit 0 to allow, exit 2 to block
# (writes stderr to Claude as the block reason).

set -e

# Resolve project root: prefer the cwd's git toplevel (correct for both
# main-session and worktree subagent contexts since each session has its
# own cwd). Fall back to $CLAUDE_PROJECT_DIR, then to the script's own
# location's grandparent (script lives at <root>/tools/hooks/...).
#
# Why not hardcode: subagents spawned with isolation="worktree" run from
# their own worktree path. A hardcoded main path makes the hook check the
# main repo's .bb2_active_func and verify against main's build, while the
# subagent is editing a different working tree entirely.
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

ACTIVE_FILE="$PROJECT_ROOT/.bb2_active_func"

# Fast path: no active function -> allow everything (the common case)
if [ ! -s "$ACTIVE_FILE" ]; then
    exit 0
fi

ACTIVE=$(tr -d '[:space:]' < "$ACTIVE_FILE")
if [ -z "$ACTIVE" ]; then
    exit 0
fi

# Read tool input JSON. Pull the command field.
INPUT=$(cat)
COMMAND=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('command',''))" 2>/dev/null || echo "")

# Allow obvious safe commands quickly.
if [ -z "$COMMAND" ]; then
    exit 0
fi

# Convert PROJECT_ROOT to a WSL-mountable path: /c/foo -> /mnt/c/foo.
# Already-WSL or non-Windows paths pass through unchanged.
case "$PROJECT_ROOT" in
    /[a-zA-Z]/*) WSL_ROOT="/mnt$PROJECT_ROOT" ;;
    *)           WSL_ROOT="$PROJECT_ROOT" ;;
esac

# Helper: clean-rebuild + verify the active function AND the full
# binary via WSL. Returns 0 only if BOTH pass.
#
# Why clean rebuild: cached build/src/*.o files can hide regressions
# (the 54a5e54 → c71ff0a trap — a "matched" function compiled in a
# stale cache silently diverged from fresh-build output for 7 commits).
# Forcing a clean rebuild at commit time is the only way to guarantee
# the committed source actually produces the matching binary.
#
# Cost: ~2-3 minutes per commit. Worth it — the alternative is silent
# regressions accumulating across many commits.
verify_active_matches() {
    local out per_func sha1_ok
    out=$(wsl bash -c "cd '$WSL_ROOT' && rm -rf build && source .venv/bin/activate 2>/dev/null; make 2>&1 | tail -3 && echo '---SEP---' && bash tools/dc.sh verify $ACTIVE 2>&1" 2>/dev/null)
    # SHA1 line shows up as "OK: bb2 matches!" on full match.
    case "$out" in
        *"OK: bb2 matches!"*) sha1_ok=1 ;;
        *) sha1_ok=0 ;;
    esac
    # Per-function line — only matters if SHA1 didn't already match
    # (SHA1-clean implies all functions match).
    case "$out" in
        *": MATCH "*) per_func=1 ;;
        *) per_func=0 ;;
    esac
    if [ "$sha1_ok" = "1" ]; then
        return 0
    fi
    return 1
}

# === Rule 1: block `git commit` unless ACTIVE matches in the build ===
# We check for the literal `git commit` substring (covers `git commit -m`,
# `git commit -F`, heredocs, etc.). We deliberately don't try to parse
# arguments -- if the command says "git commit" anywhere, we gate it.
if echo "$COMMAND" | grep -qE '(^|[^a-zA-Z])git commit($|[^a-zA-Z])'; then
    if verify_active_matches; then
        # Function matches; allow commit and clear the state.
        : > "$ACTIVE_FILE"
        exit 0
    else
        cat >&2 <<EOF
BLOCKED: $ACTIVE is the active function (set by a dc.sh queue pull) but
the CLEAN REBUILD (rm -rf build && make) does NOT SHA1-match the
original. Per THE HARD RULE in feedback_workflow_rules.md, you finish
the function before committing.

Note: this hook does a clean rebuild on every commit attempt, not a
cached \`verify\`. This is intentional — cached .o files have hidden
regressions across many commits in the past (the 54a5e54 trap).

Repo:  $PROJECT_ROOT

To diagnose:
  wsl bash -c "cd '$WSL_ROOT' && bash tools/dc.sh verify --clean"

To proceed: get the function to match (work the toolbox until score=0
and verify reports MATCH), then re-attempt the commit. The hook will
auto-clear the active marker on success.

If the user has explicitly authorized abandoning this function:
  bash tools/dc.sh release   # requires typing the function name to confirm
EOF
        exit 2
    fi
fi

# === Rule 2: block reverts of in-progress build files ===
# Protected files: src/*.c, src/*.h, regfix*.txt, asmfix.txt,
# undefined_syms_auto.txt, named_syms.txt, sdata*.txt, expand_lb_funcs.txt
if echo "$COMMAND" | grep -qE 'git (checkout|restore|reset.+--)\b.*\b(src/|regfix|asmfix|undefined_syms|named_syms|sdata|expand_lb)'; then
    cat >&2 <<EOF
BLOCKED: $ACTIVE is in progress. Reverting build files would erase
work-in-progress. Per THE HARD RULE you finish the function first.

If the user has explicitly authorized abandoning this function:
  bash tools/dc.sh release   # requires typing the function name to confirm
  # Then this revert would be allowed.

If you're trying to recover from a broken state, identify the specific
file you need to fix and edit it forward, not by reverting.
EOF
    exit 2
fi

# === Rule 3: block queue pulls while ACTIVE is set ===
# (dc.sh self-blocks too, but having the hook layer makes the intent
# visible in the agent's output.)
if echo "$COMMAND" | grep -qE 'dc\.sh next($|[^a-zA-Z])'; then
    cat >&2 <<EOF
BLOCKED: $ACTIVE is still in progress. You don't pull a new function
until the current one matches and is committed.

To proceed: finish $ACTIVE. Use the toolbox in
feedback_matching_playbook.md (escalation ladder: C variants ->
register asm -> long permuter -> regfix -> compound regfix -> named
recipes -> new pipeline pass).
EOF
    exit 2
fi

# Anything else: allow.
exit 0
