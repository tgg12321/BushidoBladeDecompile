"""Active-function scope helper for tools that modify regfix.txt / asmfix.txt.

Reads `.bb2_active_func` and provides:

  current_active() -> str | None         active function, or None if not set
  enforce_scope(target_func) -> None     raises SystemExit if target_func differs
                                          from the active marker

This prevents the failure mode where a subagent (or a misconfigured tool run)
edits regfix rules for FUNCTIONS OTHER THAN THE ACTIVE ONE, which has happened
when:
- subagent #1 on func_8007D3F8 deleted 79 regfix rules from func_8005D554,
  func_80074B18, func_8007352C — those functions had no business being edited
  during work on func_8007D3F8.

Set BB2_SCOPE_OVERRIDE=1 to bypass (only for explicit cross-function edits
authorized by the user, e.g., post-match label-drift fixes).
"""

import os
import sys
from pathlib import Path


def project_root() -> Path:
    """Find project root by walking up from cwd until .bb2_active_func dir is reached."""
    p = Path.cwd().resolve()
    for _ in range(8):
        if (p / ".bb2_active_func").exists() or (p / "Makefile").exists():
            return p
        if p.parent == p:
            break
        p = p.parent
    return Path.cwd().resolve()


def current_active() -> str | None:
    """Return the active function name from .bb2_active_func, or None."""
    f = project_root() / ".bb2_active_func"
    if not f.exists():
        return None
    text = f.read_text(encoding="utf-8").strip()
    return text if text else None


def enforce_scope(target_func: str, *, action: str = "modify regfix/asmfix rules") -> None:
    """Refuse to act on `target_func` if it differs from the active marker.

    Raises SystemExit(2) with a clear message if scope is violated.
    Pass-through silently if:
      - .bb2_active_func is unset (no active context, OK)
      - target_func == active
      - BB2_SCOPE_OVERRIDE=1 in env

    `action` is a phrase like "append regfix rule" — the func name is appended
    automatically, so don't include it in `action` itself.
    """
    if os.environ.get("BB2_SCOPE_OVERRIDE") == "1":
        return
    active = current_active()
    if not active:
        return
    if target_func == active:
        return
    print(
        f"REFUSED: cannot {action} `{target_func}` while active function is `{active}`.\n"
        f"\n"
        f"Cross-function regfix edits during active work caused subagent #1's regression of\n"
        f"79 rules across func_8005D554/func_80074B18/func_8007352C while it was nominally\n"
        f"working on func_8007D3F8. Tools refuse this by default.\n"
        f"\n"
        f"Valid escape hatches:\n"
        f"  1. Finish the active function and clear the marker (commit auto-clears it).\n"
        f"  2. If the user explicitly authorizes a cross-function fix (e.g., post-match\n"
        f"     label-drift on siblings), set `BB2_SCOPE_OVERRIDE=1` for that one command.\n"
        f"  3. If the active marker is stale, run `bash tools/dc.sh release` (user-only).",
        file=sys.stderr,
    )
    sys.exit(2)
