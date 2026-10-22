#!/usr/bin/env python3
"""
repack_tim.py - Convert BB2 TIM textures to editable PNGs and back.
Format in docs/formats/TIM.md; needs Pillow.

    repack_tim.py decode file.tim out.png [--clut N]
    repack_tim.py encode in.png out.tim
    repack_tim.py --roundtrip FILE.tim|DIR [--clut N]

`decode` writes the pixels as a PNG and every byte the pixels do not cover
(header words, CLUT block, image-block header, trailing bytes) as JSON in a
PNG text chunk `bb2tim`, so `encode` rebuilds the same TIM:

* 4/8-bit: an indexed PNG whose indices are the texel values and whose
  palette is CLUT row N (default 0). Palette entries edited in the PNG are
  written back to that row, each keeping its STP bit; the other rows come
  from the JSON.
* 16-bit: an RGBA PNG. RGB is the 5-bit colour expanded to 8 bits
  ((v << 3) | (v >> 2), so v = c >> 3 recovers it); alpha 0 = word 0x0000
  (transparent), 128 = STP set, 255 = STP clear. On encode alpha 0 gives
  0x0000, 1..254 sets STP, 255 clears it. The PNG must stay RGBA, and an
  opaque pixel darker than 8 per channel is an error (it would quantize to
  0x0000, i.e. transparent); use alpha 128 for opaque black (0x8000).
* 8-bit colours are cut to 5 bits (c >> 3) on encode. A palette entry
  edited to such a near-black with STP clear becomes 0x0000 (transparent).

The image may be resized: its header width/height follow the PNG, and
`block_length` is recomputed when the original was the standard
12 + data size (TITLE.TIM's non-standard value is kept only while the size
is unchanged). `--roundtrip` decodes and re-encodes in memory and checks the
result is byte-identical.
"""

import argparse
import io
import json
import struct
import sys
from pathlib import Path

from PIL import Image, PngImagePlugin

PX_PER_WORD = {0: 4, 1: 2, 2: 1}
META_KEY = "bb2tim"


def parse(data):
    try:
        return _parse(data)
    except struct.error as e:
        raise ValueError(f"truncated TIM ({e})")


def _parse(data):
    if len(data) < 20 or data[0] != 0x10:
        raise ValueError("not a TIM (tag != 0x10)")
    flags = struct.unpack_from("<I", data, 4)[0]
    pmode = flags & 3
    if pmode not in PX_PER_WORD:
        raise ValueError(f"PMODE {pmode} (24-bit) not supported")
    meta = {"header": data[:8].hex(), "clut": None}
    off = 8
    if flags & 8:
        blen = struct.unpack_from("<I", data, off)[0]
        dx, dy, w, h = struct.unpack_from("<HHHH", data, off + 4)
        words = list(struct.unpack_from(f"<{w * h}H", data, off + 12))
        meta["clut"] = {
            "block_length": blen, "dx": dx, "dy": dy, "w": w, "h": h,
            "words": words,
            "extra": data[off + 12 + w * h * 2:off + blen].hex(),
        }
        off += blen
    blen = struct.unpack_from("<I", data, off)[0]
    dx, dy, w, h = struct.unpack_from("<HHHH", data, off + 4)
    start = off + 12
    end = start + w * h * 2
    if end > len(data):
        raise ValueError("image data truncated")
    meta["image"] = {"block_length": blen, "dx": dx, "dy": dy, "w": w, "h": h}
    meta["trailing"] = data[end:].hex()
    return pmode, meta, w, h, data[start:end]


def rgb_of(word):
    r, g, b = word & 31, (word >> 5) & 31, (word >> 10) & 31
    return tuple((v << 3) | (v >> 2) for v in (r, g, b))


def word_of(r, g, b, stp):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | (stp << 15)


def to_png(data, clut_row=0):
    pmode, meta, w_words, h, pix = parse(data)
    width = w_words * PX_PER_WORD[pmode]
    if pmode == 2:
        out = bytearray()
        for (word,) in struct.iter_unpack("<H", pix):
            if word == 0:
                out += b"\0\0\0\0"
            else:
                out += bytes(rgb_of(word)) + bytes([128 if word >> 15 else 255])
        img = Image.frombytes("RGBA", (width, h), bytes(out))
    else:
        if meta["clut"] is None:
            raise ValueError("indexed TIM without a CLUT")
        clut = meta["clut"]
        if not 0 <= clut_row < clut["h"]:
            raise ValueError(f"--clut {clut_row} out of range (0..{clut['h'] - 1})")
        if pmode == 0:
            idx = bytearray()
            for b in pix:
                idx += bytes((b & 15, b >> 4))
        else:
            idx = pix
        img = Image.frombytes("P", (width, h), bytes(idx))
        row = clut["words"][clut_row * clut["w"]:(clut_row + 1) * clut["w"]]
        pal = [c for word in row for c in rgb_of(word)]
        img.putpalette(pal + [0] * (768 - len(pal)))
        meta["clut_row"] = clut_row
    meta["pmode"] = pmode
    info = PngImagePlugin.PngInfo()
    info.add_text(META_KEY, json.dumps(meta, separators=(",", ":")))
    buf = io.BytesIO()
    img.save(buf, "PNG", pnginfo=info)
    return buf.getvalue()


def from_png(png):
    img = Image.open(io.BytesIO(png))
    if META_KEY not in img.info:
        raise ValueError(f"PNG has no '{META_KEY}' text chunk (not from decode)")
    meta = json.loads(img.info[META_KEY])
    pmode = meta["pmode"]
    width, h = img.size
    ppw = PX_PER_WORD[pmode]
    if width % ppw:
        raise ValueError(f"width {width} not a multiple of {ppw} for PMODE {pmode}")
    w_words = width // ppw
    out = bytearray(bytes.fromhex(meta["header"]))
    if pmode == 2:
        pix = bytearray()
        if img.mode != "RGBA":
            raise ValueError(f"16-bit TIM needs an RGBA PNG, got mode {img.mode} "
                             "(the alpha channel carries STP)")
        rgba = img.tobytes()
        for i in range(0, len(rgba), 4):
            r, g, b, a = rgba[i:i + 4]
            word = 0 if a == 0 else word_of(r, g, b, 1 if a < 255 else 0)
            if word == 0 and a:
                n = i // 4
                raise ValueError(f"pixel ({n % width},{n // width}): opaque "
                                 "near-black would encode as 0x0000 "
                                 "(transparent); use alpha 128 for black")
            pix += struct.pack("<H", word)
    else:
        if img.mode != "P":
            raise ValueError(f"indexed TIM needs a palette PNG, got mode {img.mode}")
        idx = img.tobytes()
        if max(idx, default=0) >= (16 if pmode == 0 else 256):
            raise ValueError("palette index out of range for PMODE")
        if pmode == 0:
            pix = bytes(idx[i] | (idx[i + 1] << 4) for i in range(0, len(idx), 2))
        else:
            pix = idx
    if meta["clut"] is not None:
        clut = meta["clut"]
        words = list(clut["words"])
        if "clut_row" in meta:
            base = meta["clut_row"] * clut["w"]
            pal = img.getpalette() or []
            for i in range(clut["w"]):
                rgb = tuple(pal[3 * i:3 * i + 3])
                old = words[base + i]
                if len(rgb) == 3 and rgb != rgb_of(old):
                    words[base + i] = word_of(*rgb, old >> 15)
        extra = bytes.fromhex(clut["extra"])
        out += struct.pack("<IHHHH", clut["block_length"], clut["dx"],
                           clut["dy"], clut["w"], clut["h"])
        out += struct.pack(f"<{len(words)}H", *words) + extra
    im = meta["image"]
    trailing = bytes.fromhex(meta["trailing"])
    blen = im["block_length"]
    standard = blen == 12 + im["w"] * im["h"] * 2
    if standard or (w_words, h) != (im["w"], im["h"]):
        blen = 12 + len(pix)
    out += struct.pack("<IHHHH", blen, im["dx"], im["dy"], w_words, h)
    out += pix + trailing
    return bytes(out)


def roundtrip(path, clut_row):
    data = Path(path).read_bytes()
    try:
        back = from_png(to_png(data, clut_row))
    except ValueError as e:
        print(f"{Path(path).name}: ERROR {e}")
        return False
    ok = back == data
    print(f"{Path(path).name}: {'identical' if ok else 'DIFFERS'} ({len(data)} bytes)")
    return ok


def main():
    ap = argparse.ArgumentParser(description="BB2 TIM <-> PNG")
    ap.add_argument("--roundtrip", metavar="PATH",
                    help="TIM file or directory: decode+encode, check identical")
    ap.add_argument("--clut", type=int, default=0, metavar="N",
                    help="CLUT row used as the PNG palette (default 0)")
    ap.add_argument("cmd", nargs="?", choices=["decode", "encode"])
    ap.add_argument("src", nargs="?")
    ap.add_argument("dst", nargs="?")
    args = ap.parse_args()

    if args.roundtrip:
        p = Path(args.roundtrip)
        files = sorted(f for f in p.iterdir() if f.suffix.upper() == ".TIM") \
            if p.is_dir() else [p]
        results = [roundtrip(f, args.clut) for f in files]
        sys.exit(0 if results and all(results) else 1)
    if not (args.cmd and args.src and args.dst):
        ap.error("need decode|encode SRC DST, or --roundtrip PATH")
    src = Path(args.src).read_bytes()
    out = to_png(src, args.clut) if args.cmd == "decode" else from_png(src)
    Path(args.dst).write_bytes(out)
    print(f"wrote {args.dst} ({len(out)} bytes)")


if __name__ == "__main__":
    main()
