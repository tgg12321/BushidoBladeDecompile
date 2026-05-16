#!/usr/bin/env python3
"""diff_naming_proposals.py -- compare two proposals.csv runs.

Use after the decomp agent commits more matches (which can change the
call graph / Kengo-claimant counts) to see what proposals changed.

Usage:
    python3 tools/diff_naming_proposals.py OLD.csv NEW.csv
    python3 tools/diff_naming_proposals.py --before-commit                     # vs git HEAD
    python3 tools/diff_naming_proposals.py docs/naming/proposals.csv NEW.csv

Reports:
  - functions newly proposed (NEW only)
  - functions no longer in NEW (became named, or evidence weakened)
  - proposals where the proposed name changed
  - proposals where the confidence changed
"""
from __future__ import annotations

import argparse
import csv
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def load_csv(p: Path) -> dict[str, dict]:
    if not p.exists():
        print(f"ERROR: {p} does not exist", file=sys.stderr)
        return {}
    out: dict[str, dict] = {}
    for r in csv.DictReader(p.open()):
        out[r["current_name"]] = r
    return out


def load_git_head(path: str) -> dict[str, dict]:
    """Load the version of `path` from git HEAD without checking out."""
    try:
        text = subprocess.check_output(["git", "show", f"HEAD:{path}"],
                                        cwd=ROOT, text=True, errors="replace")
    except subprocess.CalledProcessError:
        print(f"ERROR: git show HEAD:{path} failed", file=sys.stderr)
        return {}
    out: dict[str, dict] = {}
    reader = csv.DictReader(text.splitlines())
    for r in reader:
        out[r["current_name"]] = r
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("old", nargs="?", help="Old proposals CSV path")
    ap.add_argument("new", nargs="?", help="New proposals CSV path")
    ap.add_argument("--before-commit", action="store_true",
                    help="Compare working-tree proposals.csv vs git HEAD version")
    args = ap.parse_args()

    if args.before_commit:
        old = load_git_head("docs/naming/proposals.csv")
        new = load_csv(ROOT / "docs" / "naming" / "proposals.csv")
        old_name = "HEAD"
        new_name = "working-tree"
    elif args.old and args.new:
        old = load_csv(Path(args.old))
        new = load_csv(Path(args.new))
        old_name, new_name = args.old, args.new
    else:
        ap.print_help()
        return 1

    if not old and not new:
        return 1

    old_funcs = set(old)
    new_funcs = set(new)
    added = new_funcs - old_funcs
    removed = old_funcs - new_funcs
    common = old_funcs & new_funcs

    name_changed = []
    conf_changed = []
    for fn in sorted(common):
        if old[fn].get("proposed_name") != new[fn].get("proposed_name"):
            name_changed.append(fn)
        elif old[fn].get("confidence") != new[fn].get("confidence"):
            conf_changed.append(fn)

    print(f"Compare {old_name} -> {new_name}")
    print(f"  total old: {len(old_funcs)}")
    print(f"  total new: {len(new_funcs)}")
    print(f"  added: {len(added)}")
    print(f"  removed: {len(removed)}")
    print(f"  name changed: {len(name_changed)}")
    print(f"  confidence changed: {len(conf_changed)}")

    if added:
        print("\n## Added (in NEW only)")
        for fn in sorted(added)[:50]:
            r = new[fn]
            print(f"  + {fn}  -> {r.get('proposed_name','?')}  [{r.get('confidence','?')}]")
        if len(added) > 50:
            print(f"  ... {len(added) - 50} more")
    if removed:
        print("\n## Removed (in OLD only)")
        for fn in sorted(removed)[:50]:
            r = old[fn]
            print(f"  - {fn}  was -> {r.get('proposed_name','?')}  [{r.get('confidence','?')}]")
        if len(removed) > 50:
            print(f"  ... {len(removed) - 50} more")
    if name_changed:
        print("\n## Proposed name changed")
        for fn in name_changed[:50]:
            print(f"  ~ {fn}  {old[fn].get('proposed_name','?')!r} -> "
                  f"{new[fn].get('proposed_name','?')!r}  "
                  f"[{old[fn].get('confidence','?')} -> {new[fn].get('confidence','?')}]")
    if conf_changed:
        print("\n## Confidence changed (same name)")
        for fn in conf_changed[:50]:
            print(f"  ~ {fn}  [{old[fn].get('confidence','?')} -> {new[fn].get('confidence','?')}]")

    # Top-line shifts
    old_by_conf = Counter(r.get("confidence", "?") for r in old.values())
    new_by_conf = Counter(r.get("confidence", "?") for r in new.values())
    print("\n## Confidence distribution shift")
    print(f"{'conf':<8s} {'old':>5s} {'new':>5s} {'delta':>6s}")
    for c in ("high", "medium", "low", "none"):
        o = old_by_conf.get(c, 0)
        n = new_by_conf.get(c, 0)
        print(f"{c:<8s} {o:>5d} {n:>5d} {n-o:>+6d}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
