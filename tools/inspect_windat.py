#!/usr/bin/env python3
"""
inspect_windat.py - Inspect disc/MOTION/WIN.DAT (victory-pose data table).

WIN.DAT is a fixed-stride table of 27 records, each 0x3800 (14336) bytes,
holding 16-bit signed fields used by the victory-animation system. See
docs/formats/BBM.md "WIN.DAT" section for the format details.

Usage:
    inspect_windat.py disc/MOTION/WIN.DAT                summary table
    inspect_windat.py disc/MOTION/WIN.DAT --rec N        hex preview of one record
    inspect_windat.py disc/MOTION/WIN.DAT --dump OUTDIR  extract every record as a .bin
    inspect_windat.py disc/MOTION/WIN.DAT --csv          dump all i16 values as CSV
"""

import argparse
import os
import struct
import sys

RECORD_SIZE = 0x3800
USED_BYTES = 0x30F0


def parse(path):
    with open(path, "rb") as f:
        data = f.read()
    if len(data) % RECORD_SIZE:
        raise ValueError(
            f"{path}: size {len(data)} (0x{len(data):x}) is not a multiple of "
            f"0x{RECORD_SIZE:x} — not a WIN.DAT file?"
        )
    return data


def record_stats(rec):
    used = len(rec) - 1
    while used >= 0 and rec[used] == 0:
        used -= 1
    used += 1
    n_i16 = used // 2
    if n_i16 == 0:
        return {"used": 0, "min": 0, "max": 0, "n_i16": 0}
    vals = struct.unpack_from(f"<{n_i16}h", rec, 0)
    return {
        "used": used,
        "min": min(vals),
        "max": max(vals),
        "n_i16": n_i16,
    }


def summary(path):
    data = parse(path)
    nrecs = len(data) // RECORD_SIZE
    print(f"{path}")
    print(f"  size            : {len(data)} bytes (0x{len(data):x})")
    print(f"  records         : {nrecs} x 0x{RECORD_SIZE:x}")
    print()
    print("  rec   used_bytes   min_i16   max_i16   field5 (bytes 8..b)")
    print("  ---   ----------   -------   -------   --------------------")
    for i in range(nrecs):
        off = i * RECORD_SIZE
        rec = data[off : off + RECORD_SIZE]
        s = record_stats(rec)
        a, b = struct.unpack_from("<hh", rec, 8)
        print(
            f"  {i:3d}   {s['used']:>5d} (0x{s['used']:04x})  "
            f"{s['min']:>7d}   {s['max']:>7d}   ({a:>5d}, {b:>5d})"
        )
    print()
    print(
        "  field5 is the 5th 16-bit pair — clusters near (0, -1024) suggest a "
        "downward unit\n  vector in PS1 1024=1.0 fixed point, but the per-record "
        "field schema is not fully\n  decoded. See docs/formats/BBM.md."
    )


def dump_record(path, idx, outdir):
    data = parse(path)
    nrecs = len(data) // RECORD_SIZE
    if not (0 <= idx < nrecs):
        raise ValueError(f"record index {idx} out of range 0..{nrecs - 1}")
    os.makedirs(outdir, exist_ok=True)
    out = os.path.join(outdir, f"rec{idx:02d}.bin")
    with open(out, "wb") as f:
        f.write(data[idx * RECORD_SIZE : (idx + 1) * RECORD_SIZE])
    print(f"wrote {out} ({RECORD_SIZE} bytes)")


def dump_all(path, outdir):
    data = parse(path)
    nrecs = len(data) // RECORD_SIZE
    os.makedirs(outdir, exist_ok=True)
    for i in range(nrecs):
        out = os.path.join(outdir, f"rec{i:02d}.bin")
        with open(out, "wb") as f:
            f.write(data[i * RECORD_SIZE : (i + 1) * RECORD_SIZE])
    print(f"wrote {nrecs} records to {outdir}/")


def hex_record(path, idx, nbytes=128):
    data = parse(path)
    off = idx * RECORD_SIZE
    rec = data[off : off + RECORD_SIZE]
    print(f"record {idx} @ file offset 0x{off:x}, first {nbytes} bytes:")
    for i in range(0, nbytes, 16):
        hexpart = " ".join(f"{b:02x}" for b in rec[i : i + 16])
        i16part = " ".join(
            f"{v:>6d}" for v in struct.unpack_from("<8h", rec, i)
        )
        print(f"  0x{i:04x}: {hexpart}    | {i16part}")


def csv_all(path):
    data = parse(path)
    nrecs = len(data) // RECORD_SIZE
    max_i16 = USED_BYTES // 2
    headers = ["rec"] + [f"i{i}" for i in range(max_i16)]
    print(",".join(headers))
    for i in range(nrecs):
        off = i * RECORD_SIZE
        vals = struct.unpack_from(f"<{max_i16}h", data, off)
        print(",".join([str(i)] + [str(v) for v in vals]))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("path", help="path to WIN.DAT")
    ap.add_argument("--rec", type=int, help="hex preview of this record index")
    ap.add_argument("--dump", metavar="OUTDIR", help="extract every record as recNN.bin")
    ap.add_argument("--csv", action="store_true", help="dump all i16 values as CSV")
    ap.add_argument("--bytes", type=int, default=128, help="bytes to preview for --rec")
    args = ap.parse_args()

    if args.csv:
        csv_all(args.path)
    elif args.dump:
        if args.rec is not None:
            dump_record(args.path, args.rec, args.dump)
        else:
            dump_all(args.path, args.dump)
    elif args.rec is not None:
        hex_record(args.path, args.rec, args.bytes)
    else:
        summary(args.path)


if __name__ == "__main__":
    try:
        main()
    except (ValueError, FileNotFoundError) as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(2)
