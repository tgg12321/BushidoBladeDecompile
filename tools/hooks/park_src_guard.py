#!/usr/bin/env python3
"""park_src_guard — git commit-msg hook.

Blocks `park:` commits that also modify build-pipeline files.

Why this exists
---------------
A `park:` commit means "the function isn't pure-C-matchable yet under current
understanding, queue the wall, advance the queue." It is NOT a WIP partial-
reconstruction commit. The project's standing rule is that build files reach
COMPLETED-C (or COMPLETED-INLINE-ASM-CANONICAL) before they touch main —
partial progress lives in .bb2_attempts/ or memory/ notes, never in src/.

The specific failure mode this guard prevents (commit 82997aa, 2026-05-31):
a worker committed a partial pure-C reconstruction as a `park:` commit while
a build-time bridge kept the oracle SHA1 green; the partial body shifted
cc1's .L label count and silently broke a sibling — the cascade only became
visible on a later worker's `verify-oracle --rebuild`, halfway through a
multi-iteration workflow.

Build files in scope
--------------------
Per CLAUDE.md / AGENTS.md (the same set that requires LF line endings):

  src/*.c, src/*.h, include/*, *.s, *.h
  sdata*.txt, named_syms.txt, undefined_syms_auto.txt, undefined_funcs_auto.txt,
  expand_lb_funcs.txt, expand_dest_funcs.txt, multu_funcs.txt,
  multu_pad_funcs.txt, maspsx_prefill_label_funcs.txt
  *.ld, splat.yaml, Makefile
  tools/prologue_config.json

Override (use sparingly, only when a `park:` legitimately needs to touch a
build file — e.g. fixing a sibling's stale rules that block the queue):

  Add `[skip-park-src-guard]` anywhere in the commit body, AND include a
  one-line justification explaining why the src change is necessary for the
  park (not the function being parked).

Install
-------
  cp tools/hooks/park_src_guard.py .git/hooks/commit-msg
  chmod +x .git/hooks/commit-msg
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

# Patterns matching files in the build pipeline. Touching any of these in
# a `park:` commit is the masking-via-bridge failure mode.
BUILD_FILE_PATTERNS = [
    re.compile(r"^src/.+\.(c|h)$"),
    re.compile(r"^include/.+$"),
    re.compile(r"^.+\.s$"),
    re.compile(r"^sdata.*\.txt$"),
    re.compile(r"^named_syms\.txt$"),
    re.compile(r"^undefined_(syms|funcs)_auto\.txt$"),
    re.compile(r"^expand_l[bh]_funcs\.txt$"),
    # The remaining maspsx gate lists. Enumerated by name here, which is why
    # maspsx_prefill_label_funcs.txt (created by the 2026-09-04 owner ruling)
    # went unguarded until 2026-09-14 — add new gate lists to this set.
    re.compile(r"^expand_dest_funcs\.txt$"),
    re.compile(r"^multu(_pad)?_funcs\.txt$"),
    re.compile(r"^maspsx_prefill_label_funcs\.txt$"),
    re.compile(r"^inline_asm_canonical\.txt$"),
    re.compile(r"^.+\.ld$"),
    re.compile(r"^splat\.yaml$"),
    re.compile(r"^Makefile$"),
    re.compile(r"^tools/prologue_config\.json$"),
]


def is_park_subject(subject: str) -> bool:
    """Match `park:` (case-insensitive), tolerating leading whitespace."""
    return bool(re.match(r"^\s*park\s*:", subject, re.IGNORECASE))


def is_build_file(path: str) -> bool:
    return any(p.match(path) for p in BUILD_FILE_PATTERNS)


def staged_build_files() -> list[str]:
    r = subprocess.run(
        ["git", "diff", "--cached", "--name-only"],
        capture_output=True, text=True,
    )
    if r.returncode != 0:
        # Don't block if git itself errors — return empty, the commit
        # will fail anyway for whatever reason git is unhappy.
        return []
    return [p for p in r.stdout.splitlines() if is_build_file(p)]


def main(msg_path: str) -> int:
    try:
        msg = Path(msg_path).read_text(encoding="utf-8")
    except OSError as e:
        print(f"park_src_guard: cannot read commit message at {msg_path}: {e}",
              file=sys.stderr)
        return 0  # don't block on tooling errors

    # Strip git comment lines (start with `#`) which are stripped by git before
    # the commit lands anyway.
    body = "\n".join(line for line in msg.splitlines()
                     if not line.startswith("#"))

    lines = [line for line in body.splitlines() if line.strip()]
    if not lines:
        return 0  # empty message; git will reject for its own reasons

    subject = lines[0]
    if not is_park_subject(subject):
        return 0  # not a park commit; nothing to enforce

    # Allow explicit override
    if "[skip-park-src-guard]" in body:
        return 0

    build_files = staged_build_files()
    if not build_files:
        return 0  # clean park, allow

    # Block — explain why and offer the path forward.
    print("=" * 70, file=sys.stderr)
    print("park_src_guard: BLOCKED — `park:` commit modifies build files",
          file=sys.stderr)
    print("=" * 70, file=sys.stderr)
    print("", file=sys.stderr)
    print("Subject:", file=sys.stderr)
    print(f"  {subject}", file=sys.stderr)
    print("", file=sys.stderr)
    print("Build files in the staged diff:", file=sys.stderr)
    for f in build_files:
        print(f"  {f}", file=sys.stderr)
    print("", file=sys.stderr)
    print("Why this is blocked:", file=sys.stderr)
    print("  A `park:` commit means 'function not yet COMPLETED-C, queue the", file=sys.stderr)
    print("  wall, advance.' Partial progress on src/*.c does NOT belong in a", file=sys.stderr)
    print("  park commit — a source change can drift cc1's .L counter or", file=sys.stderr)
    print("  shift sibling codegen in ways that only surface on later", file=sys.stderr)
    print("  workers' `verify-oracle --rebuild` (the 82997aa failure mode,", file=sys.stderr)
    print("  2026-05-31, reverted by d3576c3 mid-workflow).", file=sys.stderr)
    print("", file=sys.stderr)
    print("What to do:", file=sys.stderr)
    print("  - If your src/*.c changes were partial-progress toward the C", file=sys.stderr)
    print("    body of the function being parked: REVERT them. Save analysis", file=sys.stderr)
    print("    to memory/ or a `.bb2_attempts/<func>/` note instead.", file=sys.stderr)
    print("  - If your src/*.c changes are unblocking a SIBLING (e.g.", file=sys.stderr)
    print("    de-cheating func_8007CE0C so its hardcoded-label splice rules", file=sys.stderr)
    print("    stop blocking the queue): split into TWO commits — one", file=sys.stderr)
    print("    `cheat-cleanup:` / `Match:` for the sibling-fix, then the", file=sys.stderr)
    print("    `park:` separately.", file=sys.stderr)
    print("  - If you really need to override (rare, e.g. an unrelated", file=sys.stderr)
    print("    tooling-config tweak surfaced during the park): add", file=sys.stderr)
    print("    `[skip-park-src-guard]` to the commit body AND a one-line", file=sys.stderr)
    print("    justification.", file=sys.stderr)
    print("", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else ".git/COMMIT_EDITMSG"))
