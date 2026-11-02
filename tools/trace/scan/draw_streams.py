#!/usr/bin/env python3
"""Segment modes and handler types of every stream gpu_DrawQueuedEntries can draw.

gpu_DrawQueuedEntries (asm/funcs/gpu_DrawQueuedEntries.s) draws a queued
entry's stream g_reloc_block_entry_tables[entry+4][entry+2]. A stream is a
list of segments; each segment is:

  u32 header       low halfword count (0 ends the stream), high halfword the
                   mode, an index into jtbl_8001545C (5 cases, d_5BF0.c:64-67)
  vertex data      mode 0 / 2: count x 24 bytes (gte_ProjectSVectorTriples);
                   mode 1 / 4: count x 6 bytes, then rounded up to a word
                   (gte_ProjectPackedVertices); mode 3: none (func_8004A938,
                   an empty function, takes the projection's place)
  group list       (u16 count, u16 type) pairs, 0 count ends it
                   (func_8004A940); each group calls table[type] once, which
                   steps count records of a fixed size
  u16 more         0 ends the stream, else the next segment header follows

The handler table per mode: 0 and 3 g_gpu_default_primitive_handlers (the
scratchpad word 0x1F80001C, which gpu_PrepareViewAndFlushDrawQueue sets to it),
1 the same table, its second half of 44 when the queued entry's byte 1 bit 0
is set, 2 g_gpu_embedded_lit_packet_handlers, 4
g_gpu_packed_vertex_textured_primitive_handlers. The record size of each
handler is read from the EXE: the one `addiu $s0, $s0, imm` before its
`jr $ra`; a handler without one (gpu_FailCheckOnUnhandledPrimitiveType) is a
fault. A mode of 5 or more jumps through the next data into other code, so it
is a fault too.

Which NDATA sections are relocated into a drawn block varies by caller
(309CC.c:234-235, 35000.c:737-741, 368E4.c:151-355), so this walks every reloc
table (u32 count, bit 15 the relocated flag, then count block-relative u32
entry offsets) that any of the first 64 header words of an NDATA file or of a
sub-file (a section that itself starts with an offset table) points at. A
table is clean when every entry walks to its end inside the buffer under
either half of the default table for mode 1. The targets column counts what
the four draw-side data arms need (trace-scenarios FRONTIER): mode-3
segments (func_8004A938), type 0x14 groups in mode-4 segments
(func_8004D634, packed table entry 20), and groups dispatched to
gpu_EmitColorScaledFlatLitTexturedTriangles / Quads (default table entries
42 / 43).

  python tools/trace/scan/draw_streams.py (--bin IMAGE.bin | --dat NDATA.DAT)
      [--inf disc/NDATA/NDATA.INF] [--exe disc/SLUS_006.63]

prints one TSV row per table (block, word, entries, clean_lo, clean_hi,
first_fault, seg_mode0..4, mode3, packed14, colorscaled), then '#' lines:
totals over clean tables and the (mode, type, handler) histogram over every
entry that walks clean. Exits 1 when a handler has more than one record
size (the reader would be wrong).
"""

import argparse
import collections
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import prim_streams as ps  # noqa: E402

ROOT = Path(__file__).resolve().parents[3]
EXE_LOAD = 0x80010000
EXE_HEADER = 0x800
DEFAULT_TABLE = 0x80095328  # g_gpu_default_primitive_handlers[88] (32D04.c:1355)
PACKED_TABLE = 0x80095488   # g_gpu_packed_vertex_textured_primitive_handlers[32]
LIT_TABLE = 0x80095508      # g_gpu_embedded_lit_packet_handlers[32]
DEFAULT_HALF = 44
MODE3_HANDLER = 0x8004A938  # func_8004A938 (3AB48.c:18)
PACKED14_HANDLER = 0x8004D634  # func_8004D634
COLORSCALED = (0x8004FF40, 0x80050120)  # gpu_EmitColorScaledFlatLitTextured{Triangles,Quads}
JR_RA = 0x03E00008
HEADER_WORDS = 64
SEG_LIMIT = 4096
GROUP_LIMIT = 100000


class Fault(Exception):
    pass


def exe_word(exe, addr):
    return struct.unpack_from("<I", exe, addr - EXE_LOAD + EXE_HEADER)[0]


def handler_stride(exe, addr, limit=600):
    """Record size in bytes of the handler at addr: the immediates of every
    `addiu $s0, $s0, imm` up to its first `jr $ra`. None when it has none."""
    imms = set()
    for i in range(limit):
        w = exe_word(exe, addr + 4 * i)
        if w >> 16 == 0x2610:
            imms.add(w & 0xFFFF)
        if w == JR_RA:
            break
    else:
        raise SystemExit("draw_streams: no jr $ra after 0x%08X" % addr)
    if len(imms) > 1:
        raise ValueError("handler 0x%08X has record sizes %s" % (addr, sorted(imms)))
    return imms.pop() if imms else None


def read_tables(exe):
    """{'default'|'packed'|'lit': [handler addresses]}, {addr: stride}."""
    tabs = {
        "default": [exe_word(exe, DEFAULT_TABLE + 4 * i) for i in range(2 * DEFAULT_HALF)],
        "packed": [exe_word(exe, PACKED_TABLE + 4 * i) for i in range(32)],
        "lit": [exe_word(exe, LIT_TABLE + 4 * i) for i in range(32)],
    }
    strides = {a: handler_stride(exe, a) for t in tabs.values() for a in t}
    return tabs, strides


def walk(buf, pos, tabs, strides, hi=False):
    """Walk one stream from byte offset pos as gpu_DrawQueuedEntries does.

    Returns (segment modes, [(mode, type, handler, count)]); raises Fault
    when the walk leaves buf, meets mode 5+, an unaligned header, a type past
    its table or a handler with no record size."""

    def h(o):
        if o < 0 or o + 2 > len(buf):
            raise Fault("read past block at 0x%X" % o)
        return struct.unpack_from("<H", buf, o)[0]

    segs, groups = [], []
    for _ in range(SEG_LIMIT):
        if pos & 3:
            raise Fault("unaligned segment header at 0x%X" % pos)
        count, mode = h(pos), h(pos + 2)
        pos += 4
        if count == 0:
            return segs, groups
        if mode > 4:
            raise Fault("segment mode %d" % mode)
        if count & 0x8000 and pos + 6 * count > len(buf):
            raise Fault("count 0x%X has bit 15 set (a packed segment header, "
                        "gpu_OffsetTexPrimStream's format) and runs past the "
                        "block" % count)
        if mode in (0, 2):
            pos += 24 * count
        elif mode in (1, 4):
            pos += 6 * count
            pos += pos & 3
        if mode in (0, 3):
            tab, base = tabs["default"], 0
        elif mode == 1:
            tab, base = tabs["default"], DEFAULT_HALF if hi else 0
        elif mode == 2:
            tab, base = tabs["lit"], 0
        else:
            tab, base = tabs["packed"], 0
        segs.append(mode)
        n = h(pos)
        pos += 2
        g = 0
        while n:
            typ = h(pos)
            pos += 2
            if base + typ >= len(tab):
                raise Fault("type 0x%X past the mode-%d table" % (typ, mode))
            addr = tab[base + typ]
            if strides[addr] is None:
                raise Fault("type 0x%X in mode %d has no handler" % (typ, mode))
            groups.append((mode, typ, addr, n))
            pos += strides[addr] * n
            n = h(pos)
            pos += 2
            g += 1
            if g > GROUP_LIMIT:
                raise Fault("no group-list end after %d groups" % GROUP_LIMIT)
        more = h(pos)
        pos += 2
        if more == 0:
            return segs, groups
    raise Fault("no stream end after %d segments" % SEG_LIMIT)


def targets(segs, groups):
    c = collections.Counter()
    c["mode3"] = sum(1 for m in segs if m == 3)
    for mode, typ, addr, _n in groups:
        if mode == 4 and addr == PACKED14_HANDLER:
            c["packed14"] += 1
        if addr in COLORSCALED:
            c["colorscaled"] += 1
    return c


def reloc_tables(buf):
    """(header word index, [entry byte offsets]) for each distinct offset
    among the first HEADER_WORDS words that starts a well-formed reloc
    table."""
    out, seen = [], set()
    for k in range(1, min(HEADER_WORDS, len(buf) // 4)):
        s = struct.unpack_from("<I", buf, 4 * k)[0] & ~3
        if s in seen or not 4 <= s < len(buf) - 8:
            continue
        seen.add(s)
        c = struct.unpack_from("<I", buf, s)[0] & 0x7FFF
        if not 1 <= c < 0x1000 or s + 4 + 4 * c > len(buf):
            continue
        offs = struct.unpack_from("<%dI" % c, buf, s + 4)
        if not all(4 + 4 * c <= e < len(buf) - s for e in offs):
            continue
        out.append((k, [s + e for e in offs]))
    return out


def sub_files(buf):
    """(section index, bytes) of each offset-table section that itself
    starts with an offset table."""
    t = ps.offset_table(buf)
    if not t:
        return []
    n, offs = t
    out = []
    for k in range(n):
        a, b = offs[k] & ~3, offs[k + 1] & ~3
        if 0 < a < b <= len(buf) and ps.offset_table(buf[a:b]):
            out.append((k, buf[a:b]))
    return out


def scan_table(buf, entries, tabs, strides, hist):
    row = {"entries": len(entries), "clean": 0, "clean_lo": 0, "clean_hi": 0,
           "fault": "", "seg": [0] * 5, "t": collections.Counter()}
    for i, e in enumerate(entries):
        ok = False
        first = None
        for hi in (False, True):
            try:
                segs, groups = walk(buf, e, tabs, strides, hi)
            except Fault as ex:
                first = first or ex
                continue
            row["clean_hi" if hi else "clean_lo"] += 1
            row["t"].update(targets(segs, groups))
            for mode, typ, addr, n in groups:
                hist[(mode, typ, addr, hi)] += n
            if not ok:
                for m in segs:
                    row["seg"][m] += 1
            ok = True
        row["clean"] += ok
        if not ok and not row["fault"]:
            row["fault"] = "entry %d: %s" % (i, first)
    return row


def ndata_buffers(inf, read):
    for fid in range(len(inf) // 4):
        st, ln = struct.unpack_from("<HH", inf, fid * 4)
        if st == 0xFFFF:
            continue
        buf = read(st, ln)
        if not buf:
            continue
        yield str(fid), buf
        for k, sub in sub_files(buf):
            yield "%d/%d" % (fid, k), sub


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--bin", help="MODE2/2352 disc image holding NDATA.DAT")
    src.add_argument("--dat", help="an extracted NDATA.DAT")
    ap.add_argument("--inf", default=str(ROOT / "disc" / "NDATA" / "NDATA.INF"))
    ap.add_argument("--exe", default=str(ROOT / "disc" / "SLUS_006.63"))
    args = ap.parse_args(argv)

    try:
        tabs, strides = read_tables(Path(args.exe).read_bytes())
    except ValueError as ex:
        print("draw_streams: %s" % ex, file=sys.stderr)
        return 1
    inf = Path(args.inf).read_bytes()
    read = ps.iso_ndata_reader(args.bin) if args.bin else ps.dat_reader(args.dat)

    print("block\tword\tentries\tclean_lo\tclean_hi\tfirst_fault\tseg_mode0"
          "\tseg_mode1\tseg_mode2\tseg_mode3\tseg_mode4\tmode3\tpacked14"
          "\tcolorscaled")
    hist = collections.Counter()
    tot = collections.Counter()
    for name, buf in ndata_buffers(inf, read):
        for word, entries in reloc_tables(buf):
            row = scan_table(buf, entries, tabs, strides, hist)
            t = row["t"]
            print("\t".join([name, str(word), str(row["entries"]),
                             str(row["clean_lo"]), str(row["clean_hi"]),
                             row["fault"] or "-"]
                            + [str(x) for x in row["seg"]]
                            + [str(t["mode3"]), str(t["packed14"]),
                               str(t["colorscaled"])]))
            tot["tables"] += 1
            if row["clean"] == row["entries"]:
                tot["clean_tables"] += 1
            tot["clean_entries"] += row["clean"]
            tot["entries"] += row["entries"]
            for m in range(5):
                tot["seg%d" % m] += row["seg"][m]
            tot.update(t)
    print("# %d tables (%d clean), %d of %d entries walk clean; segments by "
          "mode 0/1/2/3/4: %d/%d/%d/%d/%d; targets over clean walks: mode-3 "
          "segments %d, mode-4 type-0x14 groups %d, colour-scaled groups %d" % (
              tot["tables"], tot["clean_tables"], tot["clean_entries"],
              tot["entries"], tot["seg0"], tot["seg1"], tot["seg2"],
              tot["seg3"], tot["seg4"], tot["mode3"], tot["packed14"],
              tot["colorscaled"]))
    for (mode, typ, addr, hi), n in sorted(hist.items()):
        print("# mode %d%s type 0x%02X handler 0x%08X stride %d: %d records" % (
            mode, " hi" if hi else "", typ, addr, strides[addr], n))
    return 0


if __name__ == "__main__":
    sys.exit(main())
