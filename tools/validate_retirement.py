#!/usr/bin/env python3
"""Pre-commit validator for cheat retirement work.

Runs the full check suite before allowing a commit:
1. dc.sh build — must succeed
2. dc.sh verify <func> — must MATCH
3. dc.sh verify --all --force — confirms no sibling cascade (CRITICAL per
   feedback_cheat_cleanup_techniques.md §0)
4. python3 tools/audit_asm_cheats.py --func <func> — no cheats remain

Exits 0 only if ALL pass. Use as pre-commit gate to avoid the
2026-05-16 batch-merge regression (468 functions broke when individual
commits all reported MATCH but cumulative state broke siblings).

Usage: python3 tools/validate_retirement.py FUNC
"""
import argparse
import subprocess
import sys


def run(cmd: list[str]) -> tuple[int, str, str]:
    """Run command, return (rc, stdout, stderr)."""
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode, r.stdout, r.stderr


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    args = ap.parse_args()

    print(f"=== Validating retirement of {args.func} ===\n")

    # 1. Build
    print("[1/4] dc.sh build...")
    rc, out, err = run(["bash", "tools/dc.sh", "build"])
    if rc != 0 or "bb2 matches!" not in out:
        print(f"  FAIL: build did not match (rc={rc})")
        print(out[-2000:] if out else err[-2000:])
        return 1
    print("  PASS: bb2 matches!")

    # 2. Per-function verify
    print(f"\n[2/4] dc.sh verify {args.func}...")
    rc, out, err = run(["bash", "tools/dc.sh", "verify", args.func])
    if "MATCH" not in out or "NO MATCH" in out:
        print(f"  FAIL: per-function verify failed")
        print(out[-1000:])
        return 1
    print(f"  PASS: {args.func}: MATCH")

    # 3. Sibling regression check (force-verify all functions)
    print(f"\n[3/4] dc.sh verify --all --force (sibling drift check)...")
    rc, out, err = run(["bash", "tools/dc.sh", "verify", "--all", "--force"])
    if "All 1241 C functions match" not in out:
        print(f"  FAIL: sibling drift detected — some function(s) regressed")
        print(out[-2000:])
        return 1
    print("  PASS: all 1241 functions match")

    # 4. Audit
    print(f"\n[4/4] audit_asm_cheats.py --func {args.func}...")
    rc, out, err = run(["python3", "tools/audit_asm_cheats.py", "--func", args.func])
    if rc != 0:
        print(f"  FAIL: cheat audit flags remaining cheats")
        print(out)
        return 1
    print("  PASS: no cheats remain for this function")

    print("\n=== VALIDATION PASSED ===")
    print(f"Safe to commit {args.func} retirement.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
