#!/bin/bash
# tools/hooks/main_branch_guard.sh
#
# PreToolUse hook. Blocks file edits and history-mutating git ops in the
# MAIN worktree. Policy: all work happens in sub-worktrees, then lands in
# main via merge. Keeps main's working tree clean (no half-finished WIP,
# no active-function markers) and prevents two agents stepping on each
# other in the same checkout.
#
# Triggers on Edit|Write|MultiEdit|NotebookEdit and Bash.
#
# Blocked in main:
#   - All file edits (Edit / Write / MultiEdit / NotebookEdit)
#   - git commit / git rebase
#   - git reset --hard / --merge / --keep
#   - git checkout <ref> -- <file> (overwrites file from history)
#   - git restore (always overwrites)
#
# Allowed in main:
#   - All reads (Read / Grep / Glob)
#   - git merge <branch>     (land worktree work)
#   - git pull / git fetch    (sync with origin)
#   - git push                (publish main to origin)
#   - git log / status / diff / show / branch / worktree
#   - Any non-git Bash
#
# Self-skips when not in the main worktree (sub-worktrees have .git as
# a file pointing to the main repo's .git/worktrees/<name>).

set -u

# --- Resolve project root (same logic as session_start_brief.sh) ---
PROJECT_ROOT=""
if ROOT_FROM_CWD=$(git rev-parse --show-toplevel 2>/dev/null) && [ -d "$ROOT_FROM_CWD" ]; then
    PROJECT_ROOT="$ROOT_FROM_CWD"
elif [ -n "${CLAUDE_PROJECT_DIR:-}" ] && [ -d "$CLAUDE_PROJECT_DIR" ]; then
    PROJECT_ROOT="$CLAUDE_PROJECT_DIR"
else
    exit 0  # can't determine; fail-open
fi

# Windows-style path normalization (C:/foo or C:\foo → /c/foo).
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"
        REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

# --- Main-worktree detection ---
# In the main worktree, .git is a directory.
# In a sub-worktree, .git is a regular file pointing to <main>/.git/worktrees/<name>.
# Fail-open if neither (not a git repo or unusual state).
if [ ! -e "$PROJECT_ROOT/.git" ]; then
    exit 0
fi
if [ -f "$PROJECT_ROOT/.git" ]; then
    # Sub-worktree — this hook does not apply.
    exit 0
fi
if [ ! -d "$PROJECT_ROOT/.git" ]; then
    exit 0
fi

# We are in the main worktree. Parse the tool input.
INPUT=$(cat)
TOOL_NAME=$(printf '%s' "$INPUT" | python3 -c "import sys,json;print(json.load(sys.stdin).get('tool_name',''))" 2>/dev/null || echo "")

# Edit tools: hard-block (no allowlist).
case "$TOOL_NAME" in
    Edit|Write|MultiEdit|NotebookEdit)
        cat >&2 <<'EOF'
BLOCKED: file edits in the main worktree are disabled by project policy.

All work must happen in a sub-worktree, then land in main via merge.

Create one:
  bash tools/dc.sh new-worktree <name>      (recommended — one command)
  or use EnterWorktree in Claude Code
  or: git worktree add ../wt-<name>

Main is publish-only: clean tree, no .bb2_active_func, no WIP.
This prevents two agents stepping on each other in the same checkout.

Override (rare emergency only): comment out the main_branch_guard hook
in .claude/settings.local.json, do the edit, then restore it.
EOF
        exit 2
        ;;
    Bash)
        ;;
    *)
        exit 0
        ;;
esac

# Bash: parse the command and check for blocked git operations.
COMMAND=$(printf '%s' "$INPUT" | python3 -c "import sys,json;print(json.load(sys.stdin).get('tool_input',{}).get('command',''))" 2>/dev/null || echo "")
[ -z "$COMMAND" ] && exit 0

# Match a real git op (not embedded in `grep "git commit"` etc.) by
# requiring it to be at command-start, after `&&`, `||`, `;`, `|`, or
# inside `$(...)` / backticks.
#
# Patterns are tried one by one; first match wins and we emit a
# command-specific reason.
match_git_op() {
    local op_pattern="$1"
    # Anchor: command-start OR shell separator OR command substitution opener
    printf '%s' "$COMMAND" | grep -qE "(^|[;&|\`(])[[:space:]]*git( -c [^[:space:]]+)*[[:space:]]+$op_pattern"
}

BLOCKED_REASON=""
BLOCKED_ALT=""

if match_git_op 'commit([[:space:]]|$)'; then
    BLOCKED_REASON="git commit"
    BLOCKED_ALT="commits should originate in a worktree branch; merge into main"
elif match_git_op 'rebase([[:space:]]|$)'; then
    BLOCKED_REASON="git rebase"
    BLOCKED_ALT="rebase in your worktree branch, not on main"
elif match_git_op 'reset[[:space:]]+(--hard|--merge|--keep)'; then
    BLOCKED_REASON="git reset --hard / --merge / --keep"
    BLOCKED_ALT="if you need to reset main, do it intentionally outside Claude"
elif match_git_op 'restore([[:space:]]|$)'; then
    BLOCKED_REASON="git restore"
    BLOCKED_ALT="don't overwrite main's working tree; investigate the change first"
elif match_git_op 'checkout[[:space:]]+[^[:space:]]+[[:space:]]+--[[:space:]]'; then
    BLOCKED_REASON="git checkout <ref> -- <file>"
    BLOCKED_ALT="don't overwrite main's working tree from history"
fi

if [ -n "$BLOCKED_REASON" ]; then
    cat >&2 <<EOF
BLOCKED in main worktree: \`$BLOCKED_REASON\`.

Reason: $BLOCKED_ALT.

Main is publish-only. Allowed git ops here:
  - git merge <branch>     (land worktree work)
  - git pull / fetch        (sync with origin)
  - git push                (publish main to origin)
  - git log/status/diff/show/branch/worktree    (read-only)

To do real work: bash tools/dc.sh new-worktree <name>
EOF
    exit 2
fi

exit 0
