#!/usr/bin/env python3
"""dc.sh retire <func>: start retirement of a bridged (replace_with_asmfile) function.

Retirement means: replace the `replace_with_asmfile` bridge with a pure-C
body that matches byte-identically. This is the opposite of bridging.

Workflow:
  1. Function must have a `replace_with_asmfile` rule in asmfix.txt
  2. No other active marker may be set
  3. Current build must match SHA1 (don't start retirement from a broken state)
  4. The asmfix rule is commented out with `# RETIRE: ` prefix
     (parser ignores `#` comments, so the bridge is effectively removed)
  5. Active marker is set to <func> (hook now enforces the function)
  6. C-body location in src/<file>.c is printed for the user to edit

On successful match + commit:
  - hook clears the active marker
  - `dc.sh refresh-queue` removes the `# RETIRE: ` line entirely (clean asmfix.txt)

On abandonment (`dc.sh release <func>`):
  - active marker cleared
  - the `# RETIRE: ` prefix is auto-stripped, restoring the bridge
"""
import os
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
ASMFIX = REPO / "asmfix.txt"
ACTIVE_MARKER = REPO / ".bb2_active_func"
SRC_DIR = REPO / "src"


def find_replace_rule(func):
    """Return (line_index_0_based, line_text, all_lines) for the active
    (non-commented) replace_with_asmfile rule, or None."""
    if not ASMFIX.exists():
        return None
    lines = ASMFIX.read_text(encoding="utf-8").splitlines()
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("#"):
            continue
        if re.match(rf"^{re.escape(func)}\s*:\s*replace_with_asmfile\b", s):
            return i, line, lines
    return None


def find_retired_rule(func):
    """Return (line_index, line, all_lines) for a `# RETIRE: ` commented
    rule for func, or None."""
    if not ASMFIX.exists():
        return None
    lines = ASMFIX.read_text(encoding="utf-8").splitlines()
    for i, line in enumerate(lines):
        s = line.strip()
        if not s.startswith("# RETIRE:"):
            continue
        rest = s[len("# RETIRE:"):].strip()
        if re.match(rf"^{re.escape(func)}\s*:\s*replace_with_asmfile\b", rest):
            return i, line, lines
    return None


def find_c_body(func):
    """Locate the function definition in src/. Returns (filename, line_no, body_snippet) or None."""
    pat = re.compile(
        rf"^[a-zA-Z_][a-zA-Z0-9_ *]*?\**\s*{re.escape(func)}\s*\([^)]*\)\s*\{{",
        re.MULTILINE,
    )
    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        m = pat.search(text)
        if m:
            line_no = text[: m.start()].count("\n") + 1
            # Snapshot first 5 lines of the body for printout
            snippet = "\n".join(text[m.start():].splitlines()[:6])
            return src.name, line_no, snippet
    return None


def check_build_matches():
    """Run `dc.sh verify --all`. Return True iff SHA1 match line is present."""
    try:
        result = subprocess.run(
            ["bash", "tools/dc.sh", "verify", "--all"],
            cwd=str(REPO),
            capture_output=True, text=True, timeout=60,
        )
    except (subprocess.TimeoutExpired, FileNotFoundError) as e:
        print(f"WARN: could not run verify --all ({e}); skipping match check", file=sys.stderr)
        return True
    out = (result.stdout or "") + (result.stderr or "")
    # dc.sh verify --all prints "SHA1 match: build/bb2.exe == disc/SLUS_006.63" on success
    return "SHA1 match:" in out


def main():
    if len(sys.argv) < 2:
        print("Usage: dc.sh retire <func>", file=sys.stderr)
        return 1
    func = sys.argv[1]

    # 1. Validate not conflicting with another active function
    if ACTIVE_MARKER.exists():
        existing = ACTIVE_MARKER.read_text(encoding="utf-8").strip()
        if existing and existing != func:
            print(
                f"ERROR: another function is active: '{existing}'.\n"
                f"  Finish/match it (commit) or release it (dc.sh release) first.",
                file=sys.stderr,
            )
            return 1

    # 2. Locate the bridge rule
    rule = find_replace_rule(func)
    if rule is None:
        # Maybe already in retirement?
        retired = find_retired_rule(func)
        if retired is not None:
            print(f"ERROR: {func} already in retirement (`# RETIRE: ` line found at asmfix.txt:{retired[0]+1}).", file=sys.stderr)
            print(f"  Active marker: {ACTIVE_MARKER.read_text(encoding='utf-8').strip() if ACTIVE_MARKER.exists() else '<none>'}", file=sys.stderr)
            return 1
        print(f"ERROR: no `replace_with_asmfile` rule for {func} in asmfix.txt.", file=sys.stderr)
        print(f"  retire is only for bridged functions. If {func} is in the normal queue, use `dc.sh next` instead.", file=sys.stderr)
        return 1
    idx, line_text, all_lines = rule

    # 3. Don't start retirement from a broken build
    print(f"[retire] verifying current build matches before starting retirement...", file=sys.stderr)
    if not check_build_matches():
        print(f"ERROR: current build does NOT match SHA1. Refusing to start retirement from a broken state.", file=sys.stderr)
        print(f"  Run `dc.sh verify --all` and investigate. Retire is only safe to start from a clean tree.", file=sys.stderr)
        return 1
    print(f"[retire] build OK.", file=sys.stderr)

    # 4. Locate the C body
    body = find_c_body(func)

    # 5. Comment out the asmfix rule
    all_lines[idx] = f"# RETIRE: {line_text}"
    ASMFIX.write_text("\n".join(all_lines) + "\n", encoding="utf-8")
    print(f"[retire] asmfix.txt:{idx+1} now commented: `# RETIRE: ...`", file=sys.stderr)

    # 6. Set active marker
    ACTIVE_MARKER.write_text(func + "\n", encoding="utf-8")
    print(f"[retire] active marker set to {func}", file=sys.stderr)

    print()
    print(f"=== Retirement started: {func} ===")
    print(f"  asmfix:   line {idx+1} commented (`# RETIRE: ...`)")
    print(f"  active:   {func}  (hook now blocks `git commit` and `dc.sh next` until match)")
    if body:
        fn, ln, snippet = body
        print(f"  C body:   src/{fn}:{ln}")
        print("    " + snippet.replace("\n", "\n    "))
    else:
        print(f"  C body:   NOT FOUND in src/. You'll need to add one (try `dc.sh setup {func} <src>` to scaffold).")

    print()
    print("Next steps:")
    print(f"  1. dc.sh build-active {func}  # incremental rebuild + bridge-aware verify-c")
    print(f"  2. dc.sh diff-align {func}    # see what changed (the bridge is now gone)")
    print(f"  3. Edit src/* to iterate (smart_match, permuter, regfix, …)")
    print(f"  4. dc.sh verify-c {func}      # bridge-aware verify (NOT plain `verify` — that's bridge-blind)")
    print(f"  5. git commit                 # hook auto-clears active marker")
    print(f"  6. dc.sh refresh-queue        # removes the `# RETIRE: ` line entirely")
    print()
    print(f"To abandon retirement and restore the bridge:")
    print(f"  dc.sh release {func}          # (interactive; will restore the # RETIRE rule)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
