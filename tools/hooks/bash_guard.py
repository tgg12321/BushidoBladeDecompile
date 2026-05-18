#!/usr/bin/env python3
"""PreToolUse hook on Bash. Three soft warnings (never blocks):

1. **Wrong-tool selection** — standalone `cat <file>` / `head <file>` /
   `tail <file>` / `grep <pattern> <file>` / `find <dir> -name ...`
   that has a Read/Grep/Glob equivalent. Suggests the equivalent.

2. **WSL boilerplate** — `wsl bash -c '... cd /mnt/c/.../"Bushido Blade 2
   Decompile" && ...'` patterns. Suggests `bash tools/wsl.sh '<cmd>'` as
   the half-character-count replacement.

3. **Sequential Bash chaining missed** — 3+ Bash calls within ~30
   seconds. Stateful: tracks recent invocations in tmp/recent_bash.log.
   Suggests `&&` chaining or batching.

All exits are 0 (warnings only, never block). Warnings go to stderr.
"""
from __future__ import annotations

import json
import re
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
RECENT_LOG = REPO_ROOT / "tmp" / "recent_bash.log"
RECENT_WINDOW_SEC = 30        # how far back "recent" means
RECENT_TRIGGER_COUNT = 3       # 3+ calls in window → warn

# Pattern 1 — wrong-tool detectors
# Each detector matches a Bash command line and returns a hint.

def _detect_wrong_tool(cmd: str) -> list[str]:
    """Return list of warnings if Bash is being used where a dedicated tool fits."""
    warnings: list[str] = []

    # Strip leading whitespace; if command has a pipe or && or ; we generally allow
    # (it's part of a larger composition where Bash makes sense)
    stripped = cmd.strip()
    has_pipe = "|" in stripped and "||" not in stripped.replace("||", "")  # rough
    has_chain = " && " in stripped or " || " in stripped or ";" in stripped
    if has_pipe or has_chain:
        return warnings  # complex composition; assume intentional

    # Standalone `cat <file>` / `head ... <file>` / `tail ... <file>`
    m = re.match(r"^(cat|head|tail)\s+(?:-\w+\s+\d+\s+)?[\"']?([^\"'\s|<>&;]+\.\w+)[\"']?\s*$", stripped)
    if m:
        verb, path = m.group(1), m.group(2)
        warnings.append(
            f"Bash `{verb} {path}` — use the Read tool instead "
            f"(supports `offset`+`limit` for large files; no shell encoding issues)."
        )

    # `grep <pattern> <file>` (no -r, no pipe)
    m = re.match(r"^grep\s+(?!-r|-R|-l|-L)\S+\s+[\"']?([^\"'\s|<>&;]+)[\"']?\s*$", stripped)
    if m:
        path = m.group(1)
        warnings.append(
            f"Bash `grep ... {path}` — use the Grep tool instead "
            f"(faster, supports content/files_with_matches/count output modes)."
        )

    # `grep -r/-R <pattern> <dir>` → Grep tool
    if re.match(r"^grep\s+-[rR]\b", stripped):
        warnings.append(
            "Bash `grep -r ...` — use the Grep tool instead "
            "(built on ripgrep; faster + better defaults)."
        )

    # `find <dir> -name '*.x'` → Glob tool
    if re.match(r"^find\s+\S+\s+-name\s+", stripped):
        warnings.append(
            "Bash `find ... -name ...` — use the Glob tool instead "
            "(use a glob pattern like `**/*.c`)."
        )

    return warnings


# Pattern 2 — WSL boilerplate detector

WSL_BOILERPLATE_RE = re.compile(
    r"wsl\s+bash\s+-c\s+['\"].*cd\s+[\"']?/mnt/c/Users/Trenton/Desktop/[\"']?Bushido Blade 2 Decompile",
    re.IGNORECASE,
)

def _detect_wsl_boilerplate(cmd: str) -> list[str]:
    if WSL_BOILERPLATE_RE.search(cmd):
        return [
            "Detected `wsl bash -c 'cd /mnt/c/.../\"Bushido Blade 2 Decompile\" && ...'` boilerplate. "
            "Use `bash tools/wsl.sh '<command>'` instead — half the characters, sets cwd for you."
        ]
    return []


# Pattern 3 — sequential Bash detector (stateful)

def _track_and_check_sequential(cmd: str) -> list[str]:
    """Append current Bash to recent log; warn if 3+ in RECENT_WINDOW_SEC."""
    now = time.time()
    RECENT_LOG.parent.mkdir(parents=True, exist_ok=True)

    # Read existing entries (each: "<unix_ts>\t<first 80 chars of cmd>")
    entries: list[tuple[float, str]] = []
    if RECENT_LOG.exists():
        try:
            for line in RECENT_LOG.read_text(encoding="utf-8", errors="replace").splitlines():
                ts_str, _, cmd_snip = line.partition("\t")
                try:
                    ts = float(ts_str)
                except ValueError:
                    continue
                if now - ts <= RECENT_WINDOW_SEC * 4:  # keep 4× window for context
                    entries.append((ts, cmd_snip))
        except Exception:
            pass

    # Append current
    cmd_snip = cmd.replace("\n", " ")[:80]
    entries.append((now, cmd_snip))

    # Write back trimmed log (keep last 50 entries to bound file size)
    try:
        trimmed = entries[-50:]
        RECENT_LOG.write_text(
            "\n".join(f"{ts:.3f}\t{c}" for ts, c in trimmed),
            encoding="utf-8",
            newline="\n",
        )
    except Exception:
        pass

    # Count entries within RECENT_WINDOW_SEC
    recent = [c for ts, c in entries if now - ts <= RECENT_WINDOW_SEC]
    if len(recent) >= RECENT_TRIGGER_COUNT:
        recent_preview = "\n".join(f"    - {c[:60]}" for c in recent[-RECENT_TRIGGER_COUNT:])
        return [
            f"Detected {len(recent)} Bash calls in the last {RECENT_WINDOW_SEC}s — "
            f"consider combining with `&&` or batching into one call.\n"
            f"  Recent commands:\n{recent_preview}"
        ]
    return []


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0

    if payload.get("tool_name") != "Bash":
        return 0

    cmd = payload.get("tool_input", {}).get("command", "")
    if not cmd:
        return 0

    warnings: list[str] = []
    warnings.extend(_detect_wrong_tool(cmd))
    warnings.extend(_detect_wsl_boilerplate(cmd))
    warnings.extend(_track_and_check_sequential(cmd))

    if warnings:
        print("HINT from bash_guard.py:", file=sys.stderr)
        for w in warnings:
            print(f"  - {w}", file=sys.stderr)
        print("\n(Warning only — command proceeds. Re-read this hint when planning the next call.)",
              file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
