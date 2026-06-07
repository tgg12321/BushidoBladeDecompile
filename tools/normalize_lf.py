#!/usr/bin/env python3
"""Normalize a build-critical file's line endings to LF (strip CR bytes).

The Windows Edit/Write tool can silently convert LF -> CRLF when writing some
build-critical files (regfix.txt, asmfix.txt, src/*.c, etc.), which breaks the
GNU toolchain and bash. The PostToolUse tooling_error_guard hook catches this
post-write; this script is the canonical remediation pointed to by the
crlf-build-file signature's permanent_fix.

Usage:
    python3 tools/normalize_lf.py <path> [<path> ...]

Idempotent and safe to run on already-LF files.
"""
import sys
from pathlib import Path


def normalize(path: str) -> bool:
    """Strip CR bytes from `path`. Returns True if the file was modified."""
    p = Path(path)
    data = p.read_bytes()
    if b"\r" not in data:
        return False
    # CRLF -> LF, then any stray CR -> nothing (defensive).
    new = data.replace(b"\r\n", b"\n").replace(b"\r", b"")
    p.write_bytes(new)
    return True


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print("usage: normalize_lf.py <path> [<path> ...]", file=sys.stderr)
        return 2
    rc = 0
    for path in argv[1:]:
        try:
            changed = normalize(path)
        except OSError as exc:
            print(f"{path}: {exc}", file=sys.stderr)
            rc = 1
            continue
        if changed:
            print(f"{path}: normalized (stripped CR bytes)")
        else:
            print(f"{path}: already LF")
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv))
