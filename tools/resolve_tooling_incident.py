#!/usr/bin/env python3
"""Resolve (clear) a tooling incident raised by tools/hooks/tooling_error_guard.py.

This is the ONLY clean way to clear .bb2_tooling_incident.json and let the Stop
hook (tooling_incident_stop_guard.sh) allow the turn to end. It enforces the
debugging-discipline rule: a tooling error must be fixed PERMANENTLY, not
papered over.

Three resolution modes (exactly one required):

  --fixed --guard <path> --root-cause "..." --verify "..."
        The expected path. <path> is the file you created/changed to PREVENT
        recurrence (a hook, a .gitattributes entry, a fixed tool, a new
        signature). It is verified to show a recent change (git-dirty or
        recently committed) -- you can't name an unrelated file.

  --false-positive "<why this was not a real failure>"
        The guard misfired. Logged so the signature can be tightened.

  --defer "<why it can't be permanently fixed right now>"
        Escape hatch (discouraged). Logged to the ledger as known-unfixed.

Every resolution is appended to docs/tooling_incidents.md (the running ledger),
then the marker is cleared. Usually invoked as:
    python3 tools/resolve_tooling_incident.py <args>   # (formerly: bash tools/dc.sh fix-tooling-incident)
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

MARKER_NAME = ".bb2_tooling_incident.json"
LEDGER_REL = Path("docs") / "tooling_incidents.md"

LEDGER_HEADER = """# Tooling Incidents Ledger

Append-only record of tooling / shell / environment failures caught by the
tooling-error guard (`tools/hooks/tooling_error_guard.py`) and how they were
resolved. Each entry is one resolution.

This ledger is institutional memory: before reaching for a workaround, scan it
for the failure class you're seeing -- the permanent fix may already be
documented. See `CLAUDE.md` (Hooks) and the `debugging-discipline` memory rule.

---
"""


def project_root() -> Path:
    try:
        out = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True, text=True, timeout=5,
        )
        if out.returncode == 0 and out.stdout.strip():
            return Path(out.stdout.strip())
    except Exception:
        pass
    return Path.cwd()


def git_available(root: Path) -> bool:
    try:
        r = subprocess.run(["git", "-C", str(root), "rev-parse", "--git-dir"],
                           capture_output=True, text=True, timeout=5)
        return r.returncode == 0
    except Exception:
        return False


def guard_recent_change(root: Path, guard_abs: Path) -> tuple[bool, str]:
    """Return (is_recent, note). If git is unavailable, accept (can't verify)."""
    if not git_available(root):
        return True, "git unavailable -- verification skipped"
    # Uncommitted modification / addition?
    try:
        r = subprocess.run(["git", "-C", str(root), "status", "--porcelain", "--", str(guard_abs)],
                           capture_output=True, text=True, timeout=10)
        if r.returncode == 0 and r.stdout.strip():
            return True, "uncommitted change"
    except Exception:
        pass
    # Touched in the last 5 commits?
    try:
        recent = subprocess.run(["git", "-C", str(root), "log", "-n", "5", "--pretty=%H"],
                                capture_output=True, text=True, timeout=10)
        last = subprocess.run(["git", "-C", str(root), "log", "-n", "1", "--pretty=%H", "--", str(guard_abs)],
                              capture_output=True, text=True, timeout=10)
        recent_set = set(recent.stdout.split())
        h = last.stdout.strip()
        if h and h in recent_set:
            return True, f"committed in {h[:8]}"
    except Exception:
        pass
    return False, "no recent change detected"


def load_marker(root: Path) -> dict | None:
    marker = root / MARKER_NAME
    if not marker.exists() or marker.stat().st_size == 0:
        return None
    try:
        return json.loads(marker.read_text(encoding="utf-8"))
    except Exception:
        # Unparseable but present: treat as a minimal unresolved incident.
        return {"status": "unresolved", "incidents": [{}], "count": 1}


def append_ledger(root: Path, entry: str) -> None:
    ledger = root / LEDGER_REL
    ledger.parent.mkdir(parents=True, exist_ok=True)
    if not ledger.exists():
        ledger.write_text(LEDGER_HEADER, encoding="utf-8", newline="\n")
    with open(ledger, "a", encoding="utf-8", newline="\n") as f:
        f.write(entry)


def latest(marker: dict) -> dict:
    incs = marker.get("incidents") or [{}]
    return incs[-1] if incs else {}


def main() -> int:
    ap = argparse.ArgumentParser(prog="resolve_tooling_incident.py",
                                 description="Resolve a tooling incident.")
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--fixed", action="store_true",
                      help="A permanent guard was implemented (requires --guard).")
    mode.add_argument("--false-positive", metavar="WHY",
                      help="The guard misfired; explain why this was not a real failure.")
    mode.add_argument("--defer", metavar="WHY",
                      help="Cannot permanently fix right now (discouraged); explain why.")
    ap.add_argument("--guard", metavar="PATH",
                    help="File you created/changed to prevent recurrence (required with --fixed).")
    ap.add_argument("--root-cause", metavar="STR", default="")
    ap.add_argument("--verify", metavar="STR", default="")
    args = ap.parse_args()

    root = project_root()
    marker_path = root / MARKER_NAME
    marker = load_marker(root)
    if marker is None:
        print("No active tooling incident (.bb2_tooling_incident.json not present). Nothing to resolve.")
        return 0

    inc = latest(marker)
    klass = inc.get("class", "?")
    sigid = inc.get("signature", "?")
    cmd = inc.get("command", "")
    count = marker.get("count", len(marker.get("incidents", []) or [1]))
    ts = time.strftime("%Y-%m-%d %H:%M:%S")

    if args.fixed:
        if not args.guard:
            print("ERROR: --fixed requires --guard <path> (the file you created/changed to "
                  "prevent recurrence).", file=sys.stderr)
            return 2
        # Resolve the guard path against root and cwd.
        candidates = [Path(args.guard), root / args.guard, Path.cwd() / args.guard]
        guard_abs = next((c for c in candidates if c.exists()), None)
        if guard_abs is None:
            print(f"ERROR: --guard path not found: {args.guard}", file=sys.stderr)
            print("Point to the file you created or modified to prevent this error recurring.", file=sys.stderr)
            return 2
        recent, note = guard_recent_change(root, guard_abs.resolve())
        if not recent:
            print(f"ERROR: --guard '{args.guard}' shows {note}.", file=sys.stderr)
            print("The guard must be a file you just created or changed to PREVENT recurrence "
                  "(e.g. a hook, a .gitattributes entry, a fixed tool, or a new signature in "
                  "tools/hooks/tooling_error_signatures.json).", file=sys.stderr)
            print("If the failure was a genuine one-off you cannot guard against, use "
                  "--defer \"<why>\" instead.", file=sys.stderr)
            return 2
        if not args.root_cause:
            print("ERROR: --fixed requires --root-cause \"<one line>\".", file=sys.stderr)
            return 2
        rel = _relpath(root, guard_abs)
        entry = (f"\n## {ts} — RESOLVED ({klass}/{sigid})\n"
                 f"- **Triggering command:** `{cmd}`\n"
                 f"- **Root cause:** {args.root_cause}\n"
                 f"- **Permanent guard:** `{rel}` ({note})\n"
                 f"- **Verified by:** {args.verify or '(not stated)'}\n"
                 f"- **Occurrences this incident:** {count}\n")
        append_ledger(root, entry)
        marker_path.unlink(missing_ok=True)
        print(f"Resolved {klass}/{sigid}. Guard: {rel} ({note}).")
        print(f"Logged to {LEDGER_REL.as_posix()} and cleared {MARKER_NAME}.")
        return 0

    if args.false_positive is not None:
        why = args.false_positive.strip()
        if not why:
            print("ERROR: --false-positive requires a non-empty reason.", file=sys.stderr)
            return 2
        entry = (f"\n## {ts} — FALSE POSITIVE ({klass}/{sigid})\n"
                 f"- **Triggering command:** `{cmd}`\n"
                 f"- **Why not a real failure:** {why}\n"
                 f"- **Action:** tighten signature `{sigid}` in tools/hooks/tooling_error_signatures.json "
                 f"so it no longer fires on this output.\n")
        append_ledger(root, entry)
        marker_path.unlink(missing_ok=True)
        print(f"Recorded false positive for {klass}/{sigid}; cleared {MARKER_NAME}.")
        print(f"Please tighten signature '{sigid}' so the next agent doesn't hit the same misfire.")
        return 0

    if args.defer is not None:
        why = args.defer.strip()
        if not why:
            print("ERROR: --defer requires a non-empty reason.", file=sys.stderr)
            return 2
        entry = (f"\n## {ts} -- DEFERRED ({klass}/{sigid})  [known-unfixed]\n"
                 f"- **Triggering command:** `{cmd}`\n"
                 f"- **Why unfixable now:** {why}\n")
        append_ledger(root, entry)
        marker_path.unlink(missing_ok=True)
        print(f"[deferred] {klass}/{sigid} logged as known-unfixed; cleared {MARKER_NAME}.")
        print("This is logged in the ledger. Prefer a permanent fix when possible.")
        return 0

    return 2  # unreachable (mutually-exclusive required group)


def _relpath(root: Path, p: Path) -> str:
    try:
        return p.resolve().relative_to(root.resolve()).as_posix()
    except Exception:
        return p.as_posix()


if __name__ == "__main__":
    sys.exit(main())
