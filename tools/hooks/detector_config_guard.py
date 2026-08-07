#!/usr/bin/env python3
"""detector_config_guard — git commit-msg hook.

Blocks commits whose staged detector-config edit makes a symbol DISAPPEAR
from the parsed inventory of `volatile_extern_allowlist.txt` or
`inline_asm_canonical.txt`.

Why this exists
---------------
The 2026-08-07 incident: commit 05d13f9a normalized the allowlist to LF and
ate one newline, gluing two entries into one line. The glued line is a
syntactically VALID entry that parses cleanly — it just parses to one symbol
instead of two — so a linter, an LF check, and a content hash all pass, and
the build SHA1 never moves (these files feed no bytes). D_800F7420 silently
left the allowlist and two COMPLETED-C SPU functions regressed in
cheat-stripped scoring with the oracle green.

The parse RESULT is therefore the guard surface, not the file content. This
hook diffs the staged file's parsed symbol inventory against HEAD's and blocks
on any LOST symbol. Whole-file reformatting (line-ending normalization,
re-sorting, re-wrapping) is the specific hazard: it touches every line at once
in exactly the way that produces this failure. Sub-second; runs only when one
of the two configs is actually staged.

Parsers
-------
No mirrors — both files go through the ENGINE'S OWN loaders:
  * inline_asm_canonical.txt: cheats.canonical_asm_funcs(path) takes a path,
    so the staged/HEAD blobs are written to temp files and parsed for real.
  * volatile_extern_allowlist.txt: volatile_cheats.use_allowlist(path) points
    the real loader at a temp file (the loader caches per (path, mtime), so
    the override is exact).

Escape hatch
------------
Deliberately REVOKING a grant (e.g. the D_800F1AEC vacation, 2026-08-07) is
legitimate. Add to the commit body:
  [allow-config-symbol-loss]
  <one-line justification naming the symbol(s)>

Fail-open policy
----------------
Unexpected errors (no HEAD yet, engine import failure, git oddity) WARN and
allow — a guard that breaks unrelated commits gets uninstalled, which is
worse. The block path itself never fails open.

Install
-------
Chained from tools/hooks/commit_msg_chain.sh; re-install via
  cp tools/hooks/commit_msg_chain.sh .git/hooks/commit-msg
"""
from __future__ import annotations

import os
import subprocess
import sys
import tempfile
from pathlib import Path

ESCAPE_TOKEN = "[allow-config-symbol-loss]"
WATCHED = ("volatile_extern_allowlist.txt", "inline_asm_canonical.txt")


def _git(*args: str) -> str | None:
    r = subprocess.run(["git", *args], capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else None


def _parse_via_engine(path_name: str, text: str) -> set[str] | None:
    """Parse `text` as config `path_name` using the engine's own loader."""
    repo = Path(__file__).resolve().parent.parent.parent
    sys.path.insert(0, str(repo))
    from engine import cheats, volatile_cheats as vc  # noqa: E402
    with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False,
                                     encoding="utf-8") as fh:
        fh.write(text)
        tmp = fh.name
    try:
        if path_name == "inline_asm_canonical.txt":
            return set(cheats.canonical_asm_funcs(tmp))
        with vc.use_allowlist(tmp):
            vc._volatile_extern_allowlist_cache = None
            got = set(vc._load_volatile_extern_allowlist())
        vc._volatile_extern_allowlist_cache = None
        return got
    finally:
        os.unlink(tmp)


def main() -> int:
    msg_file = sys.argv[1] if len(sys.argv) > 1 else None

    staged_names = _git("diff", "--cached", "--name-only", "HEAD")
    if staged_names is None:
        return 0  # no HEAD yet (initial commit) — nothing to compare against
    touched = [p for p in WATCHED if p in staged_names.splitlines()]
    if not touched:
        return 0

    losses: list[str] = []
    for path in touched:
        head_text = _git("show", f"HEAD:{path}")
        staged_text = _git("show", f":{path}")
        if head_text is None or staged_text is None:
            continue  # newly added or deleted file — inventory diff undefined
        old = _parse_via_engine(path, head_text)
        new = _parse_via_engine(path, staged_text)
        if old is None or new is None:
            continue
        lost = sorted(old - new)
        if lost:
            losses.append(f"  {path}: {len(lost)} symbol(s) present at HEAD no "
                          f"longer parse: {', '.join(lost)} "
                          f"({len(new)} parsed staged, {len(old)} at HEAD)")

    if not losses:
        return 0

    if msg_file and ESCAPE_TOKEN in Path(msg_file).read_text(encoding="utf-8",
                                                             errors="replace"):
        print(f"detector_config_guard: symbol loss authorized by "
              f"{ESCAPE_TOKEN}:")
        for l in losses:
            print(l)
        return 0

    print("DETECTOR-CONFIG GUARD: this commit makes symbol(s) disappear from a "
          "detector config's PARSED inventory:")
    for l in losses:
        print(l)
    print(
        "\nA silently dropped grant re-arms the cheat stripper against a\n"
        "sanctioned construct and un-completes functions with the oracle green\n"
        "(the 2026-08-07 D_800F7420 glue incident: a valid-looking line that\n"
        "parsed to one symbol instead of two). Check the staged file for glued\n"
        "lines / eaten newlines: python3 tools/spotcheck/spot_check_completed.py "
        "--mode config\n"
        f"\nIf the removal is DELIBERATE (a grant revoked, a function\n"
        f"de-authorized), add to the commit body:\n"
        f"  {ESCAPE_TOKEN}\n"
        f"  <one-line justification naming the symbol(s)>")
    return 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as e:  # fail open — see docstring
        print(f"detector_config_guard: WARNING — guard errored, allowing "
              f"commit: {e}")
        sys.exit(0)
