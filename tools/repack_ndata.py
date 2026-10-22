#!/usr/bin/env python3
"""
repack_ndata.py - Rebuild the BB2 NDATA archive (NDATA.INF + NDATA.DAT) from
extracted entries. The inverse of `inspect_ndata.py --dump-all`; format in
docs/formats/NDATA.md.

    repack_ndata.py --template <NDATA.INF> --entries <DIR> --out <OUTDIR>
    repack_ndata.py --roundtrip <NDATA.INF> [--dat <NDATA.DAT>]

Layout rule (the one the shipped archive follows): walk the template INF in id
order; every chained slot is placed at the next free sector, its
`ndata_NNNN.bin` from <DIR> rounded up to whole 0x800-byte sectors and the
tail zero-padded. Sentinel slots (0xFFFF, 0xFFFF) stay sentinels. Every
other slot is chained except the build leftovers 899 (0x9800, past the DAT)
and 900 (0x1900 + 0x5956: overlaps entries and runs past the DAT), which keep
their template INF words verbatim and get no data; any entry file for them
(`--dump-all` writes 899 empty and 900 truncated) is ignored. The template's
chained slots must be contiguous from sector 0 (true of the shipped INF and of
any INF this tool writes). A file (even empty) for a sentinel slot, an empty
file for a chained slot, or any file in <DIR> not named ndata_NNNN.bin for a
template slot is an error. An entry may change size; every later entry moves.

`--roundtrip` extracts every entry of the given archive in memory, repacks
with the same code and checks both outputs are byte-identical to the input.
"""

import argparse
import struct
import sys
from pathlib import Path

SECTOR_SIZE = 0x800
SENTINEL = 0xFFFF
U16_MAX = 0xFFFF


def parse_inf(data):
    if len(data) % 4 != 0:
        raise ValueError(f"INF length {len(data)} not multiple of 4")
    return [struct.unpack_from("<HH", data, i * 4) for i in range(len(data) // 4)]


LEFTOVERS = (899, 900)


def chained_slots(template):
    """Ids of the non-sentinel, non-leftover slots; checks they are laid out
    back to back from sector 0."""
    chained = []
    running = 0
    for i, (start, length) in enumerate(template):
        if (start, length) == (SENTINEL, SENTINEL) or i in LEFTOVERS:
            continue
        if start != running:
            raise ValueError(f"template slot {i} starts at sector 0x{start:x}, "
                             f"not 0x{running:x}: chain not contiguous")
        chained.append(i)
        running = start + length
    return chained


def pack(template, blobs):
    """template: [(start, length)] from the template INF.
    blobs: {id: bytes}, one non-empty entry per chained slot.
    Returns (inf_bytes, dat_bytes)."""
    chained = set(chained_slots(template))
    missing = sorted(chained - set(blobs))
    if missing:
        raise ValueError(f"no data for chained slots {missing}")
    inf = bytearray()
    dat = bytearray()
    for i, (start, length) in enumerate(template):
        blob = blobs.get(i)
        if i not in chained:
            if (start, length) == (SENTINEL, SENTINEL) and blob:
                raise ValueError(f"entry {i}: data given for a sentinel slot")
            inf += struct.pack("<HH", start, length)
            continue
        if not blob:
            raise ValueError(f"entry {i}: empty entry for a chained slot")
        sec = len(dat) // SECTOR_SIZE
        nsec = -(-len(blob) // SECTOR_SIZE)
        if sec >= U16_MAX or nsec > U16_MAX:
            raise ValueError(f"entry {i}: sector {sec}/length {nsec} exceeds u16")
        inf += struct.pack("<HH", sec, nsec)
        dat += blob
        dat += bytes(nsec * SECTOR_SIZE - len(blob))
    extra = set(blobs) - set(range(len(template)))
    if extra:
        raise ValueError(f"entries past the template's {len(template)} slots: "
                         f"{sorted(extra)}")
    return bytes(inf), bytes(dat)


def load_entries(entries_dir, count):
    """{id: bytes} for every ndata_NNNN.bin present, empty ones included."""
    blobs = {}
    for i in range(count):
        p = Path(entries_dir) / f"ndata_{i:04d}.bin"
        if p.is_file():
            blobs[i] = p.read_bytes()
    valid = {f"ndata_{i:04d}.bin" for i in range(count)}
    stray = sorted(p.name for p in Path(entries_dir).iterdir()
                   if p.name not in valid)
    if stray:
        raise ValueError(f"files in {entries_dir} not named ndata_NNNN.bin for "
                         f"one of the template's {count} slots: {stray}")
    return blobs


def extract_all(template, dat):
    blobs = {}
    for i in chained_slots(template):
        start, length = template[i]
        blobs[i] = dat[start * SECTOR_SIZE:(start + length) * SECTOR_SIZE]
    return blobs


def main():
    ap = argparse.ArgumentParser(description="Rebuild BB2 NDATA.INF/NDATA.DAT")
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--template", metavar="INF",
                   help="original NDATA.INF (slot count, sentinels, leftovers)")
    g.add_argument("--roundtrip", metavar="INF",
                   help="extract + repack in memory, check byte-identical")
    ap.add_argument("--entries", metavar="DIR",
                    help="directory of ndata_NNNN.bin (from --dump-all)")
    ap.add_argument("--out", metavar="OUTDIR",
                    help="write OUTDIR/NDATA.INF and OUTDIR/NDATA.DAT")
    ap.add_argument("--dat", help="NDATA.DAT for --roundtrip "
                    "(default: next to the INF)")
    args = ap.parse_args()

    if args.roundtrip:
        if args.entries or args.out:
            ap.error("--roundtrip takes no --entries/--out")
        inf_path = Path(args.roundtrip)
        dat_path = Path(args.dat) if args.dat else inf_path.with_suffix(".DAT")
        inf_in = inf_path.read_bytes()
        dat_in = dat_path.read_bytes()
        template = parse_inf(inf_in)
        inf_out, dat_out = pack(template, extract_all(template, dat_in))
        ok = inf_out == inf_in and dat_out == dat_in
        same = {True: "identical", False: "DIFFERS"}
        print(f"roundtrip {inf_path.name}: INF {same[inf_out == inf_in]}, "
              f"DAT {same[dat_out == dat_in]} "
              f"({len(dat_out)} vs {len(dat_in)} bytes)")
        sys.exit(0 if ok else 1)

    if not args.entries or not args.out:
        ap.error("--template needs --entries and --out")
    template = parse_inf(Path(args.template).read_bytes())
    blobs = load_entries(args.entries, len(template))
    chained = set(chained_slots(template))
    for i in sorted(set(blobs) - chained):
        if template[i] == (SENTINEL, SENTINEL):
            raise ValueError(f"ndata_{i:04d}.bin: slot {i} is a sentinel")
        if blobs[i]:
            print(f"note: ignoring ndata_{i:04d}.bin (leftover slot, kept "
                  "verbatim)", file=sys.stderr)
        del blobs[i]
    inf_out, dat_out = pack(template, blobs)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    (out / "NDATA.INF").write_bytes(inf_out)
    (out / "NDATA.DAT").write_bytes(dat_out)
    print(f"wrote {out / 'NDATA.INF'} ({len(inf_out)} bytes), "
          f"{out / 'NDATA.DAT'} ({len(dat_out)} bytes, "
          f"{len(dat_out) // SECTOR_SIZE} sectors)")


if __name__ == "__main__":
    main()
