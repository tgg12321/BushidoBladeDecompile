#!/usr/bin/env bash
# commit-msg hook chain runner (canonical source — copy to .git/hooks/commit-msg).
#
# Runs each project commit-msg guard in sequence; first non-zero exit
# aborts the commit. All guards receive the same arg: the path to the
# pending commit message file (passed by git as $1).
#
# Hooks live in tools/hooks/ and are pure Python (stdlib only).
# Install:
#   cp tools/hooks/commit_msg_chain.sh .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg
#
# To add a guard: append another `python3 ... "$@" || exit 1` line here,
# then re-install.

set -u

REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
cd "$REPO_ROOT"

python3 tools/hooks/park_src_guard.py "$@" || exit 1
python3 tools/hooks/no_new_regfix_guard.py "$@" || exit 1
python3 tools/hooks/wip_compaction_guard.py "$@" || exit 1

exit 0
