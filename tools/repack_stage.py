#!/usr/bin/env python3
"""
repack_stage.py - Split a BB2 `STAGE*.BIN` (disc/LOADSE, disc/LOADSE1) into
its six VAB packs and rebuild it. Format in docs/formats/STAGE_BIN.md; each
record is a VAB pack (docs/formats/SE.md), handled by repack_vab.py.

    repack_stage.py unpack STAGE00.BIN OUTDIR
    repack_stage.py pack DIR out.bin
    repack_stage.py --roundtrip FILE.BIN|DIR

The file is a 13-word header (six (offset, size) pairs, one per record,
then the offset of the billboard list), the six records back to back from
0x34, and the billboard list after record 5. func_8005BA8C
(src/main/4BC3C.c) loads record 0 and two of records 1..5 with
snd_LoadVabPack and hands the list to eff_SetBillboardList
(src/main/51268.c): 12-byte entries (three s32), ending at the first entry
whose word 0 has bit 0 clear (every file ends with 12 zero bytes).
`unpack` writes:

    rec0/ .. rec5/    one repack_vab.py directory per record
    billboards.bin    the billboard list with its terminator, verbatim

`pack` rebuilds each record with repack_vab.pack, lays them out back to back
and recomputes the header, so a record may change size (it must stay a
whole number of 4-byte words), and puts the billboard list after record 5. `--roundtrip` unpacks and repacks in memory
and checks byte identity.
"""

import argparse
import struct
import sys
from pathlib import Path

import repack_vab

HDR = 0x34
NREC = 6


def records(data):
    """Returns ([record bytes] * 6, billboard list bytes)."""
    if len(data) < HDR:
        raise ValueError("file shorter than the 0x34-byte header")
    w = struct.unpack_from("<13I", data, 0)
    if w[0] != HDR:
        raise ValueError(f"record 0 at 0x{w[0]:x}, expected 0x{HDR:x}")
    spans = [(w[2 * i], w[2 * i + 1]) for i in range(NREC)]
    pos = HDR
    for i, (off, size) in enumerate(spans):
        if off != pos:
            raise ValueError(f"record {i} at 0x{off:x}, expected 0x{pos:x} "
                             "(records are back to back)")
        pos = off + size
    if w[12] != pos:
        raise ValueError(f"billboard list at 0x{w[12]:x}, expected the end "
                         f"of record 5 0x{pos:x}")
    if pos > len(data):
        raise ValueError("records run past the end of the file")
    return [data[o:o + s] for o, s in spans], data[pos:]


def build(recs, billboards):
    if len(recs) != NREC:
        raise ValueError(f"{len(recs)} records, expected {NREC}")
    words = []
    pos = HDR
    for i, r in enumerate(recs):
        if len(r) % 4:
            raise ValueError(f"record {i}: {len(r)} bytes, not whole words")
        words += [pos, len(r)]
        pos += len(r)
    words.append(pos)
    return struct.pack("<13I", *words) + b"".join(recs) + billboards


def unpack(data):
    """Returns ([repack_vab.unpack parts] * 6, billboards)."""
    recs, billboards = records(data)
    parts = []
    for i, r in enumerate(recs):
        try:
            parts.append(repack_vab.unpack(r))
        except ValueError as e:
            raise ValueError(f"record {i}: {e}") from None
    return parts, billboards


def pack(parts, billboards):
    recs = []
    for i, p in enumerate(parts):
        try:
            recs.append(repack_vab.pack(*p))
        except ValueError as e:
            raise ValueError(f"record {i}: {e}") from None
    return build(recs, billboards)


def write_dir(out, parts, billboards):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    for i, p in enumerate(parts):
        repack_vab.write_dir(out / f"rec{i}", *p)
    (out / "billboards.bin").write_bytes(billboards)


def read_dir(src):
    src = Path(src)
    parts = []
    for i in range(NREC):
        try:
            parts.append(repack_vab.read_dir(src / f"rec{i}"))
        except (OSError, ValueError) as e:
            raise ValueError(f"rec{i}: {e}") from None
    return parts, (src / "billboards.bin").read_bytes()


def roundtrip(path):
    data = Path(path).read_bytes()
    try:
        back = pack(*unpack(data))
    except ValueError as e:
        print(f"{Path(path).name}: ERROR {e}")
        return False
    ok = back == data
    print(f"{Path(path).name}: {'identical' if ok else 'DIFFERS'} ({len(data)} bytes)")
    return ok


def main():
    ap = argparse.ArgumentParser(description="BB2 STAGE*.BIN unpack/pack")
    ap.add_argument("--roundtrip", metavar="PATH",
                    help="STAGE*.BIN file or directory: unpack+pack, check "
                         "identical")
    ap.add_argument("cmd", nargs="?", choices=["unpack", "pack"])
    ap.add_argument("src", nargs="?")
    ap.add_argument("dst", nargs="?")
    args = ap.parse_args()

    if args.roundtrip:
        p = Path(args.roundtrip)
        files = sorted(f for f in p.iterdir()
                       if f.name.upper().startswith("STAGE")
                       and f.suffix.upper() == ".BIN") \
            if p.is_dir() else [p]
        results = [roundtrip(f) for f in files]
        sys.exit(0 if results and all(results) else 1)
    if not (args.cmd and args.src and args.dst):
        ap.error("need unpack|pack SRC DST, or --roundtrip PATH")
    try:
        if args.cmd == "unpack":
            parts, billboards = unpack(Path(args.src).read_bytes())
            write_dir(args.dst, parts, billboards)
            print(f"wrote {args.dst}: rec0..rec{NREC - 1}, billboards.bin "
                  f"({len(billboards)} bytes)")
        else:
            out = pack(*read_dir(args.src))
            Path(args.dst).write_bytes(out)
            print(f"wrote {args.dst} ({len(out)} bytes)")
    except ValueError as e:
        sys.exit(f"error: {e}")


if __name__ == "__main__":
    main()
