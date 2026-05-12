#!/usr/bin/env python3
"""Incremental build for one function: recompile the .c containing it,
relink, and run bridge-aware verify. Cheap iteration during retirement
work — typically ~30 seconds vs ~2 minutes for a full clean rebuild.

Safety: the full clean-rebuild gate at commit time (active_func_guard
hook) still catches regressions. This tool is for the development loop
between edit and commit, not for confirming a final match.

Usage:
    python3 tools/build_active.py <func> [--no-verify] [--quiet]

What happens:
    1. Look up which src/*.c file defines <func> (or for inline-asm
       blocks, contains its `glabel <func>` line).
    2. `rm -f build/src/<that-file>.o build/bb2.elf build/bb2.exe build/bb2.bin build/bb2.map`
    3. `make` (incrementally — only rebuilds the deleted .o and relinks)
    4. Run `bash tools/dc.sh verify-c <func>` (the bridge-aware verify)

If <func> is bridged in asmfix.txt (active `replace_with_asmfile`), the
verify-c step refuses with a clear message — you need to call
`dc.sh retire <func>` first.

Exit codes:
    0 = build succeeded AND verify-c reported MATCH
    1 = build succeeded but verify-c reports diffs (or refused due to bridge)
    2 = build failed
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
BUILD = ROOT / "build"

# Match the C function definition OR a `glabel <name>` inside an inline-asm
# block. Either form means the function lives in this .c file.
FUNC_DEF_RE = re.compile(r"^[A-Za-z_][\w *]*?\s\**(\w+)\s*\([^)]*\)\s*\{", re.MULTILINE)
GLABEL_RE = re.compile(r"glabel\s+(\w+)")


# Inline-asm load/store/coproc ops with a hex offset like `swc2 $11, 0x10($a1)`
# silently break maspsx (raises `invalid literal for int() with base 10: '0x10'`).
# Maspsx requires decimal offsets in inline-asm operand strings. Catch this
# before invoking make so the agent gets a clear message instead of a noisy
# build failure.
_HEX_OFFSET_OP = (
    r"\b(?:lw|sw|lh|sh|lhu|lb|sb|lbu|lwc2|swc2|lwl|lwr|swl|swr|"
    r"ll|sc|cache)\b\s+[^,]+,\s*"
)
_INLINE_ASM_HEX_OFFSET_RE = re.compile(
    r'__asm__\s+volatile\s*\(\s*"' + _HEX_OFFSET_OP + r'(0x[0-9a-fA-F]+)\s*\(',
)


def check_hex_offsets(src_path: Path, func: str) -> list[tuple[int, str]]:
    """Scan <func>'s body in src_path for inline-asm load/store ops with hex
    offsets. Returns a list of (lineno, snippet) violations. Empty list = clean.

    Maspsx parses the offset as decimal-only; `0x10` triggers
    `invalid literal for int() with base 10: '0x10'` and breaks the build.
    """
    try:
        text = src_path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return []
    lines = text.splitlines()
    # Find <func>'s body bounds: opening { ... matching closing }
    func_def_re = re.compile(
        rf"^[A-Za-z_][\w *]*?\s\**{re.escape(func)}\s*\([^)]*\)\s*\{{",
        re.MULTILINE,
    )
    m = func_def_re.search(text)
    if not m:
        return []
    # Brace-balance scan from the opening brace
    start_line = text.count("\n", 0, m.start()) + 1
    depth = 0
    end_offset = len(text)
    for i in range(m.end() - 1, len(text)):
        c = text[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                end_offset = i + 1
                break
    end_line = text.count("\n", 0, end_offset) + 1
    body_lines = lines[start_line - 1 : end_line]
    out: list[tuple[int, str]] = []
    for offset, line in enumerate(body_lines):
        if _INLINE_ASM_HEX_OFFSET_RE.search(line):
            out.append((start_line + offset, line.strip()))
    return out


def find_src_for_func(func: str) -> Path | None:
    """Return the src/*.c file that defines <func>, or None."""
    for p in sorted(SRC.glob("*.c")):
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        for m in FUNC_DEF_RE.finditer(text):
            if m.group(1) == func:
                return p
        for m in GLABEL_RE.finditer(text):
            if m.group(1) == func:
                return p
    return None


def run(cmd: list[str], quiet: bool = False) -> int:
    """Run a command, streaming output unless quiet."""
    if quiet:
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=str(ROOT))
        # Show only the last 3 lines on stderr in quiet mode (matches `make` tail).
        tail = "\n".join((result.stdout + result.stderr).splitlines()[-3:])
        if tail.strip():
            print(tail)
        return result.returncode
    return subprocess.run(cmd, cwd=str(ROOT)).returncode


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--no-verify", action="store_true",
                    help="Skip the verify-c step. Just rebuild.")
    ap.add_argument("--quiet", action="store_true",
                    help="Suppress make output except for the tail summary.")
    args = ap.parse_args()

    src_path = find_src_for_func(args.func)
    if src_path is None:
        print(f"ERROR: could not find {args.func} in any src/*.c file", file=sys.stderr)
        return 2

    rel = src_path.relative_to(ROOT)
    obj = BUILD / rel.with_suffix(".o")

    # Pre-flight: catch inline-asm hex offsets that maspsx rejects.
    hex_violations = check_hex_offsets(src_path, args.func)
    if hex_violations:
        print(
            f"[build-active] ABORT: {len(hex_violations)} inline-asm hex offset(s) "
            f"in {args.func} (maspsx rejects these — use decimal):",
            file=sys.stderr,
        )
        for lineno, snippet in hex_violations:
            print(f"  {rel}:{lineno}: {snippet}", file=sys.stderr)
        print(
            "  Fix: convert each `0xN` offset to decimal `N` in the inline-asm "
            "operand string. Example: `swc2 $11, 0x10($a1)` -> `swc2 $11, 16($a1)`.",
            file=sys.stderr,
        )
        return 2

    # Delete the .o for the changed file + the link products. Make's
    # dependency tracking will rebuild what's needed.
    targets = [
        obj,
        BUILD / "bb2.elf",
        BUILD / "bb2.bin",
        BUILD / "bb2.exe",
        BUILD / "bb2.map",
    ]
    for t in targets:
        try:
            t.unlink()
        except FileNotFoundError:
            pass
        except Exception as e:
            print(f"WARNING: could not remove {t}: {e}", file=sys.stderr)

    print(f"[build-active] Recompiling {rel} and relinking...")
    rc = run(["make"], quiet=args.quiet)
    if rc != 0:
        print(f"[build-active] make FAILED (exit {rc})", file=sys.stderr)
        return 2

    if args.no_verify:
        return 0

    print()
    print(f"[build-active] Running verify-c for {args.func}...")
    # Capture verify-c output so we can extract the diff count for the
    # iteration log (and still stream it to the user).
    proc = subprocess.run(
        ["bash", "tools/dc.sh", "verify-c", args.func],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
    )
    out_text = (proc.stdout or "") + (proc.stderr or "")
    sys.stdout.write(proc.stdout or "")
    sys.stderr.write(proc.stderr or "")
    rc = proc.returncode

    # Parse the verify-c output for "MATCH (0 diffs in N bytes)" or
    # "N instruction(s) differ (... K bytes)" and record to iter_log.
    diffs = None
    bytes_off = None
    matched = False
    sha1_ok = False
    m_match = re.search(r"MATCH \((\d+) diffs in (\d+) bytes\)", out_text)
    m_differ = re.search(r": (\d+) instruction\(s\) differ.*?(\d+) bytes\)", out_text)
    if m_match:
        diffs = int(m_match.group(1))
        matched = (diffs == 0)
    elif m_differ:
        diffs = int(m_differ.group(1))
        # Try to compute bytes_off from "function at 0xADDR, N bytes" vs target.
        # We don't have target size here; leave bytes_off as None unless we can
        # find it (the binary verify output prints "X bytes" = mine's size).
    # Try to detect SHA1 match from the make tail (the verify --all output
    # in build-active doesn't run, but a successful incremental build that
    # then matches one function doesn't imply whole-binary SHA1. Keep it
    # conservative: only set sha1_ok when verify-c reports MATCH AND the
    # build succeeded.)
    sha1_ok = matched

    try:
        from iter_log import record as iter_record, plateau_check, render_trajectory, render_plateau_advice  # noqa: E402
        iter_record(args.func, diffs, bytes_off, matched, sha1_ok)
        traj = render_trajectory(args.func, last=5)
        if traj:
            print()
            print(f"[build-active] Trajectory for {args.func} (last 5 rounds):")
            print(traj)
        check = plateau_check(args.func)
        advice = render_plateau_advice(check)
        if advice.strip():
            print()
            print(advice.rstrip())
    except Exception as e:
        print(f"[build-active] (iter_log warning: {e})", file=sys.stderr)

    return 0 if rc == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
