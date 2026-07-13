#!/usr/bin/env python3
"""
inspect_bbm.py - Parse Bushido Blade 2 BBM motion bundles.

A BBM file is the per-character motion package used by Lightweight's
"Marionation" engine. See docs/formats/BBM.md for the full format.

Usage:
    inspect_bbm.py FILE.BBM                     summary table for one file
    inspect_bbm.py FILE.BBM --hex N             hex preview of section N (1..5)
    inspect_bbm.py FILE.BBM --dump N --out OUT.bin  dump raw section N to file
    inspect_bbm.py DIR/                         summarise every BBM in a directory
"""

import argparse
import os
import struct
import sys
from pathlib import Path

MAGICS = {
    b"MC\x00\x0b": "common motion bundle",
    b"MW\x00\x0b": "character motion bundle",
}


def parse_header(path):
    """Return dict with magic, version, section_ranges (list of (start, end))."""
    size = os.path.getsize(path)
    if size < 0x14:
        raise ValueError(f"{path}: file too small ({size} bytes)")
    with open(path, "rb") as f:
        hdr = f.read(20)
    magic = hdr[:4]
    if magic[:2] not in (b"MC", b"MW"):
        raise ValueError(
            f"{path}: not a BBM (first bytes {magic[:2]!r} not MC or MW)"
        )
    version = struct.unpack_from("<H", hdr, 2)[0]
    e1, e2, e3, e4 = struct.unpack_from("<IIII", hdr, 4)
    # Validate offsets
    for label, off in (("end1", e1), ("end2", e2), ("end3", e3), ("end4", e4)):
        if off > size:
            raise ValueError(
                f"{path}: header offset {label}=0x{off:x} past EOF (size=0x{size:x})"
            )
    sections = [
        (0x14, e1),  # section 1
        (e1, e2),    # section 2
        (e2, e3),    # section 3
        (e3, e4),    # section 4
        (e4, size),  # section 5 (bulk)
    ]
    for i, (s, e) in enumerate(sections, 1):
        if e < s:
            raise ValueError(
                f"{path}: section {i} ends ({e}) before it starts ({s})"
            )
    return {
        "size": size,
        "magic": magic,
        "magic_desc": MAGICS.get(magic, "unknown"),
        "version": version,
        "sections": sections,
    }


def cmd_summary(path):
    info = parse_header(path)
    name = Path(path).name
    print(f"{name}  ({info['size']} bytes)")
    print(f"  magic    : {info['magic']!r}  ({info['magic_desc']})")
    print(f"  version  : 0x{info['version']:04x}")
    print("  sections :")
    for i, (s, e) in enumerate(info["sections"], 1):
        sz = e - s
        if sz == 0:
            note = "EMPTY"
        elif i == 1:
            note = "joint default-pose / skeleton parameter table"
        elif i == 2:
            note = "short parameter / index block"
        elif i == 3:
            note = "animation segment descriptor table"
        elif i == 4:
            note = "auxiliary segment data"
        elif i == 5:
            note = "bulk packed keyframe payload"
        print(
            f"    [{i}]  0x{s:08x}..0x{e:08x}  ({sz} bytes)  {note}"
        )


def cmd_hex(path, sec_index, n=128):
    info = parse_header(path)
    if not (1 <= sec_index <= 5):
        sys.exit(f"--hex section index must be 1..5, got {sec_index}")
    s, e = info["sections"][sec_index - 1]
    sz = e - s
    if sz == 0:
        print(f"Section {sec_index} is empty.")
        return
    with open(path, "rb") as f:
        f.seek(s)
        blob = f.read(min(n, sz))
    print(
        f"Section {sec_index} of {Path(path).name}: "
        f"offset 0x{s:x} size {sz}, showing first {len(blob)} bytes"
    )
    for off in range(0, len(blob), 16):
        chunk = blob[off : off + 16]
        hex_part = " ".join(f"{b:02x}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        print(f"  {off:08x}: {hex_part:<48s}  {ascii_part}")


def cmd_dump(path, sec_index, out_path):
    info = parse_header(path)
    if not (1 <= sec_index <= 5):
        sys.exit(f"--dump section index must be 1..5, got {sec_index}")
    s, e = info["sections"][sec_index - 1]
    sz = e - s
    with open(path, "rb") as f:
        f.seek(s)
        blob = f.read(sz)
    with open(out_path, "wb") as g:
        g.write(blob)
    print(
        f"Wrote {out_path}  ({sz} bytes: section {sec_index} of "
        f"{Path(path).name})"
    )


def cmd_dir_summary(dirpath):
    p = Path(dirpath)
    # On case-insensitive filesystems (Windows/macOS) *.BBM and *.bbm match
    # the same files; dedupe before printing.
    files = sorted({f.resolve() for f in p.glob("*.BBM")} |
                   {f.resolve() for f in p.glob("*.bbm")})
    if not files:
        print(f"No *.BBM files found in {dirpath}")
        return
    print(
        f"{'name':<14}{'size':>8}  {'sec1':>7} {'sec2':>5} {'sec3':>5} "
        f"{'sec4':>5} {'sec5(bulk)':>12}  type"
    )
    for fpath in files:
        try:
            info = parse_header(fpath)
        except ValueError as e:
            print(f"{fpath.name:<14}  ERROR: {e}")
            continue
        sizes = [e_ - s_ for s_, e_ in info["sections"]]
        magic = info["magic"][:2].decode("ascii")
        print(
            f"{fpath.name:<14}{info['size']:>8}  "
            f"{sizes[0]:>7} {sizes[1]:>5} {sizes[2]:>5} {sizes[3]:>5} "
            f"{sizes[4]:>12}  {magic}"
        )


def main():
    ap = argparse.ArgumentParser(description="Inspect BB2 BBM motion bundles")
    ap.add_argument("target", help="BBM file or directory of BBM files")
    ap.add_argument("--hex", type=int, metavar="N", help="Hex-dump section N (1..5)")
    ap.add_argument("--bytes", type=int, default=128,
                    help="Bytes to show for --hex (default 128)")
    ap.add_argument("--dump", type=int, metavar="N",
                    help="Dump section N (1..5) to file (give OUT path next)")
    ap.add_argument("--out", help="Output file for --dump")
    args = ap.parse_args()

    p = Path(args.target)
    if p.is_dir():
        cmd_dir_summary(p)
        return

    if not p.is_file():
        sys.exit(f"{args.target}: not a file or directory")

    if args.dump is not None:
        if not args.out:
            sys.exit("--dump requires --out OUT_FILE")
        cmd_dump(p, args.dump, args.out)
        return

    if args.hex is not None:
        cmd_hex(p, args.hex, args.bytes)
        return

    cmd_summary(p)


if __name__ == "__main__":
    main()
