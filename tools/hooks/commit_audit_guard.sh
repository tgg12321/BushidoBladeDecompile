#!/bin/bash
# PreToolUse hook for Bash. Enforces the LLM auditor on git commits.
#
# Scope (mirrors active_func_guard.sh):
#   - Only fires on Bash tool calls that contain `git commit ...`
#   - Worktree-aware: if the command does `cd <other-worktree> && git commit`,
#     audit runs against that worktree's staged diff, not main's.
#   - Non-decomp commits skip via the auditor's own ALLOWLIST_RE in
#     llm_audit.sh (allowlists *.md, tools/*, data_evidence/*, kengo/*,
#     .claude/*, .gitignore). The hook does NOT add its own file filter;
#     it trusts the auditor's prefilter. So an agent committing only
#     docs/tooling pays zero LLM cost.
#   - REJECT verdicts block the commit with the auditor's reasoning.
#   - SKIP_PREFILTER / SKIP_ALLOWLIST / APPROVE pass through.
#   - ERROR / network failures pass through with a stderr warning (we
#     don't fail-closed at the hook layer; the user can always re-run
#     /cheat-audit manually if connectivity issues prevent on-commit
#     audits).
#
# Bypass options (use sparingly):
#   - `git commit --no-verify ...`     (standard git bypass)
#   - `BB2_AUDIT_SKIP=1 git commit ...` (env-var bypass)
#
# Exit codes:
#   0 — allow the bash call to proceed
#   2 — block; stderr is shown to Claude as the block reason

set -e

# ---- Resolve project root (same scheme as active_func_guard.sh) ----
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

# ---- Read hook input ----
INPUT=$(cat)
COMMAND=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('command',''))" 2>/dev/null || echo "")

if [ -z "$COMMAND" ]; then
    exit 0
fi

# ---- Filter to git commit only ----
# Match `git commit` as a whole word (avoids git commit-tree, etc.)
if ! echo "$COMMAND" | grep -qE '\bgit\s+commit(\s|$|"|'"'"')'; then
    exit 0
fi
# Explicitly exclude plumbing variants
if echo "$COMMAND" | grep -qE '\bgit\s+commit-tree'; then
    exit 0
fi

# ---- Bypass paths ----
# Standard --no-verify
if echo "$COMMAND" | grep -qE '(^|[[:space:]])--no-verify([[:space:]]|$)'; then
    exit 0
fi
# Env-var bypass
if echo "$COMMAND" | grep -qE '(^|[[:space:]])BB2_AUDIT_SKIP=1\b'; then
    exit 0
fi

# ---- Worktree-aware audit target (same parser as active_func_guard) ----
parse_cd_target() {
    local cmd="$1" rest target
    case "$cmd" in
        "cd "*) rest="${cmd#cd }" ;;
        *)      printf '' ; return ;;
    esac
    rest="${rest#"${rest%%[![:space:]]*}"}"
    if [ "${rest#\"}" != "$rest" ]; then
        target="${rest#\"}"
        target="${target%%\"*}"
    elif [ "${rest#\'}" != "$rest" ]; then
        target="${rest#\'}"
        target="${target%%\'*}"
    else
        target="${rest%% &&*}"
        target="${target%%;*}"
    fi
    printf '%s' "$target"
}

AUDIT_DIR="$PROJECT_ROOT"
TARGET_PATH="$(parse_cd_target "$COMMAND")"
if [ -n "$TARGET_PATH" ] && [ -d "$TARGET_PATH" ]; then
    TARGET_ROOT=$(git -C "$TARGET_PATH" rev-parse --show-toplevel 2>/dev/null || true)
    case "$TARGET_ROOT" in
        [A-Za-z]:[/\\]*)
            DRIVE=$(echo "$TARGET_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
            REST="${TARGET_ROOT#?:}"
            REST="${REST//\\//}"
            TARGET_ROOT="/$DRIVE$REST"
            ;;
    esac
    if [ -n "$TARGET_ROOT" ]; then
        AUDIT_DIR="$TARGET_ROOT"
    fi
fi

# ---- Rule 1a: programmatic asm-cheat gate ----
# Relocated 2026-05-22 from the deprecated active_func_guard.sh. Catches NEW
# mechanical cheats since HEAD: large regfix splices, file-scope
# __asm__("glabel ...") bodies, lost-codegen single-insn inserts, same-commit
# self-authorization, and missing evidence tags / Pure-C attempts: block /
# aliasing-barrier comments. Cheap (~1s); runs BEFORE the expensive LLM auditor
# (which assumes this formal gate already ran). audit_asm_cheats.py is
# pure-stdlib Python, so it runs directly in the hook env (no WSL/venv).
if [ -f "$AUDIT_DIR/tools/audit_asm_cheats.py" ]; then
    # Extract -m/--message so the gate can validate the "Pure-C attempts:" block.
    COMMIT_MSG_FROM_CMD=$(python3 -c "
import shlex, sys
try:
    args = shlex.split(sys.argv[1])
except Exception:
    sys.exit(0)
in_commit = False
for i, a in enumerate(args):
    if a == 'git':
        if i + 1 < len(args) and args[i+1] == 'commit':
            in_commit = True
        continue
    if in_commit:
        if a in ('-m', '--message') and i + 1 < len(args):
            print(args[i+1]); break
        if a.startswith('--message='):
            print(a[len('--message='):]); break
" "$COMMAND" 2>/dev/null || true)
    CHEAT_MSG_FILE=$(mktemp -t bb2-commitmsg.XXXXXX)
    printf '%s' "$COMMIT_MSG_FROM_CMD" > "$CHEAT_MSG_FILE"
    if cheat_out=$(cd "$AUDIT_DIR" && python3 tools/audit_asm_cheats.py --check-new --commit-msg-file "$CHEAT_MSG_FILE" 2>&1); then
        rm -f "$CHEAT_MSG_FILE"
    else
        rm -f "$CHEAT_MSG_FILE"
        echo "$cheat_out" >&2
        cat >&2 <<'EOF'

BLOCKED: see ASM-CHEAT GUARD message above.

This enforces the project's SOTN-grade policy: the default standard is pure C;
inline asm / canonical authorization / regfix are deviations requiring
compelling, evidence-driven justification. (Programmatic gate relocated from
active_func_guard.sh, deprecated 2026-05-22.)

To proceed: write a real pure-C decomp instead of wrapping asm, OR add the
required evidence tag / "Pure-C attempts:" block / INLINE_MOVE_ALIASING: comment.
See memory rules evidence-driven-authorization and lost-codegen-insert-cheat.

Bypass (use sparingly):  git commit --no-verify ...   |   BB2_AUDIT_SKIP=1 git commit ...
EOF
        exit 2
    fi
fi

# ---- Rule 1b: LLM auditor on the staged diff in the target worktree ----
if [ ! -f "$AUDIT_DIR/tools/audit_cli.sh" ]; then
    # No auditor available here; allow (don't block on missing tooling)
    exit 0
fi

# Capture audit output. Don't propagate audit_cli.sh exit code into our
# decision -- we parse the VERDICT string explicitly.
AUDIT_OUT=$(cd "$AUDIT_DIR" && bash tools/audit_cli.sh staged 2>&1 || true)
VERDICT=$(echo "$AUDIT_OUT" | grep -oE 'VERDICT: (APPROVE|REJECT|SKIP_[A-Z_]+|ERROR)' | head -1 | awk '{print $NF}')

case "$VERDICT" in
    APPROVE|SKIP_PREFILTER|SKIP_ALLOWLIST|"")
        # SKIP_* and empty (which can mean the auditor itself printed
        # SKIP early without a VERDICT: line) pass through.
        exit 0
        ;;
    REJECT)
        # Echo the auditor's reasoning + violations to stderr so Claude
        # sees them as the block reason.
        echo "" >&2
        echo "═══ LLM AUDITOR REJECTED THIS COMMIT ═══" >&2
        echo "$AUDIT_OUT" | sed -n '/REASONING\|VIOLATIONS\|VERDICT/,$p' >&2
        echo "" >&2
        echo "Bypass options (use sparingly):" >&2
        echo "  git commit --no-verify ...      (standard git)" >&2
        echo "  BB2_AUDIT_SKIP=1 git commit ... (env-var)" >&2
        exit 2
        ;;
    ERROR|*)
        # Don't fail-closed on auditor errors. Surface a warning so the
        # user knows the audit didn't run, but don't block useful work.
        echo "[commit_audit_guard] auditor reported $VERDICT; allowing commit." >&2
        echo "[commit_audit_guard] Re-run manually if needed: /cheat-audit on the commit." >&2
        exit 0
        ;;
esac
