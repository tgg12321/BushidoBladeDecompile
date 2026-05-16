#!/usr/bin/env python3
"""Build wrapper that detects cascade-drift symptoms and auto-repairs.

Problem this solves
-------------------
When a new pure-C body is integrated into a busy .c file (text1b.c,
code6cac.c, main.c), GCC's file-wide `.L<N>` label counter shifts for
every subsequent function in that compilation unit. Every sibling
asmfix.txt `rename ".LN" ".L<addr>"` rule and every regfix.txt subst
with a literal `.LN` becomes a tripwire.

Today an agent who hits this sees: assembler errors like "symbol
`.L8006BE00' is already defined" plus warnings like "asmfix: WARNING:
delete_between end did not match in func_X". They have to recognize
the symptom is cascade drift (not their own bug), remember that the
fix is `dc.sh fix-asmfix-drift --apply`, run it, rebuild, and only then
get back to their actual work. The recognize-and-react step is the
friction this tool removes.

What it does
------------
1. Run `make` via the project's `make_check.py` wrapper (which already
   surfaces baselined regfix warnings).
2. Scan the output for drift symptoms:
   - asmfix `WARNING: ... did not match in <func>:`
   - assembler `Error: symbol `.L<N>' is already defined`
   - regfix `WARNING: <func>: ...` not in baseline
3. If drift is detected:
   - Run `tools/fix_asmfix_drift.py --apply` (re-resolves stale `.LN`
     source labels in asmfix.txt rename rules from absolute addresses)
   - Run `tools/fix_label_drift.py --apply` (re-resolves stale `.L<N>`
     refs in regfix.txt subst rules from linker errors)
4. Rebuild once to verify the repair landed.
5. Report what happened with `[auto-repair]` log lines to stderr so the
   agent knows the asmfix.txt / regfix.txt modifications happened and
   need to be included in the commit.

Exit codes
----------
0 - Build clean (with or without auto-repair having fired)
1 - Drift detected but auto-repair was not sufficient
2 - Build failed for non-drift reasons (won't be helped by repair)

Usage
-----
    python3 tools/auto_drift_repair.py            # run make + auto-repair
    python3 tools/auto_drift_repair.py --quiet    # suppress make stdout
    python3 tools/auto_drift_repair.py --no-repair  # detect only

Wired into
----------
    dc.sh build-active   (via build_active.py)
    dc.sh auto-repair    (this script directly)
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASMFIX_TXT = ROOT / "asmfix.txt"
REGFIX_TXT = ROOT / "regfix.txt"

# --- drift-symptom patterns --------------------------------------------------

# asmfix.py emits these when a rename's source label has shifted; the
# delete_between / insert_before then can't find its anchor.
_RE_ASMFIX_DID_NOT_MATCH = re.compile(
    r"asmfix:\s+WARNING:\s+(?:delete_between\s+(?:start|end)|insert_before)\s+did\s+not\s+match\s+in\s+(\w+):"
)
# The assembler emits this when an asmfix-slice's insert_before block (which
# re-defines labels) didn't get its corresponding delete_between to fire.
_RE_DUP_LABEL = re.compile(r"Error:\s+symbol\s+`(\.L[0-9A-Fa-f]+)'\s+is\s+already\s+defined")
# regfix.py emits these when a subst pattern doesn't match at its `@ idx`.
# make_check.py already filters these against a baseline so we only see the
# NEW ones — those are real drift signals.
_RE_REGFIX_NO_MATCH = re.compile(
    r"regfix:\s+WARNING:\s+(\w+):\s+.*?(?:did not match|no match)"
)
# Linker undefined-reference to a `.LN`-style label — the regfix subst's
# replacement label drifted out of existence.
_RE_LD_UNDEF_LABEL = re.compile(r"undefined reference to `(\.L\d+)'")
# SHA1 mismatch: the build linked, the size is right, but bytes differ
# from the original. `make` returns exit 0 in this case (the sha1sum -c
# step prints "MISMATCH" without propagating a failure code), so without
# this signal an SHA1-silent regression looks like a clean build.
#
# This typically means PAIR-SHIFT drift: a regfix subst whose pattern
# AND replacement both hardcode `.L<N>` labels — the pattern (often a
# `\.L\d+` regex) still matches, but the replacement writes a literal
# label that has drifted to a different position. No warning, no error,
# just wrong bytes. The repair tools (fix-asmfix-drift, fix-label-drift)
# can't auto-resolve this class because there's no linker error to
# anchor the lookup; manual `cmp -l` + `dc.sh find-label-at` is needed.
_RE_SHA1_MISMATCH = re.compile(r"MISMATCH:\s+bb2 does not match")


def detect_drift(output: str) -> dict[str, set[str]]:
    """Scan combined build output for cascade-drift symptoms.

    Returns a dict with sets of involved names per signal kind. Empty
    sets across the board = no drift detected.

    `sha1_mismatch` is a sentinel single-element set — there's no per-
    function granularity available from the SHA1 step (build/bb2.exe
    is the whole binary), but using a set keeps the dict shape uniform
    so has_hard_drift / _print_signals don't need a special case.
    """
    return {
        "asmfix_funcs": {m.group(1) for m in _RE_ASMFIX_DID_NOT_MATCH.finditer(output)},
        "dup_labels": {m.group(1) for m in _RE_DUP_LABEL.finditer(output)},
        "regfix_funcs": {m.group(1) for m in _RE_REGFIX_NO_MATCH.finditer(output)},
        "ld_undef_labels": {m.group(1) for m in _RE_LD_UNDEF_LABEL.finditer(output)},
        "sha1_mismatch": {"build/bb2.exe"} if _RE_SHA1_MISMATCH.search(output) else set(),
    }


def has_hard_drift(signals: dict[str, set[str]]) -> bool:
    """High-confidence drift signals: always indicate real cascade.

    `asmfix_funcs`: asmfix.py emits "did not match" only when a rule failed
    to apply (its source `.LN` shifted out from under it).
    `dup_labels`: assembler errors on doubly-defined labels (the asmfix slice
    insert collided with an undeleted original — the `delete_between` half
    failed silently first).
    `ld_undef_labels`: linker can't find a `.LN` that a regfix subst wrote
    into its replacement (the label drifted out of existence).
    `sha1_mismatch`: linked binary differs from the original even though
    make returned 0. The actual byte difference is somewhere — usually
    pair-shift drift (literal label in regfix subst replacement) that
    silently writes wrong bytes with no warning.
    """
    return bool(
        signals["asmfix_funcs"]
        or signals["dup_labels"]
        or signals["ld_undef_labels"]
        or signals["sha1_mismatch"]
    )


def has_any_drift_signal(signals: dict[str, set[str]]) -> bool:
    """Any drift signal at all, including soft (regfix warning) signals.

    `regfix_funcs` is a SOFT signal — `make_check.py` baseline filtering
    shows that ~300 regfix WARNINGs fire on a clean fully-matching build.
    Use it only as a secondary indicator: meaningful when combined with
    a hard signal or a build failure, ignorable on a clean build.
    """
    return has_hard_drift(signals) or bool(signals["regfix_funcs"])


# --- build invocations -------------------------------------------------------

def run_build(quiet: bool = False) -> tuple[int, str]:
    """Run `make_check.py` (which wraps `make`). Returns (rc, combined output).

    make_check provides baselined-regfix-warning filtering so a clean build
    stays clean even if hundreds of dead regfix rules fire warnings.
    """
    cmd = ["python3", "tools/make_check.py"]
    if quiet:
        cmd += ["--tail", "0"]
    r = subprocess.run(
        cmd, capture_output=True, text=True, cwd=str(ROOT), timeout=600,
    )
    return r.returncode, (r.stdout or "") + (r.stderr or "")


def run_repair(name: str, script: str) -> tuple[bool, str]:
    """Run a repair tool with --apply. Return (changes_applied, output).

    Detects whether changes were actually written by looking at the tool's
    output for its FIX/Wrote markers. The tools are intentionally chatty
    when they apply changes and quiet when they don't.
    """
    try:
        r = subprocess.run(
            ["python3", script, "--apply"],
            capture_output=True, text=True, cwd=str(ROOT), timeout=600,
        )
    except subprocess.TimeoutExpired:
        return False, f"{name}: TIMEOUT after 600s"
    output = (r.stdout or "") + (r.stderr or "")
    # Both fix_asmfix_drift and fix_label_drift print "FIX " when applying
    # a rule fix, and fix_label_drift prints "Wrote " when it edits regfix.txt.
    changes = (
        bool(re.search(r"^\s*FIX\b", output, re.MULTILINE))
        or "Wrote " in output
        or "applied" in output.lower()
    )
    return changes, output


def _signal_count(signals: dict[str, set[str]]) -> int:
    """Total drift-signal count across all categories.

    Used by the rollback safety net to decide whether a repair was
    actually productive: if `_signal_count(after) >= _signal_count(before)`
    then the repair did not strictly reduce drift, and any file
    modifications it made are at best ineffective (at worst a
    misdiagnosis like the maspsx-set-noreorder-stripping case where
    fix-label-drift rewrites a regfix rule's label to a different but
    equally-wrong target). The agent is better off seeing the original
    state and the diagnostic recipe than a "fix" that didn't help.
    """
    return (
        len(signals["asmfix_funcs"])
        + len(signals["dup_labels"])
        + len(signals["regfix_funcs"])
        + len(signals["ld_undef_labels"])
        + (1 if signals["sha1_mismatch"] else 0)
    )


def _print_signals(signals: dict[str, set[str]], indent: str = "  ") -> None:
    if signals["asmfix_funcs"]:
        print(f"{indent}asmfix `did not match` in: {sorted(signals['asmfix_funcs'])}",
              file=sys.stderr)
    if signals["dup_labels"]:
        print(f"{indent}doubly-defined labels: {sorted(signals['dup_labels'])}",
              file=sys.stderr)
    if signals["regfix_funcs"]:
        print(f"{indent}regfix warnings in: {sorted(signals['regfix_funcs'])}",
              file=sys.stderr)
    if signals["ld_undef_labels"]:
        print(f"{indent}linker undefined `.LN` refs: {sorted(signals['ld_undef_labels'])}",
              file=sys.stderr)
    if signals["sha1_mismatch"]:
        print(f"{indent}SHA1 mismatch: linked binary differs from original "
              f"(silent — likely pair-shift drift)", file=sys.stderr)


def _scan_unbalanced_set_directives() -> list[tuple[str, int, str]]:
    """Find file-scope `__asm__` blocks with TAB-form `.set` directives
    that are missing SPACE-form duplicates.

    Returns list of (filepath, line_number, directive) tuples. Empty list
    means no problems found.

    Background: maspsx silently strips lines that exactly equal
    `.set\\tnoreorder`, `.set\\treorder`, `.set\\tnoat`, `.set\\tat`
    (the TAB form — see tools/maspsx/maspsx/__init__.py:894). A
    file-scope `__asm__()` block that uses ONLY TAB-form directives
    has all of them stripped before `as` runs, so:
      - The body of the block stays in `.set reorder` mode (the default
        carried over from the previous endlabel), which makes `as`
        auto-fill delay slots inside the asm body — wrong bytes.
      - The directives at the END of the block (e.g. `.set\\treorder`
        to restore reorder mode) are also stripped, so functions AFTER
        the block in the same file stay in `.set noreorder` mode if
        cc1 didn't emit its own per-function `.set noreorder` —
        meaning `as` does NOT auto-fill THEIR delay slots either,
        growing them with unfilled-slot nops.

    The fix is to duplicate every TAB-form `.set` directive with a
    SPACE-form (`.set noreorder` etc.). The SPACE form survives maspsx
    stripping and reaches `as`. See feedback_maspsx_set_noreorder_stripping.md
    for the func_800526A0 incident that motivated this check, and
    inline_asm_canonical.txt convention (e.g. func_8004A76C) for the
    canonical TAB+SPACE-duplicate format.
    """
    src_dir = ROOT / "src"
    if not src_dir.is_dir():
        return []
    issues: list[tuple[str, int, str]] = []
    # Cheap heuristic: a file-scope `__asm__(` followed within ~60 lines
    # by `glabel` indicates a canonical-asm block. We only check those —
    # function-body `__asm__ volatile (...)` blocks don't use these
    # directives.
    block_re = re.compile(
        r'^__asm__\s*\(\s*\n((?:\s*"[^"\n]*"\s*\n){2,80})\s*\)\s*;',
        re.MULTILINE,
    )
    tab_directive = re.compile(r'"\.set\\t(noreorder|noat|reorder|at)\\n"')
    space_directive = re.compile(r'"\.set\s+(noreorder|noat|reorder|at)\\n"')
    glabel_re = re.compile(r'"glabel\s+\w+\\n"')
    for c in sorted(src_dir.glob("*.c")):
        try:
            text = c.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        for m in block_re.finditer(text):
            body = m.group(1)
            if not glabel_re.search(body):
                continue  # not a glabel block — skip
            tab_dirs = {d for d in tab_directive.findall(body)}
            space_dirs = {d for d in space_directive.findall(body)}
            missing = tab_dirs - space_dirs
            if missing:
                # Find the line number of the __asm__( opening in c.
                line_no = text[:m.start()].count("\n") + 1
                for d in sorted(missing):
                    issues.append((str(c.relative_to(ROOT)), line_no, d))
    return issues


def _print_maspsx_stripping_hint(issues: list[tuple[str, int, str]]) -> None:
    """Print a focused hint when maspsx is stripping `.set` directives."""
    print(
        "[auto-repair] HINT: detected file-scope `__asm__()` block(s) with "
        "TAB-form `.set` directives MISSING SPACE-form duplicates:",
        file=sys.stderr,
    )
    for path, line_no, directive in issues[:6]:
        print(
            f"[auto-repair]   {path}:{line_no}  missing SPACE-form "
            f"`.set {directive}` duplicate",
            file=sys.stderr,
        )
    if len(issues) > 6:
        print(f"[auto-repair]   ... and {len(issues) - 6} more", file=sys.stderr)
    print(
        "[auto-repair] maspsx silently strips TAB-form directives (see "
        "tools/maspsx/maspsx/__init__.py:894). When the TAB form is the "
        "only form, the directive never reaches `as`, leaving subsequent "
        "functions in the wrong `.set reorder`/`noreorder` mode. This "
        "causes SHA1 mismatch in functions AFTER the block in the same "
        ".c file (different bytes from unfilled-vs-filled delay slots).",
        file=sys.stderr,
    )
    print(
        "[auto-repair] Fix: for each TAB-form directive, add a matching "
        "SPACE-form duplicate. Reference: src/text1b.c func_8004A76C and "
        "the canonical pattern in feedback_maspsx_set_noreorder_stripping.md.",
        file=sys.stderr,
    )


def _print_pair_shift_diagnostic() -> None:
    """Print the manual recovery recipe for SHA1-silent pair-shift drift.

    Pair-shift = a regfix subst rule whose pattern is drift-immune
    (e.g. `\\.L\\d+` regex) but whose replacement contains a literal
    `.L<N>` that has drifted. No linker error fires — the wrong label
    still exists, just at a different position — so the existing repair
    tools can't help.
    """
    print("[auto-repair] SHA1-silent pair-shift drift — repair tools can't auto-fix",
          file=sys.stderr)
    print("[auto-repair] this class. Recovery recipe:", file=sys.stderr)
    print("[auto-repair]   1. Find diff offset:", file=sys.stderr)
    print("[auto-repair]      cmp -l build/bb2.exe disc/SLUS_006.63 | head", file=sys.stderr)
    print("[auto-repair]   2. Map to function: grep that hex addr in build/bb2.map",
          file=sys.stderr)
    print("[auto-repair]      (the func with the largest <= addr in the .text section)",
          file=sys.stderr)
    print("[auto-repair]   3. Verify which instruction differs:", file=sys.stderr)
    print("[auto-repair]      bash tools/dc.sh verify <func>", file=sys.stderr)
    print("[auto-repair]   4. Find what label is currently at the intended target addr:",
          file=sys.stderr)
    print("[auto-repair]      bash tools/dc.sh find-label-at <func> <intended-addr>",
          file=sys.stderr)
    print("[auto-repair]   5. Update the offending regfix rule's replacement label",
          file=sys.stderr)
    print("[auto-repair]      (grep '<func>.*\\.L<old-N>' regfix.txt to locate it).",
          file=sys.stderr)


def _extract_fix_lines(repair_output: str, limit: int = 12) -> list[str]:
    """Pick the per-rule FIX summary lines from a repair tool's output."""
    fixes = []
    for line in repair_output.splitlines():
        s = line.rstrip()
        if re.match(r"\s*FIX\b", s):
            fixes.append(s.strip())
        elif s.lstrip().startswith("# "):
            # Tool's "# functions with asmfix slice warnings: ..." trace
            fixes.append(s.strip())
        if len(fixes) >= limit:
            fixes.append(f"... ({len(repair_output.splitlines())} more output lines)")
            break
    return fixes


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("--quiet", action="store_true",
                    help="Suppress make output; show only [auto-repair] lines")
    ap.add_argument("--no-repair", action="store_true",
                    help="Detect drift but do not run repair tools")
    args = ap.parse_args()

    print("[auto-repair] Running build...", file=sys.stderr)
    rc, output = run_build(quiet=args.quiet)
    if not args.quiet:
        sys.stdout.write(output)

    signals = detect_drift(output)

    # --- Happy path: clean build, no HARD drift -----------------------------
    # Regfix WARNINGs alone don't trigger repair — they're baseline noise on
    # passing builds (make_check.py reports ~300 of them in normal operation).
    if rc == 0 and not has_hard_drift(signals):
        if signals["regfix_funcs"]:
            n = len(signals["regfix_funcs"])
            print(f"[auto-repair] Build clean. ({n} baseline regfix warnings — "
                  f"normal noise.)", file=sys.stderr)
        else:
            print("[auto-repair] Build clean. No drift detected.", file=sys.stderr)
        return 0

    # --- Build passed but HARD drift signals present (silent cascade) -------
    # Hard drift on a passing build is suspicious — the bytes might be wrong
    # even though the link succeeded. Worth repairing.
    if rc == 0 and has_hard_drift(signals):
        print("[auto-repair] Build OK but HARD drift signals present:",
              file=sys.stderr)
        _print_signals(signals)
        if args.no_repair:
            print("[auto-repair] --no-repair: skipping repair, build still passed.",
                  file=sys.stderr)
            return 0
        print("[auto-repair] Running repair (silent cascade possible)...",
              file=sys.stderr)

    # --- Build failed without ANY drift signals — not our problem ----------
    if rc != 0 and not has_any_drift_signal(signals):
        print(f"[auto-repair] Build failed (rc={rc}) but no drift symptoms detected.",
              file=sys.stderr)
        print("[auto-repair] This looks like a real build error, not cascade drift.",
              file=sys.stderr)
        print("[auto-repair] Auto-repair will not help; investigate the make output above.",
              file=sys.stderr)
        return 2

    # --- Build failed WITH drift signals (any kind) ------------------------
    if rc != 0:
        print("[auto-repair] Build FAILED with drift symptoms:", file=sys.stderr)
        _print_signals(signals)
        if args.no_repair:
            print("[auto-repair] --no-repair: detect-only mode, exiting.",
                  file=sys.stderr)
            return 1

    # --- Run repair tools ---------------------------------------------------
    asmfix_applied = False
    label_applied = False

    # Snapshot rule files BEFORE repair so we can roll back if the post-repair
    # rebuild shows no improvement (false-positive repair). The motivating case
    # is fix-label-drift renaming `.L1076` -> `.L1080` when the actual root
    # cause was elsewhere (maspsx silently stripping a TAB-form
    # `.set noreorder` directive and propagating noreorder mode into
    # subsequent functions, growing them via unfilled delay slots). The
    # renamed rule resolved to a different but equally-wrong byte position;
    # the agent had to detect and revert manually.
    pre_asmfix = ASMFIX_TXT.read_text(encoding="utf-8") if ASMFIX_TXT.exists() else None
    pre_regfix = REGFIX_TXT.read_text(encoding="utf-8") if REGFIX_TXT.exists() else None
    pre_signal_count = _signal_count(signals)

    # fix-asmfix-drift handles asmfix.txt `rename ".LN"` rules. Trigger on
    # asmfix warnings OR duplicate-label errors (the slice-collision signature).
    if signals["asmfix_funcs"] or signals["dup_labels"]:
        print("[auto-repair] Running fix-asmfix-drift --apply ...", file=sys.stderr)
        asmfix_applied, asmfix_out = run_repair("fix-asmfix-drift",
                                                "tools/fix_asmfix_drift.py")
        if asmfix_applied:
            print("[auto-repair]   fix-asmfix-drift applied changes:", file=sys.stderr)
            for line in _extract_fix_lines(asmfix_out):
                print(f"[auto-repair]     {line}", file=sys.stderr)
        else:
            print("[auto-repair]   fix-asmfix-drift made no changes.", file=sys.stderr)

    # fix-label-drift handles regfix.txt subst replacement labels. Trigger on
    # regfix warnings OR linker undefined `.LN` (label drifted out).
    if signals["regfix_funcs"] or signals["ld_undef_labels"]:
        print("[auto-repair] Running fix-label-drift --apply ...", file=sys.stderr)
        label_applied, label_out = run_repair("fix-label-drift",
                                              "tools/fix_label_drift.py")
        if label_applied:
            print("[auto-repair]   fix-label-drift applied changes:", file=sys.stderr)
            for line in _extract_fix_lines(label_out):
                print(f"[auto-repair]     {line}", file=sys.stderr)
        else:
            print("[auto-repair]   fix-label-drift made no changes.", file=sys.stderr)

    if not (asmfix_applied or label_applied):
        # Special case: when SHA1 mismatch is the ONLY signal (no warnings,
        # no linker errors), the repair tools have nothing to anchor on.
        # This is the pair-shift class — needs the manual recovery recipe.
        sha1_only = (
            signals["sha1_mismatch"]
            and not signals["asmfix_funcs"]
            and not signals["dup_labels"]
            and not signals["ld_undef_labels"]
        )
        if sha1_only:
            stripping_issues = _scan_unbalanced_set_directives()
            if stripping_issues:
                _print_maspsx_stripping_hint(stripping_issues)
            _print_pair_shift_diagnostic()
            return 1

        print("[auto-repair] No repairs applied — drift symptoms present but the",
              file=sys.stderr)
        print("              repair tools could not resolve them. This is a",
              file=sys.stderr)
        print("              different cascade class; manual investigation needed:",
              file=sys.stderr)
        if signals["asmfix_funcs"]:
            example_func = sorted(signals["asmfix_funcs"])[0]
            print(f"              bash tools/dc.sh diff-align {example_func}",
                  file=sys.stderr)
        if signals["sha1_mismatch"]:
            print("              (SHA1 mismatch also present — see pair-shift recipe below)",
                  file=sys.stderr)
            _print_pair_shift_diagnostic()
        return 1

    # --- Final rebuild after repair ----------------------------------------
    print("[auto-repair] Rebuilding to verify repair...", file=sys.stderr)
    rc, output = run_build(quiet=args.quiet)
    if not args.quiet:
        sys.stdout.write(output)

    final_signals = detect_drift(output)

    # --- Rollback safety net ------------------------------------------------
    # If repair modified asmfix.txt / regfix.txt but the post-repair drift
    # signals did NOT strictly improve, the repair was likely a misdiagnosis.
    # Revert the files and surface the true symptoms instead of leaving the
    # bogus rewrite in place for the agent to chase. The classic case is
    # fix-label-drift renaming a `.L<N>` label in a regfix subst rule when
    # the actual problem was upstream (e.g. maspsx-directive stripping).
    post_signal_count = _signal_count(final_signals)
    repair_modified_files = asmfix_applied or label_applied
    repair_did_not_help = (
        rc != 0 or post_signal_count >= pre_signal_count
    )
    if repair_modified_files and repair_did_not_help:
        reverted: list[str] = []
        if asmfix_applied and pre_asmfix is not None:
            ASMFIX_TXT.write_text(pre_asmfix, encoding="utf-8", newline="\n")
            reverted.append("asmfix.txt")
        if label_applied and pre_regfix is not None:
            REGFIX_TXT.write_text(pre_regfix, encoding="utf-8", newline="\n")
            reverted.append("regfix.txt")
        if reverted:
            print(
                f"[auto-repair] ROLLBACK: post-repair drift signals did not "
                f"improve (pre={pre_signal_count}, post={post_signal_count}) — "
                f"reverted {' + '.join(reverted)} to pre-repair state.",
                file=sys.stderr,
            )
            print(
                "[auto-repair] The repair was a likely misdiagnosis. Common "
                "causes: file-scope `__asm__` block with TAB-form `.set "
                "noreorder` directives stripped by maspsx (add SPACE-form "
                "duplicates — see feedback_maspsx_set_noreorder_stripping.md), "
                "or a real source-level bug that looks like cascade drift.",
                file=sys.stderr,
            )
            # Re-run the build one more time so the on-disk binary reflects
            # the reverted-rule state, not the bogus-rewrite state, and
            # update the signals dict so the downstream branches see the
            # original symptoms (not the partially-repaired ones).
            print(
                "[auto-repair] Rebuilding with reverted rules for clean "
                "diagnostic output...",
                file=sys.stderr,
            )
            rc, output = run_build(quiet=args.quiet)
            if not args.quiet:
                sys.stdout.write(output)
            final_signals = detect_drift(output)
            asmfix_applied = False
            label_applied = False
    # When the build passes after repair, treat regfix WARNINGs as baseline
    # noise — make_check.py reports ~300 of them on a clean fully-matching
    # build, none of which are drift. The high-signal "is the cascade fixed?"
    # criteria are: build rc==0, no doubly-defined labels, no asmfix
    # did-not-match warnings, and no SHA1 mismatch (the last one catches
    # silent pair-shift drift that the repair tools can't address).
    hard_residuals = bool(
        final_signals["dup_labels"]
        or final_signals["asmfix_funcs"]
        or final_signals["sha1_mismatch"]
    )
    if rc == 0 and not hard_residuals:
        print("[auto-repair] Build clean after repair.", file=sys.stderr)
        modified = []
        if asmfix_applied:
            modified.append("asmfix.txt")
        if label_applied:
            modified.append("regfix.txt")
        if modified:
            print(f"[auto-repair] NOTE: modified {' + '.join(modified)} — "
                  f"include in your commit.", file=sys.stderr)
        # Show baseline regfix warnings as informational, not alarming.
        if final_signals["regfix_funcs"]:
            n = len(final_signals["regfix_funcs"])
            print(f"[auto-repair] ({n} baseline regfix warnings present — "
                  f"normal, see make_check.py for filtering)", file=sys.stderr)
        return 0

    if rc == 0 and hard_residuals:
        # SHA1-only residual after repair = pair-shift drift the repair
        # tools couldn't catch. Show the manual recipe.
        sha1_only_residual = (
            final_signals["sha1_mismatch"]
            and not final_signals["asmfix_funcs"]
            and not final_signals["dup_labels"]
        )
        if sha1_only_residual:
            print("[auto-repair] Repair landed but SHA1 still mismatches — silent",
                  file=sys.stderr)
            print("              pair-shift drift remains (a different rule from",
                  file=sys.stderr)
            print("              what repair tools handle).", file=sys.stderr)
            stripping_issues = _scan_unbalanced_set_directives()
            if stripping_issues:
                _print_maspsx_stripping_hint(stripping_issues)
            _print_pair_shift_diagnostic()
            return 1

        print("[auto-repair] Build OK but HARD residual drift signals remain:",
              file=sys.stderr)
        if final_signals["asmfix_funcs"]:
            print(f"  asmfix `did not match`: {sorted(final_signals['asmfix_funcs'])}",
                  file=sys.stderr)
        if final_signals["dup_labels"]:
            print(f"  doubly-defined labels: {sorted(final_signals['dup_labels'])}",
                  file=sys.stderr)
        print("[auto-repair] These usually mean a second cascade cycle is needed;",
              file=sys.stderr)
        print("              try running `dc.sh auto-repair` again.", file=sys.stderr)
        return 0  # build works, warn but allow

    print(f"[auto-repair] Build still FAILING (rc={rc}) after repair.",
          file=sys.stderr)
    print("[auto-repair] Auto-repair was not sufficient.", file=sys.stderr)
    if any(final_signals.values()):
        print("[auto-repair] Residual signals:", file=sys.stderr)
        _print_signals(final_signals)
    print("[auto-repair] Inspect the make output above and run diagnostics manually.",
          file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
