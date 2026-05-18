#!/bin/bash
# tools/hooks/memory_precommit_audit.sh
#
# Git pre-commit hook. Backstop validation for the memory system —
# runs the indexer + audit and fails the commit if they detect:
# - Broken [[link]] references across memory/ + .claude/rules/
# - MEMORY.md would exceed the 200-line / 24KB Claude Code auto-load limit
# - Frontmatter parse errors in any memory file
#
# Only runs if the commit touches memory/, .claude/rules/, CLAUDE.md,
# AGENTS.md, or the indexer/audit tools themselves. Otherwise pass.
#
# Install via:
#   ln -sf "$(pwd)/tools/hooks/memory_precommit_audit.sh" .git/hooks/pre-commit
# or (Windows-safe):
#   cp tools/hooks/memory_precommit_audit.sh .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit

set -u

# Check if any staged file is in scope for this hook
STAGED=$(git diff --cached --name-only 2>/dev/null)
TRIGGER=0
while IFS= read -r f; do
    case "$f" in
        .claude/rules/*|CLAUDE.md|AGENTS.md|tools/regen_memory_index.py|tools/memory_audit.py)
            TRIGGER=1
            break
            ;;
    esac
done <<< "$STAGED"

# Also trigger if any memory/ file was modified (they live outside the repo,
# but the indexer may have updated MEMORY.md anyway — re-run to be safe)
if [ "$TRIGGER" -eq 0 ]; then
    # Quick: if no staged files match and no memory tool changed, skip
    exit 0
fi

REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
cd "$REPO_ROOT"

FAILED=0

echo "[memory_precommit_audit] running indexer (validates frontmatter + links + size)..."
INDEXER_OUT=$(python3 tools/regen_memory_index.py 2>&1)
INDEXER_EXIT=$?
if [ $INDEXER_EXIT -ne 0 ]; then
    echo "[memory_precommit_audit] FAIL: regen_memory_index.py exited $INDEXER_EXIT"
    echo "$INDEXER_OUT" | sed 's/^/  /'
    FAILED=1
fi

echo "[memory_precommit_audit] running audit (broken links, orphans)..."
AUDIT_OUT=$(python3 tools/memory_audit.py 2>&1)
# audit script writes to tmp/memory_audit.md; check broken-links line in its stdout
BROKEN=$(echo "$AUDIT_OUT" | grep -oE 'Broken links: [0-9]+' | head -1 | grep -oE '[0-9]+$')
if [ -n "$BROKEN" ] && [ "$BROKEN" != "0" ]; then
    echo "[memory_precommit_audit] FAIL: $BROKEN broken [[link]] references — see tmp/memory_audit.md"
    FAILED=1
fi

# CLAUDE.md size check (soft target — warn but don't block above ~250)
if [ -f CLAUDE.md ]; then
    LINES=$(wc -l < CLAUDE.md)
    if [ "$LINES" -gt 250 ]; then
        echo "[memory_precommit_audit] WARN: CLAUDE.md is $LINES lines (target <=200). Consider moving content to memory/rules/ or AGENTS.md."
    fi
fi

# AGENTS.md import check
if [ -f CLAUDE.md ] && ! grep -q "@AGENTS.md" CLAUDE.md; then
    echo "[memory_precommit_audit] WARN: CLAUDE.md no longer imports @AGENTS.md — tool-agnostic facts won't load."
fi

if [ $FAILED -ne 0 ]; then
    echo ""
    echo "[memory_precommit_audit] Pre-commit checks failed. Fix the issues above and re-stage."
    echo "  Bypass (use sparingly): git commit --no-verify"
    exit 1
fi

exit 0
