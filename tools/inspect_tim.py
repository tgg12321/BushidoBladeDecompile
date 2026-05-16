#!/usr/bin/env python3
"""
inspect_tim.py - Inspect PlayStation TIM image files.

TIM is the PSX SDK standard texture-image format. BB2 uses TIM unchanged for
all 2D textures (laser sprites, title screen, UI graphics). See
docs/formats/TIM.md for the byte-level format.

Usage:
    inspect_tim.py file.tim                  one-line summary
    inspect_tim.py file.tim --verbose        full header dump
    inspect_tim.py file.tim --raw OUT.bin    dump raw image bytes
    inspect_tim.py file.tim --png OUT.png    decode to PNG (needs Pillow)
    inspect_tim.py DIR/                      list every TIM in a directory
"""

import argparse
import os
import struct
import sys
from pathlib import Path

PMODE_LABEL = ["4-bit", "8-bit", "16-bit", "24-bit"]


def parse_tim(path):
    with open(path, "rb") as f:
        data = f.read()
    if len(data) < 12:
        raise ValueError(f"{path}: too short ({len(data)} bytes)")
    tag = data[0]
    ver = data[1]
    if tag != 0x10:
        raise ValueError(f"{path}: bad TIM tag 0x{tag:x} (expected 0x10)")
    if ver != 0x00:
        raise ValueError(f"{path}: unknown TIM version 0x{ver:x}")
    flags = struct.unpack_from("<I", data, 4)[0]
    pmode = flags & 3
    has_clut = bool((flags >> 3) & 1)

    info = {
        "size": len(data),
        "tag": tag,
        "version": ver,
        "flags": flags,
        "pmode": pmode,
        "pmode_label": PMODE_LABEL[pmode],
        "has_clut": has_clut,
    }

    off = 8
    if has_clut:
        if off + 12 > len(data):
            raise ValueError(f"{path}: truncated CLUT header")
        clut_len = struct.unpack_from("<I", data, off)[0]
        cx, cy, cw, ch = struct.unpack_from("<HHHH", data, off + 4)
        info["clut"] = {
            "block_length": clut_len,
            "dx": cx, "dy": cy,
            "width_entries": cw,
            "n_clut": ch,
            "data_offset": off + 12,
            "data_size": cw * ch * 2,
        }
        off += clut_len
    else:
        info["clut"] = None

    if off + 12 > len(data):
        raise ValueError(f"{path}: truncated image header")
    img_len = struct.unpack_from("<I", data, off)[0]
    ix, iy, iw, ih = struct.unpack_from("<HHHH", data, off + 4)
    info["image"] = {
        "block_length": img_len,
        "dx": ix, "dy": iy,
        "width_words": iw,
        "height": ih,
        "data_offset": off + 12,
        "data_size_expected": iw * ih * 2,
    }
    # Pixel-domain width depends on PMODE
    if pmode == 0:
        info["image"]["pixel_width"] = iw * 4
    elif pmode == 1:
        info["image"]["pixel_width"] = iw * 2
    elif pmode == 2:
        info["image"]["pixel_width"] = iw
    elif pmode == 3:
        info["image"]["pixel_width"] = (iw * 2) // 3

    info["_blob"] = data
    return info


def _summary_line(name, info):
    img = info["image"]
    clut = info["clut"]
    if clut:
        clut_str = f"clut@({clut['dx']},{clut['dy']}) " \
                   f"{clut['width_entries']}x{clut['n_clut']}"
    else:
        clut_str = "no-clut"
    return (
        f"{name:<26}{info['size']:>8}  {info['pmode_label']:<6} "
        f"{clut_str:<22}  img@({img['dx']},{img['dy']}) "
        f"{img['pixel_width']}x{img['height']}px  "
        f"img_data={img['data_size_expected']}B"
    )


def cmd_summary(path, verbose=False):
    info = parse_tim(path)
    name = Path(path).name
    if not verbose:
        print(_summary_line(name, info))
        return
    print(f"{name}  ({info['size']} bytes)")
    print(f"  tag           : 0x{info['tag']:02x}")
    print(f"  version       : 0x{info['version']:02x}")
    print(f"  flags         : 0x{info['flags']:08x}  "
          f"(PMODE={info['pmode']}={info['pmode_label']}, "
          f"CLUT={'yes' if info['has_clut'] else 'no'})")
    if info["clut"]:
        c = info["clut"]
        print("  clut block    :")
        print(f"    block_length  : 0x{c['block_length']:x} ({c['block_length']})")
        print(f"    dx, dy        : ({c['dx']}, {c['dy']})")
        print(f"    width (16b)   : {c['width_entries']}")
        print(f"    n_clut (rows) : {c['n_clut']}")
        print(f"    data offset   : 0x{c['data_offset']:x}, size {c['data_size']} bytes")
    img = info["image"]
    print("  image block   :")
    print(f"    block_length  : 0x{img['block_length']:x} ({img['block_length']})")
    print(f"    dx, dy        : ({img['dx']}, {img['dy']})")
    print(f"    width (16b)   : {img['width_words']}")
    print(f"    height        : {img['height']}")
    print(f"    pixel width   : {img['pixel_width']}")
    print(f"    data offset   : 0x{img['data_offset']:x}, "
          f"expected size {img['data_size_expected']} bytes")
    actual_image_bytes = info["size"] - img["data_offset"]
    print(f"    actual bytes after header: {actual_image_bytes}")
    if actual_image_bytes != img["data_size_expected"]:
        print(f"    NOTE: file size disagrees with W*H*2 — non-standard "
              f"layout, file may have padding or anomalous block_length")


def cmd_raw(path, out_path):
    info = parse_tim(path)
    img = info["image"]
    blob = info["_blob"]
    nbytes = img["data_size_expected"]
    pixel_bytes = blob[img["data_offset"]:img["data_offset"] + nbytes]
    with open(out_path, "wb") as g:
        g.write(pixel_bytes)
    print(f"Wrote {out_path}  ({nbytes} bytes of {info['pmode_label']} image data)")


def _bgr555_to_rgb888(word):
    r = (word & 0x1F) << 3
    g = ((word >> 5) & 0x1F) << 3
    b = ((word >> 10) & 0x1F) << 3
    return r, g, b


def cmd_png(path, out_path, clut_index=0):
    try:
        from PIL import Image
    except ImportError:
        sys.exit("--png requires Pillow (pip install Pillow)")
    info = parse_tim(path)
    blob = info["_blob"]
    img = info["image"]
    pmode = info["pmode"]
    pw = img["pixel_width"]
    ph = img["height"]
    img_off = img["data_offset"]

    if pmode in (0, 1):
        if not info["has_clut"]:
            sys.exit(f"PMODE {pmode} requires a CLUT but none found")
        c = info["clut"]
        if not (0 <= clut_index < c["n_clut"]):
            sys.exit(f"--clut index {clut_index} out of 0..{c['n_clut']-1}")
        clut_row_size = c["width_entries"] * 2
        clut_off = c["data_offset"] + clut_index * clut_row_size
        palette = []
        for i in range(c["width_entries"]):
            w = struct.unpack_from("<H", blob, clut_off + i * 2)[0]
            palette.append(_bgr555_to_rgb888(w))

        # Compute actual available image bytes (may be truncated)
        max_image_bytes = len(blob) - img_off
        # Truncate height to fit available data
        bytes_per_row = img["width_words"] * 2
        max_rows = max_image_bytes // bytes_per_row
        if max_rows < ph:
            print(f"  NOTE: image data truncated to {max_rows} rows "
                  f"(header claims {ph})", file=sys.stderr)
            ph = max_rows

        if pmode == 0:
            pixels = bytearray()
            for row in range(ph):
                row_off = img_off + row * bytes_per_row
                for word_idx in range(img["width_words"]):
                    w = struct.unpack_from("<H", blob, row_off + word_idx * 2)[0]
                    pixels.append(w & 0xF)
                    pixels.append((w >> 4) & 0xF)
                    pixels.append((w >> 8) & 0xF)
                    pixels.append((w >> 12) & 0xF)
        else:  # pmode 1, 8-bit
            pixels = bytearray()
            for row in range(ph):
                row_off = img_off + row * bytes_per_row
                for word_idx in range(img["width_words"]):
                    w = struct.unpack_from("<H", blob, row_off + word_idx * 2)[0]
                    pixels.append(w & 0xFF)
                    pixels.append((w >> 8) & 0xFF)
        out = Image.new("RGB", (pw, ph))
        out_pixels = out.load()
        for y in range(ph):
            for x in range(pw):
                idx = pixels[y * pw + x]
                if idx < len(palette):
                    out_pixels[x, y] = palette[idx]
                else:
                    out_pixels[x, y] = (255, 0, 255)  # invalid palette entry
        out.save(out_path)
    elif pmode == 2:
        out = Image.new("RGB", (pw, ph))
        out_pixels = out.load()
        for y in range(ph):
            for x in range(pw):
                w = struct.unpack_from("<H", blob, img_off + (y * pw + x) * 2)[0]
                out_pixels[x, y] = _bgr555_to_rgb888(w)
        out.save(out_path)
    elif pmode == 3:
        out = Image.new("RGB", (pw, ph))
        out_pixels = out.load()
        for y in range(ph):
            row_off = img_off + y * img["width_words"] * 2
            for x in range(pw):
                b = blob[row_off + x * 3 + 0]
                g = blob[row_off + x * 3 + 1]
                r = blob[row_off + x * 3 + 2]
                out_pixels[x, y] = (r, g, b)
        out.save(out_path)
    else:
        sys.exit(f"PMODE {pmode}: unknown")
    print(f"Wrote {out_path}  ({pw}x{ph} px, {info['pmode_label']})")


def cmd_dir(dirpath):
    p = Path(dirpath)
    files = sorted({f.resolve() for f in p.glob("*.TIM")} |
                   {f.resolve() for f in p.glob("*.tim")})
    if not files:
        print(f"No *.TIM files in {dirpath}")
        return
    print(
        f"{'name':<26}{'size':>8}  {'pmode':<6} "
        f"{'clut':<22}  {'image':<22}  img_bytes"
    )
    for fpath in files:
        try:
            info = parse_tim(fpath)
            print(_summary_line(fpath.name, info))
        except ValueError as e:
            print(f"{fpath.name:<26}  ERROR: {e}")


def main():
    ap = argparse.ArgumentParser(description="Inspect PSX TIM files")
    ap.add_argument("target", help="TIM file or directory")
    ap.add_argument("--verbose", "-v", action="store_true",
                    help="Full header dump (default: one-line summary)")
    ap.add_argument("--raw", metavar="OUT",
                    help="Dump raw image pixel bytes to OUT")
    ap.add_argument("--png", metavar="OUT",
                    help="Decode to PNG (requires Pillow)")
    ap.add_argument("--clut", type=int, default=0,
                    help="CLUT index for --png (default 0)")
    args = ap.parse_args()

    p = Path(args.target)
    if p.is_dir():
        cmd_dir(p)
        return
    if not p.is_file():
        sys.exit(f"{args.target}: not a file or directory")

    if args.raw:
        cmd_raw(p, args.raw)
    elif args.png:
        cmd_png(p, args.png, args.clut)
    else:
        cmd_summary(p, args.verbose)


if __name__ == "__main__":
    main()
