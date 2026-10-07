#!/usr/bin/env python3
"""SessionStart hook - print ONE short line naming the top of engine/queue.json.

Reads the committed queue directly (no toolchain, no build). Output is plain
ASCII so the Windows console never mojibakes it. Never blocks the session; any
error is swallowed (stdlib only).
"""
from __future__ import annotations

import json
import os
import sys
from pathlib import Path


def _dup_lead(root: Path, func: str) -> str | None:
    """COMPLETED-C analog for `func` from tools/find_duplicates.py's leads file."""
    leads = root / "tmp" / "duplicates_leads.txt"
    if not leads.is_file():
        return None
    for line in leads.read_text(encoding="utf-8", errors="replace").splitlines():
        if "~=" in line and not line.startswith("#"):
            lhs, rhs = line.split("~=", 1)
            if lhs.strip() == func:
                return rhs.split()[0] if rhs.split() else None
    return None


def main() -> int:
    try:
        try:
            sys.stdin.read()  # drain the hook payload; unused
        except Exception:
            pass
        root = Path(os.environ.get("CLAUDE_PROJECT_DIR") or ".")
        qp = root / "engine" / "queue.json"
        if not qp.exists():
            print("[queue] engine/queue.json missing - see `queue status`.")
            return 0
        items = json.loads(qp.read_text(encoding="utf-8")).get("items", [])
        active = [it for it in items if it.get("status") == "active"]
        if not active:
            print("[queue] no active items - see `queue status`.")
            return 0
        top = active[0]
        line = (f"[queue] top: {top['func']} ({top['file']}.c, dist {top['distance']}, "
                f"{len(active)} active) - ledger memory/grind/{top['func']}/")
        try:
            lead = _dup_lead(root, top["func"])
            if lead:
                line += f"; near-dup {lead}"
        except Exception:
            pass
        print(line.encode("ascii", "replace").decode("ascii"))
    except Exception:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
