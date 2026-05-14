#!/usr/bin/env python3
"""Run `make` and surface NEWLY-broken regfix rules when the build mismatches.

Background. tools/regfix.py emits `regfix: WARNING: ...` on stderr when a
`subst` / `insert` / `reorder` rule fails to apply — the rule silently no-ops,
which can produce WRONG BYTES with no build error and no linker error.

The catch: ~318 such warnings fire on a clean, fully-MATCHING build. They are
dead/redundant rules that happen not to break anything. So "a regfix warning
fired" is not, by itself, a signal — the signal is buried 318-deep. That is
exactly why the 12 drift-broken rules in the func_8007CE0C session (2026-05-14)
were invisible in the build log.

This wrapper solves it with a baseline diff:

  * `.bb2_regfix_warning_baseline.txt` holds the regfix-warning set seen on the
    last known-MATCHING full build. Only `--update-baseline` writes it (wired
    to `dc.sh verify --clean`, the canonical full rebuild) — an incremental
    build only recompiles some files and must NOT clobber the full snapshot.
  * Build MATCHES        -> stay quiet (the warning set is benign by definition).
  * Build MISMATCHES     -> show only warnings that are NEW vs the baseline.
    Those are the rules that changed from firing-or-harmless to broken — the
    actual suspects. A warning for a function other than the active one is
    flagged as a SIBLING regression (the func_8007CE0C failure mode).

Usage:
    python3 tools/make_check.py [--tail N] [--update-baseline] [--all-warnings] [make-args...]

    --tail N           : suppress live output; print only the last N lines.
    --update-baseline  : on a MATCHING build, refresh the baseline snapshot.
                         Only pass this for a full rebuild (verify --clean).
    --all-warnings     : ignore the baseline; show every regfix warning.

Exit code: passes through make's exit code.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
WARN_MARKER = "regfix: WARNING:"
BASELINE = ROOT / ".bb2_regfix_warning_baseline.txt"
ACTIVE = ROOT / ".bb2_active_func"


def collect_warnings(lines: list[str]) -> set[str]:
    """Deduped set of regfix warning bodies (the `regfix: WARNING:` prefix
    stripped, so the body is stable across builds)."""
    out: set[str] = set()
    for ln in lines:
        if WARN_MARKER in ln:
            out.add(ln.split(WARN_MARKER, 1)[1].strip())
    return out


def warned_func(body: str) -> str | None:
    """Best-effort: the function name a warning body refers to (leading
    `<name>:` token), else None."""
    m = re.match(r"([A-Za-z_]\w*):", body)
    return m.group(1) if m else None


def _print_block(items: list[str], header: str, *, benign: bool,
                 active: str = "", note: str = "") -> None:
    bar = "=" * 72
    print()
    print(bar)
    print(f"  {header}" if benign else f"  !! {header} !!")
    print(bar)
    for body in items:
        fn = warned_func(body)
        tag = ""
        if not benign and active and fn and fn != active:
            tag = "   <== SIBLING (not your active function)"
        print(f"  {body}{tag}")
    if note:
        print(f"  ({note})")
    if not benign:
        print()
        print("  These rules went from firing-or-harmless to NOT MATCHING this build.")
        print("  A non-matching regfix rule produces WRONG BYTES with no build error.")
        print("  Most common cause: .L<N> label drift — a sibling function's C body")
        print("  changed GCC's file-wide label numbering.")
        print("      bash tools/dc.sh regfix-drift-immune          # audit + (--apply) fix")
        print("      bash tools/dc.sh verify --clean               # per-function diagnosis")
    print(bar)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--tail", type=int, default=0,
                    help="Suppress live output; print only the last N lines.")
    ap.add_argument("--update-baseline", action="store_true",
                    help="On a MATCHING build, refresh the baseline snapshot.")
    ap.add_argument("--all-warnings", action="store_true",
                    help="Ignore the baseline; show every regfix warning.")
    ap.add_argument("make_args", nargs=argparse.REMAINDER,
                    help="Arguments passed through to make.")
    args = ap.parse_args()

    cmd = ["make"] + args.make_args
    proc = subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1, cwd=str(ROOT),
    )
    lines: list[str] = []
    assert proc.stdout is not None
    for line in proc.stdout:
        lines.append(line.rstrip("\n"))
        if args.tail == 0:
            sys.stdout.write(line)
            sys.stdout.flush()
    proc.wait()
    rc = proc.returncode

    if args.tail > 0:
        for ln in lines[-args.tail:]:
            print(ln)

    full = "\n".join(lines)
    warnings = collect_warnings(lines)
    matched = "bb2 matches!" in full

    # make didn't even complete — the compile/link error is the story; don't
    # muddy it with regfix noise, and leave the baseline alone.
    if rc != 0 and not matched:
        if warnings:
            print(f"\n(make exited {rc}; {len(warnings)} regfix warning(s) not analyzed "
                  f"— fix the build error above first)")
        return rc

    baseline: set[str] = set()
    if BASELINE.exists():
        baseline = {l.strip() for l in BASELINE.read_text(encoding="utf-8").splitlines()
                    if l.strip()}

    if matched:
        # Known-good build: this warning set is benign by definition.
        if args.update_baseline:
            try:
                BASELINE.write_text(
                    "\n".join(sorted(warnings)) + ("\n" if warnings else ""),
                    encoding="utf-8")
                print(f"\n[make_check] build matches — refreshed regfix-warning "
                      f"baseline ({len(warnings)} entries).")
            except Exception as e:  # noqa: BLE001
                print(f"\n[make_check] (could not write baseline: {e})", file=sys.stderr)
        if args.all_warnings and warnings:
            _print_block(sorted(warnings), benign=True,
                         header=f"{len(warnings)} regfix warning(s) "
                                f"(build MATCHES — all benign-in-effect)")
        elif warnings and not args.update_baseline:
            print(f"\n(note: {len(warnings)} regfix rule(s) no-op'd this build; build "
                  f"MATCHES so they are benign-in-effect.)")
        return rc

    # Build completed but the binary does NOT match.
    if args.all_warnings or not baseline:
        suspects = sorted(warnings)
        note = ("no baseline yet — showing ALL (run `dc.sh verify --clean` once "
                "to establish one)" if not baseline else "all (--all-warnings)")
    else:
        suspects = sorted(warnings - baseline)
        note = f"{len(warnings & baseline)} pre-existing benign warning(s) suppressed"

    if not suspects:
        print()
        print("[make_check] build MISMATCHES, but no NEW regfix warnings vs the baseline.")
        print(f"            ({note})")
        print("            The regression is not a freshly-broken regfix rule. Run")
        print("            `bash tools/dc.sh verify --clean` for a per-function diagnosis.")
        return rc

    active = ACTIVE.read_text(encoding="utf-8").strip() if ACTIVE.exists() else ""
    _print_block(suspects, benign=False, active=active, note=note,
                 header=f"{len(suspects)} REGFIX RULE(S) NEWLY BROKEN THIS BUILD")
    return rc


if __name__ == "__main__":
    sys.exit(main())
