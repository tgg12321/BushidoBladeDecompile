#!/usr/bin/env python3
"""Allow/block coverage for archive_read_guard._path_in_archive.

Run: python3 tools/hooks/test_archive_read_guard.py   (exit 0 = pass)
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from archive_read_guard import _path_in_archive as f

CASES = [
    # (text, is_command, expected_block)
    ("archive/old.py", False, True),
    ("C:\\repo\\archive\\x.md", False, True),
    ("cat archive/dc.sh", True, True),
    ("ls archive\\scripts", True, True),
    ("rg foo archive/ -g '!archive/x'", True, True),
    ("cat <<'EOF' > tmp/x\nhi\nEOF\ncat archive/dc.sh", True, True),   # read AFTER the heredoc
    # false blocks measured 2026-09-22
    ("grep -rn foo . | grep -v \"^./tmp\\|^./archive\\|^./memory\"", True, False),
    ("grep -rn foo --exclude-dir=archive .", True, False),
    ("rg foo -g '!archive/**'", True, False),
    ("cat > tmp/msg.txt <<'EOF'\nblocks `cat archive/...` reads\nEOF\ngit commit -F tmp/msg.txt", True, False),
    ("git log -- src/archive_notes.c", True, False),
]


def main() -> int:
    bad = 0
    for text, cmd, want in CASES:
        got = f(text, cmd)
        if got != want:
            bad += 1
            print(f"FAIL block={got} want={want}: {text!r}")
    print(f"{len(CASES)} cases; {bad} failures")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
