#!/usr/bin/env python3
"""commit-msg guard: the documentation budget (owner directive 2026-10-01).

Agents had buried the repo in markdown — 16 MB of .md, a 3.6 MB decisions log,
1.3 MB ledgers for a 127-insn function, ~100K tokens of rules auto-loaded on every
src/*.c read. History belongs in commit messages and git; files hold current state.

Checks the STAGED tree (index blobs, not the working copy):
  1. New .md files need `[new-doc]` in the commit message (justify it in the body).
     Exempt: per-function grind ledgers (memory/grind/**), docs/naming/data_evidence/.
  2. Each .claude/rules/*.md <= RULE_MAX bytes (split long policy into on-demand files).
  3. Rules whose `paths:` match src/*.c (they auto-load on every source read)
     total <= AUTOLOAD_MAX bytes.
  4. memory/grind/<func>/{hypotheses,evidence}.md <= LEDGER_MAX bytes
     (fix: python tools/grinder/grindlib.py compact-ledger <func>).
  5. Append-only logs under their caps (fix: the rotation tool named in the error).

Override: `[skip-doc-budget]` in the message, with the reason in the body.
Usage (commit-msg chain): doc_budget_guard.py <msg-file>
"""
from __future__ import annotations

import fnmatch
import re
import subprocess
import sys

RULE_MAX = 8 * 1024
AUTOLOAD_MAX = 60 * 1024
LEDGER_MAX = 64 * 1024
LOG_CAPS = {
    "docs/grind/decisions.md": (1024 * 1024, "python tools/rotate_grind_logs.py"),
    "docs/grind/journal.md": (512 * 1024, "python tools/rotate_grind_logs.py"),
    "metrics/events.jsonl": (40 * 1024 * 1024, "python tools/metrics/rotate.py"),
}
NEW_DOC_EXEMPT = ("memory/grind/", "docs/naming/data_evidence/")
LEDGER_RE = re.compile(r"^memory/grind/[^/]+/(hypotheses|evidence)\.md$")
SRC_PROBE = "src/code6cac.c"


def git(*args: str) -> str:
    return subprocess.run(["git", *args], capture_output=True, text=True,
                          encoding="utf-8", errors="replace").stdout


def staged_size(path: str) -> int | None:
    out = git("cat-file", "-s", f":{path}").strip()
    return int(out) if out.isdigit() else None


def rule_paths(text: str) -> list[str]:
    m = re.match(r"^---\n(.*?)\n---", text, re.S)
    if not m:
        return []
    pm = re.search(r"^paths:\s*(.*?)(?=^\S|\Z)", m.group(1), re.S | re.M)
    if not pm:
        return []
    return [p.strip().strip("\"'") for p in re.findall(r"[\"']?([^\s\"'\[\],#][^\"'\],\n]*)", pm.group(1))
            if p.strip() and not p.strip().startswith("-")]


def main() -> int:
    msg = open(sys.argv[1], encoding="utf-8", errors="replace").read() if len(sys.argv) > 1 else ""
    if "[skip-doc-budget]" in msg:
        return 0
    changed = [l.split("\t") for l in git("diff", "--cached", "--name-status", "--no-renames").splitlines()]
    errors: list[str] = []

    for st, *rest in changed:
        path = rest[-1]
        if st == "A" and path.endswith(".md") and not path.startswith(NEW_DOC_EXEMPT) \
                and "[new-doc]" not in msg:
            errors.append(f"new markdown file {path}: history goes in the commit message, "
                          "current state in an EXISTING doc. If a new file is truly needed, "
                          "add [new-doc] and justify it in the body.")
        if st == "D":
            continue
        size = staged_size(path)
        if size is None:
            continue
        cap = RULE_MAX
        if path.startswith(".claude/rules/") and path.endswith(".md") and size > cap:
            errors.append(f"{path} is {size} B > {cap} B: a rule holds the operative rule "
                          "only — move rulings/case history into the commit message.")
        if LEDGER_RE.match(path) and size > LEDGER_MAX:
            func = path.split("/")[2]
            errors.append(f"{path} is {size} B > {LEDGER_MAX} B: run "
                          f"`python tools/grinder/grindlib.py compact-ledger {func}`.")
        if path in LOG_CAPS and size > LOG_CAPS[path][0]:
            errors.append(f"{path} is {size} B > {LOG_CAPS[path][0]} B: run `{LOG_CAPS[path][1]}`.")

    if any(rest[-1].startswith(".claude/rules/") for _, *rest in changed):
        total, hits = 0, []
        for path in git("ls-files", "--cached", ".claude/rules/*.md").splitlines():
            text = git("show", f":{path}")
            if any(fnmatch.fnmatch(SRC_PROBE, p) for p in rule_paths(text)):
                total += len(text.encode("utf-8"))
                hits.append(path.rsplit("/", 1)[-1])
        if total > AUTOLOAD_MAX:
            errors.append(f"rules auto-loaded on src/*.c total {total} B > {AUTOLOAD_MAX} B "
                          f"({', '.join(hits)}): narrow `paths:` or trim.")

    if errors:
        sys.stderr.write("doc_budget_guard: BLOCKED (documentation budget, CLAUDE.md)\n")
        for e in errors:
            sys.stderr.write(f"  - {e}\n")
        sys.stderr.write("  Override only with [skip-doc-budget] + a reason in the body.\n")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
