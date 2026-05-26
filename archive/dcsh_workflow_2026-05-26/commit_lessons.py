#!/usr/bin/env python3
"""commit_lessons.py — query the git commit corpus for project knowledge.

The BB2 commit history encodes substantial structured technique data:
every `cheat-cleanup:` / `Match func_*` / `inline_asm_canonical:` /
`naming:` commit carries a body with the technique used, Pure-C
attempts log, audit verdicts, etc. This tool surfaces that corpus as
queryable knowledge — replacing several memory/history/*.md files that
were essentially curated commit excerpts.

Default: search subject + body of commits on `main` (most recent first).
Use --pickaxe to also include commits whose DIFF (not just message)
mentions the term — useful for "where was this function touched?"
even when not the headline subject.

Auto-detects `func_XXXXXXXX` queries and enables pickaxe automatically.

Examples:
  python3 tools/commit_lessons.py func_8004A348
  python3 tools/commit_lessons.py "delay slot fill"
  python3 tools/commit_lessons.py "scratchpad" --limit 10
  python3 tools/commit_lessons.py --type cheat-cleanup --limit 30 --brief
  python3 tools/commit_lessons.py --type Match --recent 14d
  python3 tools/commit_lessons.py "regfix.*splice" --regex
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNC_RE = re.compile(r"^func_[0-9A-Fa-f]{8}$")
COAUTHOR_RE = re.compile(r"^Co-Authored-By:", re.MULTILINE)


def git_log_search(
    query: str,
    *,
    ref: str = "main",
    limit: int = 20,
    pickaxe: bool = False,
    regex: bool = False,
    since: str | None = None,
) -> list[dict]:
    """Single git log call returning parsed commits (sha/date/subject/body)."""
    fmt = "%H%x00%aI%x00%s%x00%b%x00ENDCOMMIT"
    cmd = ["git", "-C", str(REPO_ROOT), "log", ref, f"--format={fmt}",
           f"--max-count={limit}", "-i"]
    if since:
        cmd.append(f"--since={since}")
    if query:
        cmd.extend(["--grep", query])
        if regex:
            cmd.append("--extended-regexp")
    proc = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8",
                          errors="replace")
    if proc.returncode != 0:
        print(f"git log failed: {proc.stderr}", file=sys.stderr)
        return []
    # If query is empty (type-only filter), fall back to recent commits
    if not query:
        cmd_recent = ["git", "-C", str(REPO_ROOT), "log", ref,
                      f"--format={fmt}", f"--max-count={limit}"]
        if since:
            cmd_recent.append(f"--since={since}")
        proc = subprocess.run(cmd_recent, capture_output=True, text=True,
                              encoding="utf-8", errors="replace")
        if proc.returncode != 0:
            print(f"git log failed: {proc.stderr}", file=sys.stderr)
            return []
    commits = _parse_log_output(proc.stdout)

    if pickaxe and query:
        # Pickaxe (-S) finds commits where the diff changes the term's count.
        # Useful for "where was this symbol touched?" beyond message text.
        pickaxe_cmd = ["git", "-C", str(REPO_ROOT), "log", ref,
                       f"--format={fmt}", f"--max-count={limit}",
                       "-S", query]
        if since:
            pickaxe_cmd.append(f"--since={since}")
        proc2 = subprocess.run(pickaxe_cmd, capture_output=True, text=True,
                               encoding="utf-8", errors="replace")
        if proc2.returncode == 0:
            seen = {c["sha"] for c in commits}
            for c in _parse_log_output(proc2.stdout):
                if c["sha"] not in seen:
                    commits.append(c)
                    seen.add(c["sha"])

    # Sort newest first by date
    commits.sort(key=lambda c: c["date"], reverse=True)
    return commits[:limit]


def _parse_log_output(raw: str) -> list[dict]:
    # Each commit format ends with %x00ENDCOMMIT (NUL + ENDCOMMIT),
    # followed by git's inter-commit \n. Split on "\x00ENDCOMMIT\n".
    # Last commit may end with "\x00ENDCOMMIT\n" too (trailing newline)
    # or just "\x00ENDCOMMIT".
    chunks = re.split(r"\x00ENDCOMMIT\n?", raw)
    out: list[dict] = []
    for chunk in chunks:
        # Strip leading newline (between-commit separator)
        chunk = chunk.lstrip("\n")
        if not chunk:
            continue
        parts = chunk.split("\x00")
        if len(parts) < 4:
            continue
        out.append({
            "sha": parts[0].strip(),
            "date": parts[1].strip(),
            "subject": parts[2].strip(),
            "body": parts[3].rstrip(),  # preserve internal whitespace, strip trailing
        })
    return out


def truncate_body(body: str, max_lines: int = 15) -> str:
    """Smart truncation: drop Co-Authored-By trailer, show first N lines."""
    # Strip Co-Authored-By footer
    m = COAUTHOR_RE.search(body)
    if m:
        body = body[:m.start()].rstrip()
    lines = body.split("\n")
    if len(lines) <= max_lines:
        return "\n".join(lines)
    return "\n".join(lines[:max_lines]) + f"\n    ... ({len(lines) - max_lines} more lines — use --full for all)"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="See tools/commit_lessons.py source for full docstring + examples.",
    )
    p.add_argument("query", nargs="*", help="Search term (function name, keyword, regex). Multi-word: combined into a single phrase.")
    p.add_argument("--type", "-t", help="Filter by commit subject prefix (e.g., cheat-cleanup, Match, naming, auth). Case-insensitive.")
    p.add_argument("--limit", "-n", type=int, default=20, help="Max commits to return (default: 20)")
    p.add_argument("--all-branches", "-a", action="store_true", help="Search across all branches (default: main only)")
    p.add_argument("--brief", "-b", action="store_true", help="Subject-only output (one commit per line)")
    p.add_argument("--full", "-F", action="store_true", help="Show full body (no truncation)")
    p.add_argument("--pickaxe", "-p", action="store_true", help="Also include commits whose DIFF mentions the term (slower)")
    p.add_argument("--regex", "-r", action="store_true", help="Treat query as extended regex (passed to git --grep --extended-regexp)")
    p.add_argument("--recent", help="Filter to commits within last N days (e.g., 14d or '2 weeks ago')")
    p.add_argument("--stat", "-s", action="store_true", help="Include file-stat line per commit (extra git call per hit; slower)")
    return p.parse_args()


def get_stat(sha: str) -> str:
    """Get one-line stat: 'N files changed, +Add / -Del'."""
    proc = subprocess.run(
        ["git", "-C", str(REPO_ROOT), "show", "--stat", "--format=", sha],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
    )
    if proc.returncode != 0:
        return ""
    # Last line is usually the summary
    lines = [l for l in proc.stdout.strip().split("\n") if l.strip()]
    return lines[-1].strip() if lines else ""


def main() -> int:
    args = parse_args()
    query = " ".join(args.query).strip()
    if not query and not args.type:
        print("Error: provide a query or --type", file=sys.stderr)
        return 2

    # Auto-detect function names → enable pickaxe
    pickaxe = args.pickaxe
    if query and FUNC_RE.match(query):
        pickaxe = True

    ref = "--all" if args.all_branches else "main"
    since = None
    if args.recent:
        # Allow "14d" shorthand or pass-through to git --since
        if re.match(r"^\d+d$", args.recent):
            since = f"{args.recent[:-1]} days ago"
        else:
            since = args.recent

    commits = git_log_search(
        query=query if query else "",
        ref=ref,
        limit=max(args.limit * 2, 40) if args.type else args.limit,
        pickaxe=pickaxe,
        regex=args.regex,
        since=since,
    )

    # Filter by subject prefix
    if args.type:
        type_lower = args.type.lower()
        commits = [c for c in commits if c["subject"].lower().startswith(type_lower)]
        commits = commits[:args.limit]

    if not commits:
        print(f"# No commits matching query={query!r} type={args.type!r}", file=sys.stderr)
        print("\nSuggestions:", file=sys.stderr)
        print("  - Try --all-branches if the work is on a worktree", file=sys.stderr)
        print("  - Try --pickaxe if searching for a symbol that may not be in commit messages", file=sys.stderr)
        print("  - Try --regex for more complex patterns", file=sys.stderr)
        return 1

    # Render
    label_query = query if query else "(no text query)"
    print(f"# {len(commits)} commit(s) matching {label_query!r}" + (f"  type={args.type!r}" if args.type else ""))
    print(f"# branch={'all' if args.all_branches else 'main'}" +
          (f"  pickaxe=on" if pickaxe else "") +
          (f"  since={since}" if since else ""))
    print()

    for c in commits:
        sha_short = c["sha"][:8]
        if args.brief:
            print(f"{sha_short}  {c['date'][:10]}  {c['subject']}")
            continue

        print(f"-- commit {sha_short}  {c['date'][:10]}")
        print(f"   {c['subject']}")
        if c["body"]:
            body = c["body"] if args.full else truncate_body(c["body"])
            indented = "\n".join("   " + line for line in body.split("\n"))
            print()
            print(indented)
        if args.stat:
            stat = get_stat(c["sha"])
            if stat:
                print(f"\n   [files]  {stat}")
        print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
