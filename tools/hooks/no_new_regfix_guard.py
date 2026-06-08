#!/usr/bin/env python3
"""no_new_regfix_guard — git commit-msg hook.

Blocks commits that add NET rule lines to `regfix.txt` or `asmfix.txt`.

Why this exists
---------------
Per `.claude/rules/no-new-regfix-rules.md` (2026-06-08): SOTN — the PS1 decomp
community standard — has NO regfix-equivalent. Their bar is "pure C OR
`INCLUDE_ASM` (= our COMPLETED-INLINE-ASM-CANONICAL)" with no middle layer.
Every regfix/asmfix rule in BB2 is debt against that bar.

This guard makes "new regfix rule" the EXCEPTIONAL case (escape-hatch only),
not the default lever. The Phase 2 migration pass that's modifying existing
rules in-place is unaffected: only NET ADDITIONS (lines starting with
`<func_name>:` introduced by the diff) trigger the block.

What counts as a "rule line"
----------------------------
A line whose first non-space char is alphanumeric/underscore and which
matches `^[a-zA-Z_][a-zA-Z_0-9]+:` — i.e. the `func_NAME:` rule prefix used
by both regfix.txt and asmfix.txt. Comments (`#`), blank lines, and
continuation lines (e.g. extended `splice` payloads) don't count.

Net delta math:
  added_rule_lines   - removed_rule_lines = NET_DELTA
  NET_DELTA <= 0  →  modification-in-place, allow
  NET_DELTA  > 0  →  new rules added, block (unless escape-hatch is present)

Escape hatch
------------
Add to the commit body:
  [infra-rule: <category>]
  <one-line justification>

Sanctioned categories (closed list):
  jtbl-infra            - rodata-split jump-table renames/jlabels/delete_between
  prologue-coordination - tools/prologue_config.json + paired regfix reorder
  maspsx-label-nop      - load-delay-after-label gate (maspsx_label_nop_funcs.txt)
  lwl-fix               - lwl/lwr handling per FIX_LWL_FILES
  multu-pad             - multu/mflo scheduling pad (canonical maspsx workaround)

Install
-------
This hook chains with the existing park_src_guard. Install via the chain
runner at `.git/hooks/commit-msg` (see install instructions in the policy doc).
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

# Rule-line regex: a func name followed by ':' at the start of the line.
# Matches both regfix and asmfix formats:
#   func_8001EEB4: $3 <-> $5 @ 14-14
#   replay_camera_rob_back_loose2: rename ".L28" "jtbl_800108CC"
RULE_LINE_RE = re.compile(r"^[a-zA-Z_][a-zA-Z_0-9]*:")

# Sanctioned infrastructure-rule categories for the escape hatch.
SANCTIONED_CATEGORIES = {
    "jtbl-infra",
    "prologue-coordination",
    "maspsx-label-nop",
    "lwl-fix",
    "multu-pad",
}

# Files under guard. The hook silently passes if neither is in the staged diff.
GUARDED_FILES = ("regfix.txt", "asmfix.txt")

# `[infra-rule: <category>]` tag parser.
INFRA_TAG_RE = re.compile(
    r"\[infra-rule:\s*([a-zA-Z][a-zA-Z0-9_-]*)\s*\]",
    re.IGNORECASE,
)


def staged_diff_unified0(paths: list[str]) -> str:
    """Return the staged unified=0 diff restricted to `paths`. Empty on error."""
    if not paths:
        return ""
    r = subprocess.run(
        ["git", "diff", "--cached", "--unified=0", "--", *paths],
        capture_output=True, text=True,
    )
    if r.returncode != 0:
        return ""
    return r.stdout


def staged_files() -> list[str]:
    r = subprocess.run(
        ["git", "diff", "--cached", "--name-only"],
        capture_output=True, text=True,
    )
    if r.returncode != 0:
        return []
    return [p for p in r.stdout.splitlines() if p.strip()]


def count_net_rule_lines(diff: str) -> int:
    """Count net added rule lines in a unified diff.

    +<line> with RULE_LINE_RE match counts +1.
    -<line> with RULE_LINE_RE match counts -1.
    Diff header lines (`+++ b/...`, `--- a/...`, `@@ ...`, `diff --git ...`)
    are skipped.
    """
    added = 0
    removed = 0
    for line in diff.splitlines():
        # Skip diff metadata / headers.
        if not line:
            continue
        if line.startswith("+++") or line.startswith("---"):
            continue
        if line.startswith("@@") or line.startswith("diff "):
            continue
        if line.startswith("index ") or line.startswith("new file") \
                or line.startswith("deleted file"):
            continue
        if line.startswith("+"):
            body = line[1:]
            if RULE_LINE_RE.match(body):
                added += 1
        elif line.startswith("-"):
            body = line[1:]
            if RULE_LINE_RE.match(body):
                removed += 1
    return added - removed


def find_infra_tag(body: str) -> str | None:
    """Return the (lowercased) infra category tag in `body`, or None."""
    m = INFRA_TAG_RE.search(body)
    if not m:
        return None
    return m.group(1).lower()


def main(msg_path: str) -> int:
    # Pre-check: are any guarded files staged? If not, silent exit.
    files = staged_files()
    touched = [p for p in files if p in GUARDED_FILES]
    if not touched:
        return 0

    try:
        msg = Path(msg_path).read_text(encoding="utf-8")
    except OSError as e:
        print(f"no_new_regfix_guard: cannot read commit message at "
              f"{msg_path}: {e}", file=sys.stderr)
        return 0  # don't block on tooling errors

    # Strip git comment lines (start with `#`).
    body = "\n".join(line for line in msg.splitlines()
                     if not line.startswith("#"))

    diff = staged_diff_unified0(list(touched))
    net = count_net_rule_lines(diff)

    if net <= 0:
        # Modifications-in-place or pure removal — always allow.
        return 0

    # Net additions detected. Check for the escape hatch.
    tag = find_infra_tag(body)
    if tag is not None:
        if tag in SANCTIONED_CATEGORIES:
            print(
                f"no_new_regfix_guard: ALLOW — {net} net rule line(s) added "
                f"under sanctioned category [infra-rule: {tag}]",
                file=sys.stderr,
            )
            return 0
        # Unknown category — fall through to BLOCK with a specific note.
        print(
            f"no_new_regfix_guard: BLOCKED — [infra-rule: {tag}] is not a "
            f"sanctioned category. Sanctioned: "
            f"{', '.join(sorted(SANCTIONED_CATEGORIES))}.",
            file=sys.stderr,
        )
        return 1

    # Block with the full message.
    print("=" * 70, file=sys.stderr)
    print(f"no_new_regfix_guard: BLOCKED — commit adds {net} net rule "
          f"line(s) to regfix.txt/asmfix.txt", file=sys.stderr)
    print("=" * 70, file=sys.stderr)
    print("", file=sys.stderr)
    print("Per .claude/rules/no-new-regfix-rules.md (2026-06-08), new "
          "regfix/asmfix", file=sys.stderr)
    print("rules are forbidden by default. SOTN has no regfix-equivalent; "
          "every new", file=sys.stderr)
    print("rule is debt against the SOTN bar.", file=sys.stderr)
    print("", file=sys.stderr)
    print("Options:", file=sys.stderr)
    print("  (a) Find a pure-C lever (the real work)", file=sys.stderr)
    print("  (b) Authorize as COMPLETED-INLINE-ASM-CANONICAL (if hand-coded "
          "signals support it)", file=sys.stderr)
    print("  (c) Park the function with documented lever-exhaustion",
          file=sys.stderr)
    print("", file=sys.stderr)
    print("Escape hatch (rare): if this rule is legitimate infrastructure, "
          "add to", file=sys.stderr)
    print("the commit body:", file=sys.stderr)
    print("  [infra-rule: <category>]  # one of: jtbl-infra, "
          "prologue-coordination,", file=sys.stderr)
    print("                            #         maspsx-label-nop, lwl-fix, "
          "multu-pad", file=sys.stderr)
    print("  <one-line justification>", file=sys.stderr)
    print("", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1
                  else ".git/COMMIT_EDITMSG"))
