#!/usr/bin/env python3
"""bb2_apply_match.py -- apply a permuter score=0 match back to src/.

Given a backlog function that the permuter matched, this:
  1. Reads permuter/<func>/output-0-*/source.c (the matching candidate)
  2. Locates the function definition in src/<file>.c
  3. Replaces the function body with the permuter's version
  4. (Optional) comments out the regfix rule(s) for the function

Use: bb2_apply_match.py <func> <src/file.c> [--retire-regfix]

The user should manually inspect the diff and re-run `make` to verify
SHA1 still matches before committing.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path


def read_lf(path: Path) -> str:
    return path.read_bytes().decode("utf-8", errors="ignore")


def write_lf(path: Path, content: str) -> None:
    # Hard-assert no CRLF leaked in. Windows Python text-mode silently
    # converts \n -> \r\n on write; binary-mode + explicit UTF-8 prevents
    # that, but defense in depth here.
    if "\r\n" in content:
        raise SystemExit("REFUSING to write CRLF -- LF only for build files")
    path.write_bytes(content.encode("utf-8"))


def _is_wsl() -> bool:
    """Detect whether we're already running inside WSL (vs Windows host)."""
    try:
        with open("/proc/version") as f:
            return "microsoft" in f.read().lower() or "wsl" in f.read().lower()
    except Exception:
        return False


def _wsl_or_native(cmd: str) -> list[str]:
    """Build a subprocess argv that runs `cmd` via wsl bash from Windows,
    or native bash if already inside WSL.
    """
    if _is_wsl():
        return ["bash", "-c", cmd]
    return ["wsl", "bash", "-c", cmd]


def _run_make(wsl_root: str, src_path: Path) -> tuple[bool, str, bool]:
    """Run make once. Returns (matches_sha1, output_tail, label_drift_detected)."""
    obj_name = src_path.stem + ".o"
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'rm -f build/src/{obj_name} && make 2>&1 | tail -20'
    )
    r = subprocess.run(_wsl_or_native(cmd),
                       capture_output=True, text=True, timeout=300)
    out = (r.stdout or "") + (r.stderr or "")
    matches = "bb2 matches!" in out
    # Label-drift signature: ld reports "undefined reference to `.L<N>`"
    label_drift = "undefined reference to `.L" in out
    return matches, out, label_drift


def _run_fix_label_drift(wsl_root: str) -> tuple[bool, str]:
    """Invoke dc.sh fix-label-drift --apply. Returns (ran_cleanly, output)."""
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'bash tools/dc.sh fix-label-drift --apply 2>&1 | tail -15'
    )
    r = subprocess.run(_wsl_or_native(cmd),
                       capture_output=True, text=True, timeout=120)
    out = (r.stdout or "") + (r.stderr or "")
    return r.returncode == 0, out


def run_make_and_check_sha1(wsl_root: str, src_path: Path) -> tuple[bool, str]:
    """Rebuild touched .c, run make, return (matches, last_lines).

    Two-pass cascade-recovery: if the first make fails with label-drift
    (undefined .L<N> reference, common when applying a match to a
    label-rich file like text1b.c or main.c), auto-invoke
    `dc.sh fix-label-drift --apply` and re-run make. Roll back only if
    STILL mismatched after the drift fix.
    """
    matches, out, drift = _run_make(wsl_root, src_path)
    if matches:
        return True, out

    if drift:
        # Try the auto-repair before giving up
        _, drift_log = _run_fix_label_drift(wsl_root)
        # Re-run make
        matches2, out2, _ = _run_make(wsl_root, src_path)
        combined = (
            out
            + "\n\n[apply] label-drift detected; auto-ran fix-label-drift:\n"
            + drift_log
            + "\n[apply] re-running make:\n"
            + out2
        )
        return matches2, combined

    return False, out


def record_registry_entry(func: str, src_file: str, regfix_retired: int,
                          mutation_summary: str, permuter_dir: Path) -> None:
    """Append a JSON entry to backlog_results/match_registry.json.
    Tracks what mutations work for what rule shapes -- future agents
    can query this for "try this first" hints.
    """
    registry_path = Path("backlog_results/match_registry.json")
    registry_path.parent.mkdir(parents=True, exist_ok=True)
    entries = []
    if registry_path.exists():
        try:
            entries = json.loads(registry_path.read_text(encoding="utf-8"))
        except Exception:
            entries = []
    entries.append({
        "func": func,
        "src_file": src_file,
        "regfix_rules_retired": regfix_retired,
        "mutation_summary": mutation_summary,
        "permuter_dir": str(permuter_dir),
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
    })
    registry_path.write_bytes(
        json.dumps(entries, indent=2).encode("utf-8")
    )


def extract_func_from_source_c(text: str, func_name: str) -> str | None:
    """Extract the function body (including signature) from a permuter
    output's source.c file by depth-tracking the braces.
    """
    lines = text.split("\n")
    pat = re.compile(rf"\b{re.escape(func_name)}\s*\(")
    start = None
    for i, line in enumerate(lines):
        if pat.search(line) and "{" not in line:
            if line.rstrip().endswith(";"):
                continue
            start = i
            break
        elif pat.search(line) and "{" in line and "(" in line:
            if line.rstrip().endswith(";"):
                continue
            start = i
            break
    if start is None:
        return None

    # Find first { from start
    brace = None
    for j in range(start, min(start + 10, len(lines))):
        if "{" in lines[j]:
            brace = j
            break
    if brace is None:
        return None

    depth = 0
    in_str = False
    in_chr = False
    end = None
    for j in range(brace, len(lines)):
        line = lines[j]
        for ch in line:
            if in_str:
                if ch == '"':
                    in_str = False
                continue
            if in_chr:
                if ch == "'":
                    in_chr = False
                continue
            if ch == '"':
                in_str = True
            elif ch == "'":
                in_chr = True
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    end = j
                    break
        if end is not None:
            break
    if end is None:
        return None
    return "\n".join(lines[start:end + 1])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("func")
    p.add_argument("src_file")
    p.add_argument("--retire-regfix", action="store_true",
                   help="Comment out the function's regfix rules in regfix.txt")
    p.add_argument("--permuter-dir", default=None,
                   help="Override permuter dir (default: permuter/<func>)")
    p.add_argument("--verify", action="store_true", default=True,
                   help="Run make and check SHA1; rollback on mismatch (default: on)")
    p.add_argument("--no-verify", dest="verify", action="store_false",
                   help="Skip the build verify step (caller will do it)")
    p.add_argument("--no-registry", action="store_true",
                   help="Skip writing to backlog_results/match_registry.json")
    args = p.parse_args()

    pdir = Path(args.permuter_dir) if args.permuter_dir else Path(f"permuter/{args.func}")
    src_path = Path(args.src_file)
    regfix_path = Path("regfix.txt")
    if not src_path.exists():
        sys.exit(f"src file not found: {src_path}")

    # Find output-0-N
    matches = sorted(pdir.glob("output-0-*"))
    if not matches:
        sys.exit(f"No score=0 output in {pdir} -- nothing to apply")
    match_dir = matches[0]
    print(f"Using match: {match_dir}")

    match_source = read_lf(match_dir / "source.c")
    matched_body = extract_func_from_source_c(match_source, args.func)
    if matched_body is None:
        sys.exit(f"Could not extract {args.func} from {match_dir}/source.c")

    src_text_before = read_lf(src_path)
    current_body = extract_func_from_source_c(src_text_before, args.func)
    if current_body is None:
        sys.exit(f"Could not find {args.func} in {src_path}")

    # Snapshot for rollback
    rf_before = read_lf(regfix_path) if regfix_path.exists() else ""

    changed_src = False
    if current_body == matched_body:
        print("src body already matches; nothing to apply.")
    else:
        print(f"  Replacing {len(current_body.split(chr(10)))}-line body "
              f"with {len(matched_body.split(chr(10)))}-line matched version")
        src_text_after = src_text_before.replace(current_body, matched_body, 1)
        write_lf(src_path, src_text_after)
        print(f"  Updated {src_path}")
        changed_src = True

    retired = 0
    if args.retire_regfix:
        new_lines = []
        for line in rf_before.split("\n"):
            stripped = line.lstrip()
            if stripped.startswith(f"{args.func}:") and not stripped.startswith("#"):
                new_lines.append(f"# RETIRED (targeted-permuter): {line}")
                retired += 1
            else:
                new_lines.append(line)
        if retired > 0:
            write_lf(regfix_path, "\n".join(new_lines))
            print(f"  Retired {retired} regfix rule(s) for {args.func}")
        else:
            print(f"  No active regfix rules found for {args.func}")

    if args.verify and (changed_src or retired > 0):
        root = Path.cwd().resolve()
        wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
        print(f"  Verifying build...")
        matches_sha, tail = run_make_and_check_sha1(wsl_root, src_path)
        if not matches_sha:
            # Rollback
            print(f"  BUILD MISMATCH -- rolling back changes")
            print(f"  Build tail:\n{tail.strip()}")
            if changed_src:
                write_lf(src_path, src_text_before)
            if retired > 0:
                write_lf(regfix_path, rf_before)
            sys.exit(1)
        print(f"  SHA1 verified")

    # Record in registry (only if we actually changed something AND verified)
    if not args.no_registry and (changed_src or retired > 0) and args.verify:
        # Get a short mutation summary from the diff
        diff_text = ""
        diff_path = match_dir / "diff.txt"
        if diff_path.exists():
            diff_text = read_lf(diff_path)
        # Pick out first few added/removed lines as a hint
        plus_lines = [l[1:].strip() for l in diff_text.split("\n")
                      if l.startswith("+") and not l.startswith("+++") and l[1:].strip()]
        minus_lines = [l[1:].strip() for l in diff_text.split("\n")
                       if l.startswith("-") and not l.startswith("---") and l[1:].strip()]
        mutation_summary = "; ".join(plus_lines[:3]) if plus_lines else "no diff"
        record_registry_entry(args.func, str(src_path), retired,
                              mutation_summary, pdir)
        print(f"  Registry entry recorded")

    if not args.verify:
        print()
        print("Manual verify steps:")
        print(f"  1. Inspect diff: git diff {src_path}")
        if args.retire_regfix:
            print(f"  2. Inspect regfix: git diff regfix.txt")
        print(f"  3. rm -f build/src/$(basename {src_path} .c).o && make")
        print(f"  4. If 'bb2 matches!': git add ... && git commit")


if __name__ == "__main__":
    main()
