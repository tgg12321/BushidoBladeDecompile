#!/usr/bin/env python3
"""
repack_bnk.py - Split a BB2 `.BNK` (disc/TIM2D/GAME*.BNK) into the two VRAM
rectangles it fills and rebuild it. Format in docs/formats/BNK.md.

    repack_bnk.py unpack GAME.BNK OUTDIR
    repack_bnk.py pack DIR out.bnk
    repack_bnk.py --roundtrip FILE.BNK|DIR

A BNK is raw 16-bit VRAM data with no header. eff_LoadImagesAndSetup
(src/main/51268.c) reads it and uploads it with two LoadImage calls:
bytes 0..0x1DC00 to the rect (896, 0) 128 x 476 and the rest to
(896, 476) 112 x 36. `unpack` writes each rect as a 16-bit TIM (no CLUT)
carrying those coordinates:

    main.tim     (896, 0)   128 x 476
    strip.tim    (896, 476) 112 x 36

so tools/repack_tim.py can turn them into PNGs and back. A 16-bit PNG of
VRAM is a raw view: the textures stored there are drawn through texture
pages and CLUTs, not as 16-bit colour. `pack` requires both TIMs to keep
their rects, since the loader's are fixed. `--roundtrip` unpacks and
repacks in memory and checks byte identity.
"""

import argparse
import struct
import sys
from pathlib import Path

# (name, x, y, w, h) in VRAM halfwords, from eff_LoadImagesAndSetup.
RECTS = [("main", 0x380, 0, 0x80, 0x1DC), ("strip", 0x380, 0x1DC, 0x70, 0x24)]
SIZE = sum(w * h * 2 for _n, _x, _y, w, h in RECTS)  # 129,920


def tim16(x, y, w, h, pix):
    """A 16-bit TIM with no CLUT holding pix at VRAM (x, y)."""
    return struct.pack("<II", 0x10, 2) + \
        struct.pack("<I4H", 12 + len(pix), x, y, w, h) + pix


def untim16(data, name):
    """Returns (x, y, w, h, pixels) of a 16-bit TIM with no CLUT."""
    if len(data) < 20:
        raise ValueError(f"{name}: truncated TIM")
    tag, flags = struct.unpack_from("<II", data, 0)
    if tag != 0x10 or flags != 2:
        raise ValueError(f"{name}: not a 16-bit TIM without CLUT "
                         f"(tag 0x{tag:x}, flags 0x{flags:x})")
    bnum, x, y, w, h = struct.unpack_from("<I4H", data, 8)
    pix = data[20:]
    if bnum != 12 + w * h * 2 or len(pix) != w * h * 2:
        raise ValueError(f"{name}: block length {bnum} / {len(pix)} pixel "
                         f"bytes do not match {w} x {h}")
    return x, y, w, h, pix


def unpack(data):
    """Returns {name: TIM bytes}."""
    if len(data) != SIZE:
        raise ValueError(f"{len(data)} bytes, a BNK is {SIZE}")
    out, off = {}, 0
    for name, x, y, w, h in RECTS:
        n = w * h * 2
        out[name] = tim16(x, y, w, h, data[off:off + n])
        off += n
    return out


def pack(tims):
    parts = []
    for name, x, y, w, h in RECTS:
        got = untim16(tims[name], name)
        if got[:4] != (x, y, w, h):
            raise ValueError(f"{name}: rect {got[:4]}, the loader uploads "
                             f"{(x, y, w, h)}")
        parts.append(got[4])
    return b"".join(parts)


def roundtrip(path):
    try:
        data = Path(path).read_bytes()
        back = pack(unpack(data))
    except (OSError, ValueError) as e:
        print(f"{Path(path).name}: ERROR {e}")
        return False
    ok = back == data
    print(f"{Path(path).name}: {'identical' if ok else 'DIFFERS'} "
          f"({len(data)} bytes)")
    return ok


def main():
    ap = argparse.ArgumentParser(description="BB2 .BNK (VRAM rects) unpack/pack")
    ap.add_argument("--roundtrip", metavar="PATH",
                    help=".BNK file or directory: unpack+pack, check identical")
    ap.add_argument("cmd", nargs="?", choices=["unpack", "pack"])
    ap.add_argument("src", nargs="?")
    ap.add_argument("dst", nargs="?")
    args = ap.parse_args()

    if args.roundtrip:
        p = Path(args.roundtrip)
        if not p.exists():
            sys.exit(f"error: {p} does not exist")
        files = sorted(f for f in p.iterdir()
                       if f.is_file() and f.suffix.upper() == ".BNK") \
            if p.is_dir() else [p]
        if not files:
            sys.exit(f"error: no .BNK files in {p}")
        results = [roundtrip(f) for f in files]
        sys.exit(0 if results and all(results) else 1)
    if not (args.cmd and args.src and args.dst):
        ap.error("need unpack|pack SRC DST, or --roundtrip PATH")
    try:
        if args.cmd == "unpack":
            tims = unpack(Path(args.src).read_bytes())
            out = Path(args.dst)
            out.mkdir(parents=True, exist_ok=True)
            for name, t in tims.items():
                (out / f"{name}.tim").write_bytes(t)
            print(f"wrote {args.dst}: " + ", ".join(f"{n}.tim" for n in tims))
        else:
            src = Path(args.src)
            data = pack({n: (src / f"{n}.tim").read_bytes()
                         for n, *_r in RECTS})
            Path(args.dst).write_bytes(data)
            print(f"wrote {args.dst} ({len(data)} bytes)")
    except (OSError, ValueError) as e:
        sys.exit(f"error: {e}")


if __name__ == "__main__":
    main()
