#!/usr/bin/env python3
"""Generate a summary of session progress on cheat retirement and asmfix work.

Useful for handoff between sessions or for user review on return.

Shows:
- Cheats retired this session (commits matching cheat-cleanup:* pattern)
- Stale-rule cleanups
- Current cheat audit state
- Current asmfix queue size
- Outstanding intractable items (with memory file references)

Usage: python3 tools/session_summary.py [--since GIT_REF]
"""
import argparse
import re
import subprocess
import sys


def git_log_since(ref: str) -> list[str]:
    """Return list of commit subjects since ref."""
    r = subprocess.run(
        ["git", "log", f"{ref}..HEAD", "--oneline"],
        capture_output=True, text=True,
    )
    return r.stdout.strip().split("\n") if r.stdout.strip() else []


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--since", default="ac6c4f1", help="Git ref to summarize since")
    args = ap.parse_args()

    print("=" * 70)
    print("BB2 DECOMP SESSION SUMMARY")
    print("=" * 70)
    print()

    commits = git_log_since(args.since)
    cheat_commits = [c for c in commits if "cheat-cleanup" in c]
    cleanup_commits = [c for c in commits if "cleanup:" in c and "cheat-cleanup" not in c]
    tool_commits = [c for c in commits if "tools:" in c]
    queue_commits = [c for c in commits if "queue:" in c]
    other_commits = [
        c for c in commits
        if not any(s in c for s in ("cheat-cleanup", "cleanup:", "tools:", "queue:"))
    ]

    print(f"Total commits since {args.since}: {len(commits)}")
    print()

    print(f"Cheat retirements ({len(cheat_commits)}):")
    for c in cheat_commits:
        print(f"  {c}")
    print()

    print(f"Stale-rule cleanups ({len(cleanup_commits)}):")
    for c in cleanup_commits:
        print(f"  {c}")
    print()

    print(f"New tooling ({len(tool_commits)}):")
    for c in tool_commits:
        print(f"  {c}")
    print()

    print(f"Queue refresh ({len(queue_commits)}):")
    for c in queue_commits:
        print(f"  {c}")
    print()

    if other_commits:
        print(f"Other ({len(other_commits)}):")
        for c in other_commits:
            print(f"  {c}")
        print()

    # Current state
    print("--- Current state ---")
    r = subprocess.run(["python3", "tools/audit_asm_cheats.py", "--summary"],
                       capture_output=True, text=True)
    for line in r.stdout.split("\n"):
        if line.strip():
            print(f"  {line}")
    print()

    # Outstanding intractable items
    print("--- Outstanding intractable items ---")
    print("  See memory/feedback_voice_contorol_intractable.md for v11 source")
    print("  (best pure-C attempt = 26 diffs; LICM + strength-reduction prevent")
    print("  reaching 0 diffs without new tooling).")
    print()
    print("  saTan0Main: 221 wildcard substs = full asm replacement disguised as")
    print("  regfix. Requires from-scratch decomp (~233 insns) or canonical-asm")
    print("  authorization.")
    print()

    print("--- Recommended next session ---")
    print("  1. Run `python3 tools/canonical_asm_candidates.py` for review list")
    print("  2. User authorization for canonical-asm path on intractable functions")
    print("  3. OR multi-session tooling work (regfix-position-shifter)")
    print("  4. OR focused per-function decomp on smaller asmfix items")

    return 0


if __name__ == "__main__":
    sys.exit(main())
