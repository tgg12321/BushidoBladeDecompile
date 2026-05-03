#!/bin/bash
# PreToolUse hook for Bash. Enforces THE HARD RULE: once a function is
# selected (via `dc.sh next`), the agent must finish it before:
#   - committing code (git commit)
#   - reverting build files (git checkout/restore on src/*.c, regfix.txt, etc.)
#   - selecting another function (dc.sh next)
#
# State file: .bb2_active_func at project root.
#   - Written by `dc.sh next`
#   - Cleared by `dc.sh refresh-queue` (post-match) or `dc.sh release` (manual)
#
# Hook protocol: read JSON on stdin, exit 0 to allow, exit 2 to block
# (writes stderr to Claude as the block reason).

set -e

PROJECT_ROOT="/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
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

# Helper: run dc.sh verify <ACTIVE> via WSL, return 0 if MATCH.
# regfix_verify.py prints e.g. "func_X: MATCH (0 diffs in N bytes)" on
# success, or "func_X: M instruction(s) differ" on diff. We grep for
# "MATCH" — only the success case has that token in the first line.
verify_active_matches() {
    local out
    out=$(wsl bash -c "cd /mnt/c/Users/Trenton/Desktop/'Bushido Blade 2 Decompile' && bash tools/dc.sh verify $ACTIVE 2>&1" 2>/dev/null | head -1)
    case "$out" in
        *": MATCH "*) return 0 ;;
        *) return 1 ;;
    esac
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
BLOCKED: $ACTIVE is the active function (set by 'dc.sh next') but it
does not match in the current build. Per THE HARD RULE in
feedback_workflow_rules.md, you finish the function before committing.

To diagnose:
  wsl bash -c "cd /mnt/c/Users/Trenton/Desktop/'Bushido Blade 2 Decompile' && bash tools/dc.sh verify $ACTIVE"

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

# === Rule 3: block `dc.sh next` while ACTIVE is set ===
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
