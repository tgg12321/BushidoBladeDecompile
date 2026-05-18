#!/bin/bash
# PreToolUse hook for Bash. When a `git commit` would add an "escape
# valve" pattern (INLINE_MOVE_ALIASING block, asmfix bridge entry,
# inline_asm_canonical addition), require the affected function's
# .bb2_attempts/<func>.jsonl log to pass tools/check_attempts.py.
#
# Rationale: the documented escape valves accept "≥2 failed pure-C
# attempts" as sufficient justification. That bar is trivially met by
# agents that try 2-3 variations of one technique. This hook enforces
# attempts span ≥4 distinct technique CATEGORIES before allowing the
# commit, forcing genuine breadth of effort.
#
# Detection: scans `git diff --cached` for staged additions of:
#   1. New `INLINE_MOVE_ALIASING:` comment blocks in src/*.c
#   2. New `<func>: replace_with_asmfile "..."` lines in asmfix.txt
#      (uncommented; commented-out is fine — that's RETIRING a bridge)
#   3. New `<func>:` lines in inline_asm_canonical.txt
#
# For each affected function, runs check_attempts.py. ANY function
# failing the gate blocks the commit. Hook output is the rejection
# reason shown to the agent.
#
# Self-skips when no escape valve is being added (the common case for
# routine matches that don't need escape valves).

set -u

# --- Resolve project root from cwd ---
PROJECT_ROOT=""
if ROOT_FROM_CWD=$(git rev-parse --show-toplevel 2>/dev/null) && [ -d "$ROOT_FROM_CWD" ]; then
    PROJECT_ROOT="$ROOT_FROM_CWD"
elif [ -n "${CLAUDE_PROJECT_DIR:-}" ] && [ -d "$CLAUDE_PROJECT_DIR" ]; then
    PROJECT_ROOT="$CLAUDE_PROJECT_DIR"
else
    exit 0
fi

case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"
        REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

# --- Parse tool input ---
INPUT=$(cat)
COMMAND=$(printf '%s' "$INPUT" | python3 -c "import sys,json;print(json.load(sys.stdin).get('tool_input',{}).get('command',''))" 2>/dev/null || echo "")
[ -z "$COMMAND" ] && exit 0

# Only fire on git commit. Be permissive about the form
# (`git commit -m ...`, `git commit -F ...`, etc.) but anchor to a
# real shell-position git commit.
if ! echo "$COMMAND" | grep -qE '(^|[;&|`(])[[:space:]]*git( -c [^[:space:]]+)*[[:space:]]+commit([[:space:]]|$)'; then
    exit 0
fi

# Convert to WSL path for the wsl bash -c invocations below.
case "$PROJECT_ROOT" in
    /[a-zA-Z]/*) WSL_ROOT="/mnt$PROJECT_ROOT" ;;
    *)           WSL_ROOT="$PROJECT_ROOT" ;;
esac

# --- Detect escape-valve additions in staged diff ---
# Use Git Bash's git directly (where this hook is running). Going
# through WSL hits the .git-pointer-file confusion in worktrees
# (gitdir lines written as Windows paths break under WSL).
# Use $PROJECT_ROOT (Git Bash form) for -C so git operates on the
# right worktree from any cwd.
DIFF=$(git -C "$PROJECT_ROOT" diff --cached 2>/dev/null || echo "")
if [ -z "$DIFF" ]; then
    # Nothing staged — let the commit fail naturally with git's own error.
    exit 0
fi

# Collect (function, escape_type) tuples in $AFFECTED, one per line.
# scan_escape_valves.py reads the diff on stdin and emits one
# `<func>\t<kind>` line per detected escape-valve addition.
# Run python in WSL since that's where the venv lives.
AFFECTED=$(printf '%s' "$DIFF" | wsl bash -c "cd '$WSL_ROOT' && python3 tools/scan_escape_valves.py 2>/dev/null" || true)

[ -z "$AFFECTED" ] && exit 0

# --- For each affected function, run check_attempts.py ---
FAILED=""
DETAILS=""
while IFS=$'\t' read -r FUNC KIND; do
    [ -z "$FUNC" ] && continue
    if [ "$FUNC" = "<unknown-from-diff>" ]; then
        FAILED="$FAILED\n  unknown function (INLINE_MOVE_ALIASING added but the comment block's target function couldn't be parsed from the diff context)"
        continue
    fi
    VERDICT=$(wsl bash -c "cd '$WSL_ROOT' && python3 tools/check_attempts.py '$FUNC' 2>&1")
    RC=$?
    if [ "$RC" -ne 0 ]; then
        FAILED="$FAILED\n  $FUNC ($KIND)"
        DETAILS="$DETAILS\n--- $FUNC ---\n$VERDICT\n"
    fi
done <<EOF
$AFFECTED
EOF

if [ -z "$FAILED" ]; then
    exit 0
fi

cat >&2 <<EOF
BLOCKED: escape-valve commit requires sufficient documented pure-C effort.

This commit adds an escape-valve pattern (INLINE_MOVE_ALIASING comment,
asmfix bridge entry, and/or inline_asm_canonical declaration) for:
$(printf '%b' "$FAILED")

Per the project's anti-shortcut policy, escape valves require the
.bb2_attempts/<func>.jsonl log to demonstrate genuine breadth of
pure-C effort: ≥4 distinct technique categories, ≥6 attempts, ≥30
cumulative minutes. The current logs don't meet that bar.

$(printf '%b' "$DETAILS")

To proceed:
  - Try more pure-C variants in the missing categories (each variant
    you actually run: log it via \`dc.sh log-attempt <func> <category>
    <variant> <minutes> '<outcome>'\`).
  - When the gate passes, retry the commit.
  - If you genuinely exhausted everything and the user has explicitly
    approved this escape valve, add an OVERRIDE entry:
      bash tools/dc.sh log-attempt <func> OVERRIDE override \\
        "user approved 2026-XX-XX after N-hour effort: <details>"

The point is to prevent the "I tried 3 variations of one approach and
called it exhausted" failure mode. Distinct categories = different
KINDS of fix, not just different variants of the same fix.
EOF
exit 2
