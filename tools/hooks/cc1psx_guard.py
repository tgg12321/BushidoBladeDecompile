#!/usr/bin/env python3
"""PreToolUse hook on Bash: BLOCK execution of cc1psx / dosemu / the cc1psx
diagnostic harness.

cc1psx (Sony's original PsyQ compiler) is DEPRECATED for this project
(2026-05-18). The "maybe it's a compiler difference" hypothesis was settled
empirically: 8/8 sampled functions are byte-identical between the KMC- and
PSX-tailored gcc-2.7.2 forks, and 0/16 tier-3 functions are fixable by cc1psx.
The compiler is never the variable — the gap is always the C source. See
memory/rules/cc1psx-calibration-only.md and memory/rules/compiler-patch-low-roi.md.

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

cc1psx is DEPRECATED for this project and must not be run.

WHY: "maybe it's a compiler difference" is a settled question. Measured
2026-05-18:
  - 8/8 sampled functions are BYTE-IDENTICAL between the KMC- and PSX-tailored
    gcc-2.7.2 forks.
  - 0/16 tier-3 functions are fixable by cc1psx (Sony's original compiler).
THE COMPILER IS NEVER THE VARIABLE. When a function won't match in pure C, the
gap is ALWAYS the C source structure (typing, dataflow, scheduling, register
allocation, cross-jump merging) — not the compiler / optimizer / fork.

See:  memory/rules/cc1psx-calibration-only.md
      memory/rules/compiler-patch-low-roi.md
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
