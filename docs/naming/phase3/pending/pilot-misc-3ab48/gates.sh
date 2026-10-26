#!/bin/bash
# wave91 gates: usage  gates.sh p3w91 --from-manifests <fn.json> [--base <sha> --new <sha>]
# Copy of the wave89 gates with the worktree of wave91; the extra args go to naming_keycheck.
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-c
. .venv/bin/activate
TAG="$1"; shift
echo "== integrity"; python3 tools/check_completion_integrity.py 2>&1 | tail -8; echo "rc=${PIPESTATUS[0]}"
echo "== keycheck"; python3 tools/naming_keycheck.py "$@" 2>&1 | tail -25; echo "rc=${PIPESTATUS[0]}"
echo "== label census"; python3 tools/test_label_census.py 2>&1 | tail -8; echo "rc=${PIPESTATUS[0]}"
