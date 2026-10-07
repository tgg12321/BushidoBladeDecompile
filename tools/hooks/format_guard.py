#!/usr/bin/env python3
"""format_guard -- git commit-msg hook.

Blocks a commit whose staged src/**/*.c|h or include/**/*.h is not in the repo's
C style (.clang-format, applied by tools/format.py). Formatting is
token-preserving, so fixing it never changes the build or a layer-2 key:

    python tools/format.py <files>        (Windows or WSL)

Only runs when such a file is staged. Escape hatch: `[skip-format]` in the
commit message, with a one-line reason.
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main() -> int:
    msg = Path(sys.argv[1]).read_text(encoding="utf-8", errors="replace") if len(sys.argv) > 1 else ""
    msg = msg.split("# ------------------------ >8 ------------------------")[0]  # `commit -v` diff
    if "[skip-format]" in msg:
        return 0
    staged = subprocess.run(["git", "diff", "--cached", "--name-only", "--diff-filter=ACMR", "-z"],
                            cwd=ROOT, capture_output=True).stdout.decode("utf-8", "replace").split("\0")
    if not any(p.endswith((".c", ".h")) and p.startswith(("src/", "include/")) for p in staged):
        return 0
    r = subprocess.run([sys.executable, str(ROOT / "tools" / "format.py"), "--check-staged"],
                       cwd=ROOT, capture_output=True, text=True)
    if r.returncode == 0:
        return 0
    sys.stderr.write("format_guard: BLOCKED (C style, .clang-format)\n" + r.stdout + r.stderr +
                     "Fix: python tools/format.py <files>, re-stage, commit again "
                     "(token-preserving: bytes and layer-2 keys are unchanged).\n")
    return 1


if __name__ == "__main__":
    sys.exit(main())
