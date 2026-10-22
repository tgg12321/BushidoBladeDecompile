#!/usr/bin/env python3
"""
repack_vab.py - Split a BB2 VAB pack (the `.SE` files) into its parts and
rebuild it. Format (and VH layout) in docs/formats/SE.md.

    repack_vab.py unpack file.se OUTDIR
    repack_vab.py pack DIR out.se
    repack_vab.py --roundtrip FILE.se|DIR

A VAB pack is the 16-byte head snd_LoadVabPack reads (`Unk8005C2A8Pack`,
include/game.h: event-table offset, VH offset, VB offset, VB size), the
event table, the VH (pBAV), a gap, the VB (the VAG bodies back to back,
sized by the VH's VAG size table) and a tail. `unpack` writes:

    events.bin        the event table (head word 0 .. word 1)
    vh.bin            the VH, 0x20 + 0x800 + ps * 0x200 + 0x200 bytes
    vag_NNN.adpcm     VAG N (1..vs) as raw SPU-ADPCM, no VAGp header
    pack.json         the gap and tail bytes

`pack` rebuilds the file, recomputing the head offsets, the VH's VAG size
table entries (bytes / 8) and its fsize (VH + VB size). A VAG may change
size (a multiple of 16 bytes, one ADPCM block); the VAG count must match
the VH's `vs`, since the tone attributes refer to VAGs by number, and the
VH and VB must stay 4-byte aligned.
`--roundtrip` unpacks and repacks in memory and checks byte identity.
Growing VAGs can exceed SPU RAM: SsVabOpenHeadWithMode refuses a bank whose
SPU address plus total size passes 0x80000.
"""

import argparse
import json
import struct
import sys
from pathlib import Path

HEAD = 16


def vh_layout(vh):
    if vh[:4] != b"pBAV":
        raise ValueError("no pBAV magic at the VH offset")
    if len(vh) < 0x20:
        raise ValueError("VH truncated")
    version = struct.unpack_from("<I", vh, 4)[0]
    if version < 5:
        raise ValueError(f"VAB version {version}: size table in 4-byte units, "
                         "not supported")
    ps = struct.unpack_from("<H", vh, 0x12)[0]
    if ps > 128:
        raise ValueError(f"ps {ps} > 128 programs")
    vs = vh[0x16]  # VabHdr.vs is a u8 (include/psxsdk/libsnd.h)
    table = 0x20 + 0x800 + ps * 0x200
    if len(vh) < table + 0x200:
        raise ValueError("VH truncated")
    return table, table + 0x200, vs


def unpack(data):
    """Returns (events, vh, vags, extra) with vags = [bytes] for VAG 1..vs."""
    if len(data) < HEAD:
        raise ValueError("file shorter than the 16-byte head")
    ev, vh_off, vb_off, vb_size = struct.unpack_from("<4I", data, 0)
    if ev != HEAD:
        raise ValueError(f"event table at 0x{ev:x}, expected 0x{HEAD:x}")
    if vh_off < HEAD:
        raise ValueError(f"VH offset 0x{vh_off:x} inside the head")
    table, vh_size, vs = vh_layout(data[vh_off:])
    vh = data[vh_off:vh_off + vh_size]
    sizes = [s * 8 for s in struct.unpack_from("<256H", vh, table)]
    if sizes[0] or any(sizes[vs + 1:]):
        raise ValueError("VAG size table has entries outside 1..vs")
    if not all(sizes[1:vs + 1]):
        raise ValueError("VAG size table has an empty entry in 1..vs")
    if sum(sizes) != vb_size:
        raise ValueError(f"VAG sizes sum to 0x{sum(sizes):x}, head says "
                         f"0x{vb_size:x}")
    fsize = struct.unpack_from("<I", vh, 0xC)[0]
    if fsize != vh_size + vb_size:
        raise ValueError(f"VH fsize 0x{fsize:x} != VH + VB 0x{vh_size + vb_size:x}")
    if vb_off < vh_off + vh_size or vb_off + vb_size > len(data):
        raise ValueError("VB overlaps the VH or runs past the file")
    vags = []
    off = vb_off
    for n in range(1, vs + 1):
        vags.append(data[off:off + sizes[n]])
        off += sizes[n]
    extra = {"gap": data[vh_off + vh_size:vb_off].hex(),
             "tail": data[vb_off + vb_size:].hex()}
    return data[ev:vh_off], vh, vags, extra


def pack(events, vh, vags, extra):
    table, vh_size, vs = vh_layout(vh)
    if len(vh) != vh_size:
        raise ValueError(f"vh.bin is {len(vh)} bytes, its header says {vh_size}")
    if len(vags) != vs:
        raise ValueError(f"{len(vags)} VAGs, the VH's vs is {vs}")
    sizes = [0] + [len(v) for v in vags] + [0] * (255 - vs)
    for n, s in enumerate(sizes):
        if 1 <= n <= vs and not s:
            raise ValueError(f"VAG {n} is empty")
        if s % 16:
            raise ValueError(f"VAG {n}: {s} bytes, not whole 16-byte blocks")
        if s // 8 > 0xFFFF:
            raise ValueError(f"VAG {n}: {s} bytes, too big for the size table")
    vb = b"".join(vags)
    vh = bytearray(vh)
    struct.pack_into("<256H", vh, table, *(s // 8 for s in sizes))
    struct.pack_into("<I", vh, 0xC, vh_size + len(vb))
    gap = bytes.fromhex(extra["gap"])
    vh_off = HEAD + len(events)
    vb_off = vh_off + vh_size + len(gap)
    if vh_off % 4 or vb_off % 4:
        raise ValueError(f"VH at 0x{vh_off:x} / VB at 0x{vb_off:x} not 4-byte "
                         "aligned (events.bin and the gap must be whole words)")
    head = struct.pack("<4I", HEAD, vh_off, vb_off, len(vb))
    return head + events + bytes(vh) + gap + vb + bytes.fromhex(extra["tail"])


def write_dir(out, events, vh, vags, extra):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    (out / "events.bin").write_bytes(events)
    (out / "vh.bin").write_bytes(vh)
    for n, v in enumerate(vags, 1):
        (out / f"vag_{n:03d}.adpcm").write_bytes(v)
    (out / "pack.json").write_text(json.dumps(extra, indent=1) + "\n",
                                   newline="\n")


def read_dir(src):
    src = Path(src)
    vh = (src / "vh.bin").read_bytes()
    _table, _size, vs = vh_layout(vh)
    names = sorted(p.name for p in src.glob("vag_*.adpcm"))
    want = [f"vag_{n:03d}.adpcm" for n in range(1, len(names) + 1)]
    if names != want:
        raise ValueError(f"VAG files must be vag_001..vag_{len(names):03d}.adpcm")
    vags = [(src / n).read_bytes() for n in names]
    extra = json.loads((src / "pack.json").read_text())
    if not (isinstance(extra, dict) and set(extra) == {"gap", "tail"}
            and all(isinstance(v, str) for v in extra.values())):
        raise ValueError("pack.json must be an object holding exactly the hex "
                         "strings 'gap' and 'tail'")
    for key in ("gap", "tail"):
        try:
            bytes.fromhex(extra[key])
        except ValueError:
            raise ValueError(f"pack.json: '{key}' is not a hex string")
    return (src / "events.bin").read_bytes(), vh, vags, extra


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
    ap = argparse.ArgumentParser(description="BB2 VAB pack (.SE) unpack/pack")
    ap.add_argument("--roundtrip", metavar="PATH",
                    help=".SE file or directory: unpack+pack, check identical")
    ap.add_argument("cmd", nargs="?", choices=["unpack", "pack"])
    ap.add_argument("src", nargs="?")
    ap.add_argument("dst", nargs="?")
    args = ap.parse_args()

    if args.roundtrip:
        p = Path(args.roundtrip)
        files = sorted(f for f in p.iterdir() if f.suffix.upper() == ".SE") \
            if p.is_dir() else [p]
        results = [roundtrip(f) for f in files]
        sys.exit(0 if results and all(results) else 1)
    if not (args.cmd and args.src and args.dst):
        ap.error("need unpack|pack SRC DST, or --roundtrip PATH")
    if args.cmd == "unpack":
        parts = unpack(Path(args.src).read_bytes())
        write_dir(args.dst, *parts)
        print(f"wrote {args.dst}: events, vh, {len(parts[2])} VAGs, pack.json")
    else:
        out = pack(*read_dir(args.src))
        Path(args.dst).write_bytes(out)
        print(f"wrote {args.dst} ({len(out)} bytes)")


if __name__ == "__main__":
    main()
