#!/usr/bin/env python3
"""Segment modes of every primitive stream the texture-offset walker can load.

gpu_OffsetTexPrimStream (src/main/32D04.c:588-772) walks one entry of a
relocated block: segments of primitive groups, each segment headed by a count
halfword and (count bit 15 clear) a mode halfword. Its four
gpu_OffsetTexPoly{FT3,FT4,GT3,GT4} calls (32D04.c:635-660) run only for a
textured group (type bit 2 set, or type 0x26-0x29) in a mode-0 segment. Its
two callers walk section 1 of a loaded NDATA file (word 1 of the file's
offset table, a reloc_RelocateBlock table: u32 count, then count
block-relative u32 entry offsets, 32D04.c:898-915):

  slot  gpu_OffsetBlockTexPrimStreams over every entry of a fighter slot's
        transform block (309CC.c:228-266, 2B344.c:1757-1781). The asset id
        is g_fight_selection_object_ids[] (0-30, 62, 63; d_7D920.c:137) or
        g_fight_step_row_object_ids[] (58-61, 30, 63; d_7D920.c:704);
        ndata_ReadRemappedIndexPackFirstElseRead maps ids >= 0x1F to id - 0x1B
        (35000.c:227-234), so NDATA 0-36, and ids 0x1E-0x22 and 0x24 may
        instead be served from a sub-block of pack 0x25 + n, n 0 or 1
        (g_ndata_pack_index, 35000.c:293-301, 9F9C.c:1847), i.e. NDATA 37 / 38
        sub-blocks 0-4.
  block6  gpu_OffsetBlockTexPrimStream over the selected entries of block 6
        (35000.c:733-769), loaded from NDATA 0x83 + i or 0x10C + i
        (35000.c:240-246), i the math_UnorderedPairIndex selection
        (368E4.c:1848-1937): NDATA 131-267 and 268-304.

This walks every entry of every NDATA file and pack sub-block whose word 1 is
a well-formed reloc table, with the walker's own rules and the EXE's
g_gpu_prim_type_halfword_sizes, and counts segments and textured groups per
mode. A walk is clean when it stays inside the file, meets only modes 0-2
and types below 44 (the size table's length). Unclean blocks are listed with
the first fault; the walker's real callers never load them if every
caller-set block is clean. The tex_mode columns count textured primitives
(the sum of the groups' counts). Exits 1 when a caller-set block has no
well-formed table (so it would be missing from the rows).

  python tools/trace/scan/prim_streams.py (--bin IMAGE.bin | --dat NDATA.DAT)
      [--inf disc/NDATA/NDATA.INF] [--exe disc/SLUS_006.63]

prints one TSV row per block with a reloc table (block, caller, entries,
clean, first_fault, seg_mode0, seg_mode1, seg_mode2, tex_mode0, tex_mode1,
tex_mode2), then '#' summary lines per caller set.
"""

import argparse
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
EXE_LOAD = 0x80010000
EXE_HEADER = 0x800
SIZES_ADDR = 0x80095588  # g_gpu_prim_type_halfword_sizes[44] (32D04.c:1513)
N_TYPES = 44
SECTOR = 0x800
PACK_IDS = (0x25, 0x26)
PACK_SUBBLOCKS = 5
SLOT_IDS = set(range(0, 37))
BLOCK6_FIRST = 0x83
BLOCK6_IDS = set(range(131, 305))
EXPECTED = ({str(i) for i in SLOT_IDS | BLOCK6_IDS}
            | {"%d/%d" % (p, k) for p in PACK_IDS for k in range(PACK_SUBBLOCKS)})
# mode 1 / mode 2 textured primitive sizes in halfwords, by kind
# (32D04.c:662-741)
MODE1_SIZE = {0: 12, 1: 12, 2: 18, 3: 22}
MODE2_SIZE = {0: 10, 1: 12, 2: 10, 3: 12}
STEP_LIMIT = 400000


class Fault(Exception):
    pass


def read_sizes(exe):
    off = SIZES_ADDR - EXE_LOAD + EXE_HEADER
    return struct.unpack_from("<%dh" % N_TYPES, exe, off)


def is_textured(typ):
    return bool(typ & 4) or 0 <= typ - 0x26 < 4


def kind_of(typ):
    return typ - 0x26 if 0 <= typ - 0x26 < 4 else (typ >> 3) & 3


def walk(buf, pos, sizes):
    """Walk one stream from byte offset pos as gpu_OffsetTexPrimStream does.

    Returns (segment modes, [(mode, type, count)] textured groups); raises
    Fault when the walk leaves buf, meets a mode above 2 or a type outside
    the size table."""

    def h(o):
        if o < 0 or o + 2 > len(buf):
            raise Fault("read past block at 0x%X" % o)
        return struct.unpack_from("<H", buf, o)[0]

    def size(typ):
        if typ >= N_TYPES:
            raise Fault("type 0x%X outside the size table" % typ)
        return sizes[typ]

    segs, groups = [], []
    packed_segs = 0
    steps = 0
    while True:
        count = h(pos)
        if count & 0x8000:
            packed, mode = True, 2
            pos += 2 + 2 * ((count & 0x7FFF) + 1)
            if pos & 3:
                pos += 4 - (pos & 3)
        else:
            packed, mode = False, h(pos + 2)
            if mode > 2:
                raise Fault("segment mode %d" % mode)
            pos += 4 + 2 * (count * 12 if mode == 0 else count * 3)
        if mode == 1 and pos & 3:
            pos += 2
        segs.append(mode)
        while True:
            count = h(pos)
            pos += 2
            if count == 0:
                break
            typ = h(pos)
            pos += 2
            steps += 1
            if steps > STEP_LIMIT:
                raise Fault("no end after %d groups" % STEP_LIMIT)
            if is_textured(typ):
                groups.append((mode, typ, count))
                if mode == 0:
                    pos += 2 * size(typ) * count
                elif mode == 1:
                    pos += 2 * MODE1_SIZE[kind_of(typ)] * count
                else:
                    pos += 2 * MODE2_SIZE[kind_of(typ)] * count
            elif packed and typ == 0:
                pos += 2 * 26 * count
            elif packed and typ == 8:
                pos += 2 * 32 * count
            else:
                n = (count * size(typ)) & 0xFFFF  # the walker's (s16) cast
                pos += 2 * (n - 0x10000 if n & 0x8000 else n)
        if packed:
            if packed_segs == 12:
                return segs, groups
            packed_segs += 1
            while h(pos) != 0xFFFF:
                pos += 6
            pos += 2
        more = h(pos)
        pos += 2
        if more == 0:
            return segs, groups


def offset_table(buf):
    """(count, [u32 offsets]) of a file's leading offset table, or None."""
    if len(buf) < 8:
        return None
    n = struct.unpack_from("<I", buf, 0)[0]
    if not 2 <= n < 64 or 4 * (n + 2) > len(buf):
        return None
    return n, list(struct.unpack_from("<%dI" % (n + 1), buf, 4))


def reloc_entries(buf):
    """Entry byte offsets (from the start of buf) of section 1's reloc
    table, or None when word 1 is not a well-formed table."""
    t = offset_table(buf)
    if not t:
        return None
    s1 = t[1][0] & ~3
    if not 8 <= s1 < len(buf):
        return None
    c = struct.unpack_from("<I", buf, s1)[0]
    if not 1 <= c < 0x1000 or s1 + 4 + 4 * c > len(buf):
        return None
    offs = struct.unpack_from("<%dI" % c, buf, s1 + 4)
    if not all(4 + 4 * c <= e < len(buf) - s1 for e in offs):
        return None
    return [s1 + e for e in offs]


def scan_block(buf, sizes):
    entries = reloc_entries(buf)
    if entries is None:
        return None
    row = {"entries": len(entries), "fault": ""}
    for m in range(3):
        row["seg%d" % m] = row["tex%d" % m] = 0
    for i, e in enumerate(entries):
        try:
            segs, groups = walk(buf, e, sizes)
        except Fault as ex:
            if not row["fault"]:
                row["fault"] = "entry %d: %s" % (i, ex)
            continue
        for m in segs:
            row["seg%d" % m] += 1
        for m, _typ, cnt in groups:
            row["tex%d" % m] += cnt
    return row


def ndata_blocks(inf, read):
    """(name, id, sub, bytes) for every NDATA file and pack sub-block."""
    for fid in range(len(inf) // 4):
        st, ln = struct.unpack_from("<HH", inf, fid * 4)
        if st == 0xFFFF:
            continue
        buf = read(st, ln)
        if not buf:
            continue
        yield str(fid), fid, None, buf
        if fid in PACK_IDS:
            t = offset_table(buf)
            if t and t[0] == PACK_SUBBLOCKS:
                offs = t[1]
                for k in range(PACK_SUBBLOCKS):
                    a, b = offs[k] & ~3, offs[k + 1]
                    if 0 < a < b <= len(buf):
                        yield "%d/%d" % (fid, k), fid, k, buf[a:b]


def caller_of(fid, sub):
    if fid in PACK_IDS:
        return "slot" if sub is not None else "-"
    if fid in SLOT_IDS:
        return "slot"
    if fid >= BLOCK6_FIRST:
        return "block6"
    return "-"


def iso_ndata_reader(bin_path):
    sys.path.insert(0, str(ROOT / "tools"))
    import extract_iso as iso

    f = open(bin_path, "rb")

    def find(extent, size, name):
        data = b"".join(
            iso.read_sector(f, extent + i) for i in range((size + 2047) // 2048)
        )
        o = 0
        while o < len(data):
            if data[o] == 0:
                o = (o // 2048 + 1) * 2048
                continue
            r = iso.parse_directory_record(data, o)
            if r and r["name"].split(";")[0].upper() == name:
                return r
            o += data[o]
        raise SystemExit("prim_streams: %s not found in %s" % (name, bin_path))

    root = iso.parse_directory_record(iso.read_sector(f, 16), 156)
    d = find(root["extent"], root["size"], "NDATA")
    r = find(d["extent"], d["size"], "NDATA.DAT")

    def read(st, ln):
        return b"".join(iso.read_sector(f, r["extent"] + st + i) for i in range(ln))

    return read


def dat_reader(dat_path):
    f = open(dat_path, "rb")

    def read(st, ln):
        f.seek(st * SECTOR)
        return f.read(ln * SECTOR)

    return read


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--bin", help="MODE2/2352 disc image holding NDATA.DAT")
    src.add_argument("--dat", help="an extracted NDATA.DAT")
    ap.add_argument("--inf", default=str(ROOT / "disc" / "NDATA" / "NDATA.INF"))
    ap.add_argument("--exe", default=str(ROOT / "disc" / "SLUS_006.63"))
    args = ap.parse_args(argv)

    sizes = read_sizes(Path(args.exe).read_bytes())
    inf = Path(args.inf).read_bytes()
    read = iso_ndata_reader(args.bin) if args.bin else dat_reader(args.dat)

    cols = ["seg0", "seg1", "seg2", "tex0", "tex1", "tex2"]
    print("block\tcaller\tentries\tclean\tfirst_fault\tseg_mode0\tseg_mode1"
          "\tseg_mode2\ttex_mode0\ttex_mode1\ttex_mode2")
    totals = {}
    seen = set()
    for name, fid, sub, buf in ndata_blocks(inf, read):
        row = scan_block(buf, sizes)
        if row is None:
            continue
        seen.add(name)
        caller = caller_of(fid, sub)
        clean = 0 if row["fault"] else 1
        print("\t".join([name, caller, str(row["entries"]), str(clean),
                         row["fault"] or "-"] + [str(row[c]) for c in cols]))
        t = totals.setdefault(caller, {"blocks": 0, "unclean": 0,
                                       "entries": 0, **{c: 0 for c in cols}})
        t["blocks"] += 1
        t["unclean"] += 1 - clean
        t["entries"] += row["entries"]
        for c in cols:
            t[c] += row[c]
    for caller in sorted(totals):
        t = totals[caller]
        print("# %s: %d blocks (%d unclean), %d entries; segments by mode "
              "0/1/2: %d/%d/%d; textured primitives by mode 0/1/2: %d/%d/%d" % (
                  caller, t["blocks"], t["unclean"], t["entries"], t["seg0"],
                  t["seg1"], t["seg2"], t["tex0"], t["tex1"], t["tex2"]))
    missing = sorted(EXPECTED - seen, key=lambda n: [int(x) for x in n.split("/")])
    if missing:
        print("# missing caller-set blocks (no well-formed table): "
              + " ".join(missing))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
