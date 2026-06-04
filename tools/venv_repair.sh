#!/bin/bash
# venv_repair.sh — restore .venv/bin/python3 symlink if missing.
#
# Idempotent. Silent on success when nothing needs repair. Called automatically
# by tools/wsl.sh on every invocation as a self-healing measure for the venv-
# python-symlink-missing failure mode (see
# tools/hooks/tooling_error_signatures.json id `venv-python-symlink-missing`).
#
# Run directly from WSL bash with cwd = project root.
set -u

# Find project root if not already there.
if [ ! -e ".venv/bin/activate" ]; then
    # cwd doesn't have .venv; nothing to do
    exit 0
fi
if [ -e ".venv/bin/python3" ]; then
    # Already healthy
    exit 0
fi
echo "wsl.sh: auto-repair: restoring .venv/bin/python3 symlink" >&2
for cand in /usr/bin/python3.12 /usr/bin/python3.11 /usr/bin/python3.10 /usr/bin/python3; do
    if [ -e "$cand" ]; then
        ln -sf "$cand" ".venv/bin/python3"
        ln -sf python3 ".venv/bin/python"
        ln -sf python3 ".venv/bin/python3.12"
        exit 0
    fi
done
echo "wsl.sh: auto-repair: no system python3 found; pip will still fail" >&2
exit 1
