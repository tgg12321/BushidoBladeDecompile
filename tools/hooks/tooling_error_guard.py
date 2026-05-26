#!/usr/bin/env python3
"""PostToolUse hook on Bash | PowerShell | Edit | Write | MultiEdit.

Enforces the `debugging-discipline` rule: *every tooling error happens once.*
When a command's output matches a KNOWN, permanently-fixable tooling/shell/
environment failure (CRLF line endings, WSL unavailable, broken worktree
symlinks / missing linked deps, core tool not on PATH, ...), or an Edit/Write
just wrote CRLF into a build-critical file, this hook:

  1. writes a per-worktree incident marker (.bb2_tooling_incident.json), and
  2. exits 2 with a directive on stderr telling the agent to STOP, root-cause
     it, and fix it PERMANENTLY (not paper over it and move on).

The companion Stop hook (tooling_incident_stop_guard.sh) then refuses to let
the turn end until the incident is resolved via `dc.sh fix-tooling-incident`.

Signatures live in tooling_error_signatures.json (next to this file) so the
catalog is extensible without touching code -- adding a signature is itself a
valid permanent fix.

DESIGN PRINCIPLES (mirrors grind_check.sh):
  * FAIL OPEN. Any internal error -> exit 0 (never break the agent's flow
    because the guard itself hiccuped).
  * HIGH PRECISION. Signatures fire on every Bash/PowerShell result, so a
    false positive that blocks legitimate work is worse than a miss. Only
    block on unambiguous infra-failure text.
  * Operator off-switch: a `.bb2_tooling_guard_off` file at the project root
    disables the guard entirely.

Exit codes:
  0 -- allow (no match, warn-only match, or off-switch present)
  2 -- block (a block-tier signature matched; directive on stderr)
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time
from fnmatch import fnmatch
from pathlib import Path

HOOK_DIR = Path(__file__).resolve().parent
SIGNATURES_PATH = HOOK_DIR / "tooling_error_signatures.json"
MARKER_NAME = ".bb2_tooling_incident.json"
OFF_SWITCH = ".bb2_tooling_guard_off"
MAX_SCAN_BYTES = 4 * 1024 * 1024  # cap output / file scan for performance
MAX_INCIDENTS_KEPT = 10           # bound the marker's occurrence history

# A signature substring/regex living in this project's own source (dc.sh's
# `echo "ERROR: ... worktree_bootstrap.sh missing"`, the signatures JSON, docs)
# appears verbatim whenever an agent greps/cats/reads that file -- which is NOT
# a tooling failure. These markers identify a line as DISPLAYED SOURCE/CONFIG
# rather than live command output; such lines are dropped before matching. Real
# failure text (e.g. a bare `cc1: not found` on stderr) carries none of them, so
# this kills self-inspection false positives without masking actual failures.
_DISPLAY_MARKERS = [
    re.compile(r"^\s*\d+[:\-]"),                       # grep -n / context prefix: "379:" / "382-"
    re.compile(r"\b(echo|printf)\b"),                  # shell echo/printf statement (source)
    re.compile(r">&\d"),                               # shell stderr/stdout redirect (source)
    re.compile(r"\bprint\(|\bsys\.std(err|out)\b"),    # python source line
    re.compile(r'^\s*"[^"]*",?\s*$'),                  # lone quoted string: JSON array element
    re.compile(r'"(summary|substrings|regexes|root_cause|permanent_fix|suggested_guard)"'),
]


def _is_display_line(line: str) -> bool:
    """True if a line is displayed source/config (a grep/cat/head of a script or
    the signatures JSON that *contains* a failure string) rather than live
    command output."""
    return any(rx.search(line) for rx in _DISPLAY_MARKERS)


def _real_output(text: str) -> str:
    """Drop displayed-source lines, keeping only plausible live command output."""
    return "\n".join(ln for ln in text.splitlines() if not _is_display_line(ln))


# --------------------------------------------------------------------------
# Pure, importable classification logic (exercised by the test suite).
# --------------------------------------------------------------------------

def load_signatures(path: Path = SIGNATURES_PATH) -> dict:
    """Load the signature library. Returns {} on any failure (fail open)."""
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {}


def classify_text(tool_name: str, text: str, signatures: dict) -> dict | None:
    """Return the matching signature dict for command OUTPUT, or None.

    Block-tier matches take priority over warn-tier. Pure function: no I/O,
    no side effects -- safe to call from tests.
    """
    if not text:
        return None
    scan = text[-MAX_SCAN_BYTES:] if len(text) > MAX_SCAN_BYTES else text
    scan = _real_output(scan)  # ignore displayed-source lines (self-inspection)
    low = scan.lower()

    block_hit = None
    warn_hit = None
    for sig in signatures.get("signatures", []):
        if sig.get("match", "").startswith("special:"):
            continue  # special detectors (e.g. build_file_crlf) handled elsewhere
        if tool_name not in sig.get("tools", []):
            continue
        matched = False
        for sub in sig.get("substrings", []):
            if sub and sub.lower() in low:
                matched = True
                break
        if not matched:
            for rx in sig.get("regexes", []):
                try:
                    if re.search(rx, scan, re.IGNORECASE):
                        matched = True
                        break
                except re.error:
                    continue
        if not matched:
            continue
        if sig.get("tier") == "block":
            return sig  # first block-tier match wins
        if warn_hit is None:
            warn_hit = sig
    return block_hit or warn_hit


def is_build_critical(file_path: str, signatures: dict) -> bool:
    """True if file_path must be LF (per the build_file config)."""
    cfg = signatures.get("build_file", {})
    p = Path(file_path)
    name = p.name
    if p.suffix.lower() in cfg.get("suffixes", []):
        return True
    if name in cfg.get("names", []):
        return True
    for g in cfg.get("name_globs", []):
        if fnmatch(name, g):
            return True
    return False


def file_has_crlf(file_path: str) -> bool:
    """True if the on-disk file contains a carriage return. False on any error."""
    try:
        with open(file_path, "rb") as f:
            data = f.read(MAX_SCAN_BYTES)
        return b"\r" in data
    except Exception:
        return False


def find_special_signature(signatures: dict, match_tag: str) -> dict | None:
    for sig in signatures.get("signatures", []):
        if sig.get("match") == match_tag:
            return sig
    return None


# --------------------------------------------------------------------------
# I/O helpers
# --------------------------------------------------------------------------

def project_root() -> Path:
    """Resolve the per-worktree project root (matches grind_check.sh).

    Priority: git toplevel from cwd -> CLAUDE_PROJECT_DIR -> this checkout.
    """
    try:
        out = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True, text=True, timeout=5,
        )
        if out.returncode == 0 and out.stdout.strip():
            return Path(out.stdout.strip())
    except Exception:
        pass
    env = os.environ.get("CLAUDE_PROJECT_DIR")
    if env and Path(env).is_dir():
        return Path(env)
    return HOOK_DIR.parent.parent


def extract_output(payload: dict) -> str:
    """Best-effort stringify of the tool result + command, robust to schema."""
    parts: list[str] = []
    resp = payload.get("tool_response")
    if isinstance(resp, str):
        parts.append(resp)
    elif isinstance(resp, dict):
        for key in ("stdout", "stderr", "output", "content", "error"):
            v = resp.get(key)
            if isinstance(v, str):
                parts.append(v)
        if not parts:  # unknown shape: dump the whole thing
            try:
                parts.append(json.dumps(resp))
            except Exception:
                pass
    elif resp is not None:
        parts.append(str(resp))
    return "\n".join(parts)


def record_incident(root: Path, incident: dict) -> None:
    """Write/append the incident marker. Best-effort; never raises."""
    marker = root / MARKER_NAME
    try:
        existing = {}
        if marker.exists():
            try:
                existing = json.loads(marker.read_text(encoding="utf-8"))
            except Exception:
                existing = {}
        incidents = existing.get("incidents", [])
        if not isinstance(incidents, list):
            incidents = []
        incidents.append(incident)
        data = {
            "status": "unresolved",
            "first_seen": existing.get("first_seen", incident["seen"]),
            "last_seen": incident["seen"],
            "count": len(incidents),
            "incidents": incidents[-MAX_INCIDENTS_KEPT:],
        }
        marker.write_text(json.dumps(data, indent=2), encoding="utf-8", newline="\n")
    except Exception:
        pass


def build_directive(sig: dict, command: str, snippet: str) -> str:
    fix_steps = "\n".join(f"  {i+1}. {s}" for i, s in enumerate(sig.get("permanent_fix", [])))
    cmd_line = (command or "").strip().replace("\n", " ")
    if len(cmd_line) > 200:
        cmd_line = cmd_line[:200] + " ..."
    snip = (snippet or "").strip()
    if len(snip) > 300:
        snip = snip[:300] + " ..."
    return f"""TOOLING-ERROR GUARD -- BLOCKED  (class: {sig.get('class')}, signature: {sig.get('id')})

You just hit a KNOWN, PERMANENTLY-FIXABLE tooling failure:
  {sig.get('summary')}
  Triggering command: {cmd_line}
  Matched output:     {snip}

Per the debugging-discipline rule, do NOT work around this or move on.
ROOT CAUSE: {sig.get('root_cause')}

Fix it PERMANENTLY so no future agent hits it:
{fix_steps}
  Suggested guard: {sig.get('suggested_guard')}

You cannot end this turn until the incident is resolved (the Stop hook will
block it). Once you have implemented a permanent fix -- a new/changed hook,
a .gitattributes entry, a tool fix, or a new signature in
tools/hooks/tooling_error_signatures.json -- record it and clear the marker:

  bash tools/dc.sh fix-tooling-incident --fixed \\
       --guard <path/you/created/or/changed> \\
       --root-cause "<one line>" --verify "<how you confirmed it>"

If this was a genuine misfire, clear it (and help tighten the signature):
  bash tools/dc.sh fix-tooling-incident --false-positive "<why this was not a real failure>"
"""


# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------

def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0

    tool_name = payload.get("tool_name", "")
    if not tool_name:
        return 0

    root = project_root()
    if (root / OFF_SWITCH).exists():
        return 0  # operator disabled the guard

    signatures = load_signatures()
    if not signatures:
        return 0  # no library -> nothing to enforce (fail open)

    sig = None
    command = ""
    snippet = ""

    tool_input = payload.get("tool_input", {}) or {}

    if tool_name in ("Edit", "Write", "MultiEdit"):
        file_path = tool_input.get("file_path", "")
        if file_path and is_build_critical(file_path, signatures) and file_has_crlf(file_path):
            sig = find_special_signature(signatures, "special:build_file_crlf")
            command = f"{tool_name} {file_path}"
            snippet = f"{file_path} contains CRLF (\\r) bytes after the write."
    elif tool_name in ("Bash", "PowerShell"):
        command = tool_input.get("command", "")
        output = extract_output(payload)
        sig = classify_text(tool_name, output, signatures)
        if sig is not None:
            # capture a short snippet around the first match for the directive
            snippet = _first_match_snippet(sig, _real_output(output))

    if sig is None:
        return 0

    if sig.get("tier") == "warn":
        print(f"TOOLING HINT ({sig.get('class')}/{sig.get('id')}): {sig.get('summary')}", file=sys.stderr)
        print(f"  Likely cause: {sig.get('root_cause')}", file=sys.stderr)
        return 0

    # block tier
    incident = {
        "seen": time.strftime("%Y-%m-%dT%H:%M:%S"),
        "tool": tool_name,
        "class": sig.get("class"),
        "signature": sig.get("id"),
        "summary": sig.get("summary"),
        "command": (command or "")[:500],
        "snippet": (snippet or "")[:500],
        "root_cause": sig.get("root_cause"),
        "permanent_fix": sig.get("permanent_fix", []),
        "suggested_guard": sig.get("suggested_guard"),
    }
    record_incident(root, incident)
    print(build_directive(sig, command, snippet), file=sys.stderr)
    return 2


def _first_match_snippet(sig: dict, text: str) -> str:
    low = text.lower()
    for sub in sig.get("substrings", []):
        idx = low.find(sub.lower()) if sub else -1
        if idx >= 0:
            start = max(0, idx - 40)
            return text[start:idx + len(sub) + 40]
    for rx in sig.get("regexes", []):
        try:
            m = re.search(rx, text, re.IGNORECASE)
        except re.error:
            m = None
        if m:
            start = max(0, m.start() - 40)
            return text[start:m.end() + 40]
    return ""


if __name__ == "__main__":
    sys.exit(main())
