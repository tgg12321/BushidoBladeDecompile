#!/usr/bin/env python3
"""PreToolUse hook on Bash: BLOCK execution of cc1psx / dosemu / the cc1psx
diagnostic harness.

cc1psx (Sony's original PsyQ compiler) is not a debugging tool here. The
"maybe it's a compiler difference" hypothesis is settled and re-proven every
session (docs/diagnostics/compiler_parity.txt):
decompals-gcc-2.7.2 reproduces the original game byte-for-byte; cc1psx is never
closer to the original (0 genuine compiler-fixable) and is sometimes worse
(e.g. ang_near_dif). The compiler is never the variable — the gap is always the
C source. See memory/rules/cc1psx-calibration-only.md, compiler-patch-low-roi.md.

This stops an agent the moment it reaches for cc1psx — a common disguised
give-up ("let me check whether this is a compiler divergence"). Reading/grepping
cc1psx files (cat/grep/ls/...) is allowed; only EXECUTION is blocked.

Hook protocol: JSON on stdin; exit 0 = allow, exit 2 = block (stderr -> Claude).
"""
from __future__ import annotations

import json
import re
import sys

# Substrings that indicate a cc1psx-family tool is referenced.
BLOCKED_TOKENS = (
    "cc1psx_wrapper.sh",
    "cc1psx.exe",
    "cc1psx_diagnostic.py",
    "diagnose_batch.sh",
    "dosemu",  # the only thing dosemu runs here is cc1psx.exe
)

# If the command's first real word is one of these, it's an inspection of the
# file (read), not an execution — allow it.
READ_FIRST_WORDS = {
    "cat", "grep", "rg", "egrep", "fgrep", "ls", "head", "tail", "wc", "stat",
    "file", "less", "more", "find", "diff", "cmp", "xxd", "od", "strings",
    "sha1sum", "md5sum", "echo", "printf", "readlink", "realpath", "dirname",
    "basename", "test", "[",
}


def _basename(tok: str) -> str:
    tok = tok.strip().strip('"').strip("'").replace("\\", "/")
    return tok.rsplit("/", 1)[-1].lower()


def _first_real_word(cmd: str) -> str:
    """First token, skipping leading env-var assignments (FOO=bar)."""
    for tok in cmd.split():
        if re.match(r"^[A-Za-z_][A-Za-z0-9_]*=", tok):
            continue
        return _basename(tok)
    return ""


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0
    if payload.get("tool_name") != "Bash":
        return 0
    cmd = payload.get("tool_input", {}).get("command", "") or ""
    if not cmd:
        return 0

    low = cmd.lower()
    hit = next((t for t in BLOCKED_TOKENS if t in low), None)
    if not hit:
        return 0

    # Allow direct file inspection (the agent is reading, not running).
    if _first_real_word(cmd) in READ_FIRST_WORDS:
        return 0

    sys.stderr.write(
        f"""BLOCKED by cc1psx_guard.py: this command runs `{hit}` (cc1psx / dosemu).

Reaching for cc1psx is a disguised give-up. The compiler-choice question is
SETTLED and the project proves it empirically — see the compiler-parity
record in docs/diagnostics/compiler_parity.txt:

  - decompals-gcc-2.7.2 reproduces the ORIGINAL GAME byte-for-byte (build SHA1 OK).
  - cc1psx is NEVER closer to the original than decompals (0 genuine
    compiler-fixable), and is SOMETIMES WORSE — e.g. `ang_near_dif`: cc1psx
    schedules the block differently and does NOT match the original; decompals does.
  - The one function where cc1psx looked "better" (`func_80016A8C`) is just the
    decompals big-endian `lwl/lwr` offset quirk, which the build already corrects
    deterministically (fix_lwl / asmfix offset-swaps) — NOT a real cc1psx win.

So switching compilers can NEVER turn a non-match into a match (and would BREAK
functions decompals matches). A function that won't match is a C-SOURCE problem
(typing, dataflow, scheduling, register allocation, cross-jump merging) — never
the compiler / optimizer / fork.

See:  docs/diagnostics/compiler_parity.txt
      memory/rules/cc1psx-calibration-only.md, compiler-patch-low-roi.md
      .claude/rules/compiler-flags-canonical.md

WHAT TO DO INSTEAD: switch technique within pure C — reread the target's
dataflow for the value's real provenance/typing, grep an already-matched
sibling for the codegen shape, restart from m2c, run the permuter on the
diverging region. The matching C exists; keep finding it.

(To inspect the cc1psx files without running them, use the Read/Grep tools or a
`cat`/`grep` command.)
"""
    )
    return 2


if __name__ == "__main__":
    sys.exit(main())
