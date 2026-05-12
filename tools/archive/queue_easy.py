#!/usr/bin/env python3
"""queue_easy.py -- emit easiest N queue entries (sorted by quick-win score).

Bottleneck queue view. Standard 'dc.sh next' returns top-of-queue by
queue position (loosely sorted by size). This command sorts by an
'expected ease' heuristic so the coordinator can spawn quick wins
first.

Heuristic (lower = easier):
  base score:    insn count
  +50 penalty:   aliasing_heavy tag (more iterations than insns suggest)
  +100 penalty:  needs_lwl_fix / needs_rodata_split / gte_function tier
  -20 bonus:     same source file as a recently-matched function

Usage:
  python3 tools/queue_easy.py 5      # easiest 5 in-scope queue entries
  python3 tools/queue_easy.py 20     # easiest 20
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
QUEUE = ROOT / "WORK_QUEUE.md"


def parse_queue() -> list[dict]:
    if not QUEUE.exists():
        return []
    out = []
    in_queue = False
    in_block = False
    text = QUEUE.read_text(encoding="utf-8", errors="replace")
    for line in text.splitlines():
        if line.startswith("## Queue"):
            in_queue = True
            continue
        if in_queue and line.startswith("## "):
            in_queue = False
        if in_queue and line.strip() == "```":
            in_block = not in_block
            continue
        if in_queue and in_block:
            m = re.match(
                r"\s*\d+\s+(\S+)\s+(\d+)\s+(\S+)\s+(\S+)\s*(.*)",
                line
            )
            if m:
                out.append({
                    "func": m.group(1),
                    "insns": int(m.group(2)),
                    "rec": m.group(3),
                    "src": m.group(4),
                    "tags": m.group(5).strip(),
                })
    return out


def recent_matched_files() -> set[str]:
    """Source files that had a match in the last 50 commits."""
    try:
        out = subprocess.check_output(
            ["git", "log", "--name-only", "--format=", "-50"],
            cwd=ROOT, text=True, errors="replace"
        )
    except subprocess.CalledProcessError:
        return set()
    files = set()
    for line in out.splitlines():
        line = line.strip()
        if line.startswith("src/") and line.endswith(".c"):
            files.add(line.replace("/", "\\"))
    return files


def score(entry: dict, recent: set[str]) -> int:
    s = entry["insns"]
    if "aliasing_heavy" in entry["tags"]:
        s += 50
    if entry["rec"] in {"needs_lwl_fix", "needs_rodata_split", "gte_function"}:
        s += 100
    if entry["src"] in recent:
        s -= 20
    return s


def main() -> int:
    args = sys.argv[1:]
    n = int(args[0]) if args and args[0].isdigit() else 10

    entries = parse_queue()
    if not entries:
        print("No queue entries (run dc.sh refresh-queue?)", file=sys.stderr)
        return 1

    recent = recent_matched_files()
    scored = [(score(e, recent), e) for e in entries]
    scored.sort(key=lambda t: t[0])

    print(f"# Easiest {n} queue entries (lower score = quicker win)")
    print()
    print(f"{'score':>5}  {'func':<28}  {'insns':>5}  {'rec':<22}  {'src':<22}  tags")
    print("-" * 110)
    for s, e in scored[:n]:
        recent_marker = " *" if e["src"] in recent else "  "
        print(f"{s:>5}  {e['func']:<28}  {e['insns']:>5}  {e['rec']:<22}  {e['src']:<22}{recent_marker}{e['tags']}")
    print()
    print("(* = same file as recent match)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
