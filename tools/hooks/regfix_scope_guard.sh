#!/bin/bash
# PreToolUse hook on Edit/Write. Blocks edits to regfix.txt/asmfix.txt that
# REMOVE rules for functions other than the active one.
#
# Why: subagent #1 on func_8007D3F8 used Write/Edit to strip 79 rules across
# func_8005D554, func_80074B18, func_8007352C — destroying matched-function
# state. The PreToolUse hook on Bash (active_func_guard.sh) doesn't catch
# Edit/Write tool calls. This hook does.
#
# Logic:
#   - Read .bb2_active_func.
#   - If unset, allow everything (no active context).
#   - Read tool input. If file_path is not regfix.txt/asmfix.txt, allow.
#   - For Edit: examine old_string. If it contains any rule line for a
#     function other than the active one, block (the user may be moving the
#     line, but more often they're removing it).
#   - For Write: compare current file vs proposed content; if any line for a
#     non-active function is being removed, block.
#
# Bypass: set BB2_SCOPE_OVERRIDE=1 in the agent's env. The hook checks the
# parent process env (best-effort) and falls back to the var-in-input pattern.
#
# Hook protocol: read JSON on stdin, exit 0 to allow, exit 2 with stderr
# message to block (the message is shown to the agent as the rejection).

set -e

INPUT=$(cat)

# Resolve project root: prefer git toplevel.
PROJECT_ROOT=""
if ROOT_FROM_CWD=$(git rev-parse --show-toplevel 2>/dev/null) && [ -d "$ROOT_FROM_CWD" ]; then
    PROJECT_ROOT="$ROOT_FROM_CWD"
elif [ -n "${CLAUDE_PROJECT_DIR:-}" ] && [ -d "$CLAUDE_PROJECT_DIR" ]; then
    PROJECT_ROOT="$CLAUDE_PROJECT_DIR"
else
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
fi

# Normalize Windows-style paths to /drive/path form.
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"
        REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

ACTIVE_FILE="$PROJECT_ROOT/.bb2_active_func"

# Fast path: no active marker -> allow.
if [ ! -s "$ACTIVE_FILE" ]; then
    exit 0
fi
ACTIVE=$(tr -d '[:space:]' < "$ACTIVE_FILE")
if [ -z "$ACTIVE" ]; then
    exit 0
fi

# Bypass via env override.
if [ "${BB2_SCOPE_OVERRIDE:-}" = "1" ]; then
    exit 0
fi

# Extract tool fields.
TOOL_NAME=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_name',''))" 2>/dev/null || echo "")
case "$TOOL_NAME" in
    Edit|Write|MultiEdit) ;;
    *) exit 0 ;;
esac

FILE_PATH=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('file_path',''))" 2>/dev/null || echo "")
case "$FILE_PATH" in
    *regfix.txt|*regfix_stage2.txt|*asmfix.txt) ;;
    *) exit 0 ;;
esac

# Helper: scan a block of text for rule lines (pattern `^<func>: ...`) and
# print the unique function names found. Skips comment-only lines.
# Note: `python3 -c` (not heredoc) so stdin from the pipe reaches the script.
extract_funcs() {
    python3 -c "
import re, sys
funcs = set()
for line in sys.stdin.read().splitlines():
    m = re.match(r'^\s*([A-Za-z_]\w+)\s*:\s+\w', line)
    if m:
        funcs.add(m.group(1))
print('\n'.join(sorted(funcs)))
"
}

# Read tool-specific content fields.
if [ "$TOOL_NAME" = "Edit" ] || [ "$TOOL_NAME" = "MultiEdit" ]; then
    OLD_STRING=$(echo "$INPUT" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ti = d.get('tool_input',{})
if 'edits' in ti:
    parts = []
    for e in ti['edits']:
        parts.append(e.get('old_string',''))
    print('\n'.join(parts))
else:
    print(ti.get('old_string',''))
" 2>/dev/null || echo "")
    NEW_STRING=$(echo "$INPUT" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ti = d.get('tool_input',{})
if 'edits' in ti:
    parts = []
    for e in ti['edits']:
        parts.append(e.get('new_string',''))
    print('\n'.join(parts))
else:
    print(ti.get('new_string',''))
" 2>/dev/null || echo "")

    # Funcs touched by old_string but no longer in new_string => removed.
    OFFENDERS=$(OLD="$OLD_STRING" NEW="$NEW_STRING" ACTIVE="$ACTIVE" python3 -c "
import os, re
def funcs(t):
    out = set()
    for line in t.splitlines():
        m = re.match(r'^\s*([A-Za-z_]\w+)\s*:\s+\w', line)
        if m: out.add(m.group(1))
    return out
old = funcs(os.environ.get('OLD',''))
new = funcs(os.environ.get('NEW',''))
removed = old - new
active = os.environ.get('ACTIVE','')
offenders = sorted(f for f in removed if f != active)
print(' '.join(offenders))
")
    if [ -n "$OFFENDERS" ]; then
        cat >&2 <<EOF
BLOCKED: this Edit removes regfix/asmfix rules for non-active function(s):
  $OFFENDERS

Active marker: $ACTIVE  (file: $ACTIVE_FILE)
Target file:   $FILE_PATH

Rule edits across functions during active work caused the func_8007D3F8 disaster
(79 rules across 3 sibling functions silently dropped). Tools refuse this by default.

Valid escape hatches:
  1. Confine your edit to rules for $ACTIVE only.
  2. If user authorized a cross-function fix (e.g., post-match label drift on a
     sibling), set BB2_SCOPE_OVERRIDE=1 in env for that one Bash command. Note
     this hook can't directly read your env — set it via:
       wsl bash -c 'BB2_SCOPE_OVERRIDE=1 <command>'
     when invoking through Bash, or instruct the user to clear the active
     marker first.
  3. Finish $ACTIVE and the marker auto-clears on commit.
EOF
        exit 2
    fi
fi

if [ "$TOOL_NAME" = "Write" ]; then
    NEW_CONTENT=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('content',''))" 2>/dev/null || echo "")
    if [ ! -f "$FILE_PATH" ]; then
        # Creating a new file — nothing to remove.
        exit 0
    fi
    OLD_CONTENT=$(cat "$FILE_PATH")
    OFFENDERS=$(OLD="$OLD_CONTENT" NEW="$NEW_CONTENT" ACTIVE="$ACTIVE" python3 -c "
import os, re
def funcs(t):
    out = set()
    for line in t.splitlines():
        m = re.match(r'^\s*([A-Za-z_]\w+)\s*:\s+\w', line)
        if m: out.add(m.group(1))
    return out
old = funcs(os.environ.get('OLD',''))
new = funcs(os.environ.get('NEW',''))
removed = old - new
active = os.environ.get('ACTIVE','')
offenders = sorted(f for f in removed if f != active)
print(' '.join(offenders))
")
    if [ -n "$OFFENDERS" ]; then
        cat >&2 <<EOF
BLOCKED: this Write would remove regfix/asmfix rules for non-active function(s):
  $OFFENDERS

Active marker: $ACTIVE  (file: $ACTIVE_FILE)
Target file:   $FILE_PATH

This is the failure mode that subagent #1 fell into on func_8007D3F8.
Either narrow your Write to add-only changes for $ACTIVE, or finish that
function first and let the marker clear.
EOF
        exit 2
    fi
fi

exit 0
