#!/usr/bin/env python3
"""
inspect_ndata.py - Inspect / extract entries from the BB2 NDATA archive.

Bushido Blade 2 packs most of its bulk data (motion files, stage geometry,
textures, audio sample banks) into a single archive `NDATA.DAT` indexed by a
flat directory `NDATA.INF`. See docs/formats/NDATA.md for the full format
specification.

This tool exposes that archive:

    inspect_ndata.py <NDATA.INF> --list                     List every entry
    inspect_ndata.py <NDATA.INF> --extract <ID> [--out F]   Extract one entry
    inspect_ndata.py <NDATA.INF> --hex <ID> [--bytes N]     Hex preview first N bytes
    inspect_ndata.py <NDATA.INF> --dump-all <OUTDIR>        Extract all entries
    inspect_ndata.py <NDATA.INF> --stats                    Summary statistics

The NDATA.DAT file is auto-located next to the INF (same dir, `.DAT` extension)
unless `--dat <path>` is given.
"""

import argparse
import os
import struct
import sys
from pathlib import Path

SECTOR_SIZE = 0x800
SENTINEL = 0xFFFF


def load_directory(inf_path):
    """Parse NDATA.INF into a list of (start_sector, length_sectors) tuples."""
    with open(inf_path, "rb") as f:
        data = f.read()
    if len(data) % 4 != 0:
        raise ValueError(f"INF length {len(data)} not multiple of 4")
    entries = []
    for i in range(len(data) // 4):
        start, length = struct.unpack_from("<HH", data, i * 4)
        entries.append((start, length))
    return entries


def is_sentinel(entry):
    s, l = entry
    return s == SENTINEL and l == SENTINEL


def find_dat(inf_path, override=None):
    """Locate NDATA.DAT next to the given INF (or honour override)."""
    if override:
        p = Path(override)
        if not p.is_file():
            raise FileNotFoundError(override)
        return p
    inf = Path(inf_path)
    # Try same dir, swap .INF -> .DAT
    candidates = [
        inf.with_suffix(".DAT"),
        inf.with_suffix(".dat"),
        inf.parent / "NDATA.DAT",
    ]
    for c in candidates:
        if c.is_file():
            return c
    raise FileNotFoundError(
        f"Could not find NDATA.DAT next to {inf}; pass --dat to specify"
    )


def preview_label(blob):
    """Heuristic one-line summary of an entry's contents."""
    if len(blob) < 4:
        return "(empty)"
    head = blob[:4]
    # Common signatures
    if head == b"MC\x00\x0b":
        return "BBM (common motion bundle)"
    if head == b"MW\x00\x0b":
        return "BBM (character motion bundle)"
    if head[:4] == b"PS-X":
        return "PSX EXE"
    if blob[:8].endswith(b"pBAV") or blob[4:8] == b"pBAV":
        return "VAB header"
    if blob[:4] == b"\x10\x00\x00\x00":
        return "TIM image"
    # Try to detect a "small TOC" pattern: u32 count, u32 header_size, then
    # ascending u32 offsets
    try:
        count, hdr = struct.unpack_from("<II", blob, 0)
    except struct.error:
        return "(<8 bytes)"
    if 1 <= count <= 64 and 8 <= hdr <= 0x400 and 8 + count * 4 <= hdr:
        offs = []
        for i in range(count):
            o = struct.unpack_from("<I", blob, 8 + i * 4)[0]
            offs.append(o)
        if all(a < b for a, b in zip(offs, offs[1:])):
            return f"TOC: count={count} hdr=0x{hdr:x}"
    return f"data ({blob[:4].hex()} ...)"


def cmd_list(args, entries, dat_path):
    with open(dat_path, "rb") as f:
        for i, (s, l) in enumerate(entries):
            if is_sentinel((s, l)):
                if args.show_sentinels:
                    print(f"  [{i:4d}]  -- SENTINEL --")
                continue
            offset = s * SECTOR_SIZE
            length = l * SECTOR_SIZE
            f.seek(offset)
            head = f.read(min(64, length))
            label = preview_label(head)
            print(
                f"  [{i:4d}]  sec={s:5d} len={l:4d}  "
                f"bytes={length:8d}  off=0x{offset:08x}  {label}"
            )


def cmd_extract(args, entries, dat_path):
    idx = args.extract
    if idx < 0 or idx >= len(entries):
        sys.exit(f"entry id {idx} out of range (0..{len(entries)-1})")
    s, l = entries[idx]
    if is_sentinel((s, l)):
        sys.exit(f"entry {idx} is a sentinel — nothing to extract")
    with open(dat_path, "rb") as f:
        f.seek(s * SECTOR_SIZE)
        blob = f.read(l * SECTOR_SIZE)
    out = args.out or f"ndata_{idx:04d}.bin"
    with open(out, "wb") as g:
        g.write(blob)
    print(
        f"Wrote {out}  (entry {idx}: start_sector={s} len_sectors={l} "
        f"bytes={len(blob)})"
    )


def cmd_hex(args, entries, dat_path):
    idx = args.hex
    if idx < 0 or idx >= len(entries):
        sys.exit(f"entry id {idx} out of range")
    s, l = entries[idx]
    if is_sentinel((s, l)):
        sys.exit(f"entry {idx} is a sentinel")
    n = args.bytes
    with open(dat_path, "rb") as f:
        f.seek(s * SECTOR_SIZE)
        blob = f.read(min(n, l * SECTOR_SIZE))
    # Plain hex dump in 16-byte rows
    print(
        f"Entry {idx}: start_sector={s} len_sectors={l} bytes={l*SECTOR_SIZE} "
        f"(showing first {len(blob)})"
    )
    for off in range(0, len(blob), 16):
        chunk = blob[off : off + 16]
        hex_part = " ".join(f"{b:02x}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        print(f"  {off:08x}: {hex_part:<48s}  {ascii_part}")


def cmd_dump_all(args, entries, dat_path):
    out_dir = Path(args.dump_all)
    out_dir.mkdir(parents=True, exist_ok=True)
    written = 0
    skipped = 0
    with open(dat_path, "rb") as f:
        for i, (s, l) in enumerate(entries):
            if is_sentinel((s, l)):
                skipped += 1
                continue
            f.seek(s * SECTOR_SIZE)
            blob = f.read(l * SECTOR_SIZE)
            (out_dir / f"ndata_{i:04d}.bin").write_bytes(blob)
            written += 1
    print(
        f"Wrote {written} entries to {out_dir} "
        f"({skipped} sentinel slots skipped)"
    )


def cmd_stats(args, entries, dat_path):
    total = len(entries)
    sentinels = sum(1 for e in entries if is_sentinel(e))
    active = total - sentinels
    sectors = sum(l for s, l in entries if not is_sentinel((s, l)))
    dat_size = os.path.getsize(dat_path)
    lengths = sorted(l for s, l in entries if not is_sentinel((s, l)))
    print(f"NDATA.INF directory:")
    print(f"  total entries:     {total}")
    print(f"  active entries:    {active}")
    print(f"  sentinel entries:  {sentinels}")
    print(f"  total used sectors: {sectors} ({sectors * SECTOR_SIZE} bytes)")
    print(f"NDATA.DAT:")
    print(f"  size: {dat_size} bytes ({dat_size / SECTOR_SIZE:.1f} sectors)")
    if lengths:
        med = lengths[len(lengths) // 2]
        print(f"  entry-length distribution (sectors):")
        print(f"    min={lengths[0]}  med={med}  max={lengths[-1]}  mean={sum(lengths)/len(lengths):.1f}")

    # Walk in INF order, see how far the cumulative-sum stays continuous
    running = 0
    last_continuous = -1
    for i, (s, l) in enumerate(entries):
        if is_sentinel((s, l)):
            continue
        if s != running:
            break
        last_continuous = i
        running = s + l
    print(
        f"  last contiguous valid entry: {last_continuous} "
        f"(running total = sector 0x{running:x})"
    )


def main():
    ap = argparse.ArgumentParser(
        description="Inspect/extract BB2 NDATA archive entries"
    )
    ap.add_argument("inf", help="Path to NDATA.INF")
    ap.add_argument("--dat", help="Path to NDATA.DAT (default: auto-locate)")
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--list", action="store_true",
                   help="List every entry with preview")
    g.add_argument("--extract", type=int, metavar="ID",
                   help="Extract single entry to disk")
    g.add_argument("--hex", type=int, metavar="ID",
                   help="Hex-dump start of single entry")
    g.add_argument("--dump-all", metavar="OUTDIR",
                   help="Extract every non-sentinel entry to OUTDIR")
    g.add_argument("--stats", action="store_true",
                   help="Print directory summary statistics")
    ap.add_argument("--out", metavar="FILE",
                    help="Output filename for --extract (default ndata_NNNN.bin)")
    ap.add_argument("--bytes", type=int, default=128, metavar="N",
                    help="Bytes to show for --hex (default 128)")
    ap.add_argument("--show-sentinels", action="store_true",
                    help="Include sentinel rows in --list output")

    args = ap.parse_args()

    entries = load_directory(args.inf)
    dat_path = find_dat(args.inf, args.dat)

    if args.list:
        cmd_list(args, entries, dat_path)
    elif args.extract is not None:
        cmd_extract(args, entries, dat_path)
    elif args.hex is not None:
        cmd_hex(args, entries, dat_path)
    elif args.dump_all:
        cmd_dump_all(args, entries, dat_path)
    elif args.stats:
        cmd_stats(args, entries, dat_path)


if __name__ == "__main__":
    main()
