#!/usr/bin/env python3
"""
inspect_bnk.py - Inspect BB2 sound bank files (.BNK / .SE).

BB2 uses split VAB sound banks:
  * .BNK files = raw VAB body (concatenated SPU-ADPCM samples, no headers).
  * .SE files  = container (same layout as one STAGE record) with the
                 matching VH (pBAV header) embedded inside.

See docs/formats/BNK.md for the format spec.

Usage:
    inspect_bnk.py FILE.BNK              ADPCM block stats
    inspect_bnk.py FILE.SE               detect + dump VH summary
    inspect_bnk.py FILE.SE --vh OUT      extract just the VH bytes to OUT
    inspect_bnk.py DIR/                  list every BNK/SE in DIR
"""

import argparse
import os
import struct
import sys
from pathlib import Path


def parse_se(blob):
    """Detect SE container layout. Return None if not SE-like."""
    if len(blob) < 0x40:
        return None
    sub_toc_count = struct.unpack_from("<I", blob, 0)[0]
    if not (4 <= sub_toc_count <= 256):
        return None
    vab_off = struct.unpack_from("<I", blob, 4)[0]
    if vab_off + 4 > len(blob):
        return None
    if blob[vab_off:vab_off + 4] != b"pBAV":
        return None
    return {
        "sub_toc_count": sub_toc_count,
        "vab_offset": vab_off,
    }


def parse_vh(blob, off):
    if blob[off:off + 4] != b"pBAV":
        return None
    version, vab_id, fsize = struct.unpack_from("<III", blob, off + 4)
    reserved0, nprogs_m1, ntones, nvags_m1 = struct.unpack_from(
        "<HHHH", blob, off + 0x10)
    mvol, pan, attr1, attr2 = struct.unpack_from("<BBBB", blob, off + 0x18)
    return {
        "id": "pBAV",
        "version": version,
        "vab_id": vab_id,
        "fsize": fsize,
        "reserved0": reserved0,
        "nprogs": nprogs_m1 + 1,
        "ntones": ntones,
        "nvags": nvags_m1 + 1,
        "mvol": mvol, "pan": pan,
        "attr1": attr1, "attr2": attr2,
    }


def adpcm_block_stats(blob, start=0, max_blocks=8):
    """Sample the first N 16-byte SPU-ADPCM blocks."""
    out = []
    for i in range(max_blocks):
        off = start + i * 16
        if off + 16 > len(blob):
            break
        sf = blob[off]
        fl = blob[off + 1]
        out.append((off, sf, fl))
    return out


def cmd_summary(path):
    with open(path, "rb") as f:
        blob = f.read()
    name = Path(path).name
    se = parse_se(blob)
    if se:
        print(f"{name}  ({len(blob)} bytes) — SE container")
        print(f"  sub_TOC count : {se['sub_toc_count']}")
        print(f"  VH offset     : 0x{se['vab_offset']:x}")
        vh = parse_vh(blob, se["vab_offset"])
        if vh:
            print(f"  VH:")
            print(f"    version       : {vh['version']}")
            print(f"    vab_id        : {vh['vab_id']}")
            print(f"    fsize         : 0x{vh['fsize']:x} ({vh['fsize']})")
            print(f"    n programs    : {vh['nprogs']}")
            print(f"    n tones       : {vh['ntones']}")
            print(f"    n VAGs        : {vh['nvags']}")
            print(f"    mvol/pan      : {vh['mvol']} / {vh['pan']}")
            print(f"    attr1/attr2   : 0x{vh['attr1']:02x} / 0x{vh['attr2']:02x}")
        return
    # Try treat as raw VAB body
    print(f"{name}  ({len(blob)} bytes) — assumed VAB body (.BNK)")
    n_blocks = len(blob) // 16
    print(f"  16-byte block count : {n_blocks}")
    blocks = adpcm_block_stats(blob, start=0, max_blocks=8)
    print(f"  first 8 blocks (offset / shift_filter / flags):")
    for off, sf, fl in blocks:
        loop_end = bool(fl & 1)
        loop = bool(fl & 2)
        loop_start = bool(fl & 4)
        print(
            f"    0x{off:06x}  sf=0x{sf:02x} flags=0x{fl:02x} "
            f"({'LE' if loop_end else '  '}{'LP' if loop else '  '}"
            f"{'LS' if loop_start else '  '})"
        )
    # Count loop-end markers (= number of samples)
    loop_ends = sum(1 for i in range(n_blocks)
                    if blob[i * 16 + 1] & 1)
    print(f"  loop-end (sample boundary) markers seen: {loop_ends}")


def cmd_extract_vh(path, out_path):
    with open(path, "rb") as f:
        blob = f.read()
    se = parse_se(blob)
    if not se:
        sys.exit(f"{path}: not a SE container — no VH to extract")
    vh = parse_vh(blob, se["vab_offset"])
    if not vh:
        sys.exit(f"{path}: VH at offset 0x{se['vab_offset']:x} unreadable")
    vh_end_min = se["vab_offset"] + 0x20 + 128 * 16
    # We don't know the true VH end without inspecting the program/tone counts,
    # but for a safe extract we copy through fsize from the VH header.
    end = se["vab_offset"] + vh["fsize"]
    end = min(end, len(blob))
    with open(out_path, "wb") as g:
        g.write(blob[se["vab_offset"]:end])
    print(f"Wrote {out_path}  ({end - se['vab_offset']} bytes "
          f"of VH starting at 0x{se['vab_offset']:x})")


def cmd_dir(dirpath):
    p = Path(dirpath)
    files = sorted({f.resolve() for f in p.glob("*.BNK")} |
                   {f.resolve() for f in p.glob("*.bnk")} |
                   {f.resolve() for f in p.glob("*.SE")} |
                   {f.resolve() for f in p.glob("*.se")})
    if not files:
        print(f"No *.BNK / *.SE files in {dirpath}")
        return
    print(f"{'name':<24}{'size':>10}  format    notes")
    for fpath in files:
        with open(fpath, "rb") as f:
            blob = f.read()
        se = parse_se(blob)
        if se:
            vh = parse_vh(blob, se["vab_offset"])
            note = f"VH @ 0x{se['vab_offset']:x}, " \
                   f"{vh['nprogs']}p/{vh['ntones']}t/{vh['nvags']}v" \
                if vh else f"VH @ 0x{se['vab_offset']:x} (unreadable)"
            print(f"{fpath.name:<24}{len(blob):>10}  SE        {note}")
        else:
            print(f"{fpath.name:<24}{len(blob):>10}  BNK/body  "
                  f"{len(blob)//16} ADPCM blocks")


def main():
    ap = argparse.ArgumentParser(description="Inspect BB2 sound banks")
    ap.add_argument("target", help="BNK or SE file or directory")
    ap.add_argument("--vh", metavar="OUT",
                    help="Extract embedded VH from SE container to OUT")
    args = ap.parse_args()

    p = Path(args.target)
    if p.is_dir():
        cmd_dir(p)
        return
    if not p.is_file():
        sys.exit(f"{args.target}: not a file or directory")

    if args.vh:
        cmd_extract_vh(p, args.vh)
    else:
        cmd_summary(p)


if __name__ == "__main__":
    main()
