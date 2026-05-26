#!/usr/bin/env python3
"""Allow/block coverage for shell_footgun_guard.reasons_for.

Run: python3 tools/hooks/test_shell_footgun_guard.py   (exit 0 = pass)
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import shell_footgun_guard as g

# Commands that must pass through untouched.
ALLOW = [
    "wsl bash -c 'git status'",
    "wsl bash -c 'make 2>&1 | tail -5'",
    "& tools/eng.ps1 queue next",
    "& tools/eng.ps1 sandbox func_X --disable all",
    "wsl bash -c 'cd x && git commit -F tmp/msg.txt'",
    "wsl bash -c 'grep -n foo bar.c'",
    "git status",
    "wsl bash -c 'python3 tmp/norm_diff.py a b'",   # running a script FILE is fine
]

# Commands that must be blocked (genuine footguns).
BLOCK = [
    "git commit -F - <<'\"'\"'EOF'\"'\"'\nmsg\nEOF",          # nested quote escape
    "wsl bash -c 'cd x && source .venv/bin/activate && python3 -m engine.cli queue next'",  # engine direct
    "wsl bash -c 'cat <<EOF\nhi\nEOF'",                       # heredoc in wsl
    "wsl bash -c 'norm() { echo hi; }; norm'",                # funcdef in wsl
    'wsl bash -c \'awk "{print \\$3}" f.txt\'',               # awk touching $
    "wsl bash -c 'python3 x.py; echo \"rc=$?\"'",             # unreliable $? capture in wsl
]


def main() -> int:
    fails = 0
    for c in ALLOW:
        r = g.reasons_for(c)
        if r:
            fails += 1
            print(f"FAIL (should ALLOW): {c!r}\n   -> {r}")
    for c in BLOCK:
        if not g.reasons_for(c):
            fails += 1
            print(f"FAIL (should BLOCK): {c!r}")
    print(f"{len(ALLOW)} allow + {len(BLOCK)} block cases; {fails} failures")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
