#!/usr/bin/env python3
"""
inspect_str.py - Inspect a PlayStation STR streaming-video file.

A STR file is a sequence of 2,048-byte data sectors (the data payload of
CD-XA Form 2 sectors with their 24-byte subheader stripped, as written by
tools/extract_iso.py). Each sector is either:
  * A video sector: first 32 bytes are the STR video header (control word
    starts with `60 01 01 80` in LE = 0x80010160).
  * An audio sector: 18 XA-ADPCM sound groups (128 bytes each).

See docs/formats/STR.md for the byte-level spec.

Usage:
    inspect_str.py file.str                  summary statistics
    inspect_str.py file.str --frames N       list first N video-frame headers
    inspect_str.py file.str --hex SECTOR     hex-dump one sector
"""

import argparse
import os
import struct
import sys
from pathlib import Path

SECTOR_SIZE = 2048
STR_MAGIC_CONTROL = 0x80010160


def is_video_sector(blob, off):
    if off + 32 > len(blob):
        return False
    control = struct.unpack_from("<I", blob, off)[0]
    return control == STR_MAGIC_CONTROL


def parse_video_header(blob, off):
    control = struct.unpack_from("<I", blob, off)[0]
    chunk_idx, chunks_total = struct.unpack_from("<HH", blob, off + 4)
    frame = struct.unpack_from("<I", blob, off + 8)[0]
    demux = struct.unpack_from("<I", blob, off + 12)[0]
    w, h = struct.unpack_from("<HH", blob, off + 16)
    mdec_half = struct.unpack_from("<H", blob, off + 20)[0]
    const = struct.unpack_from("<H", blob, off + 22)[0]
    q = struct.unpack_from("<H", blob, off + 24)[0]
    ver = struct.unpack_from("<H", blob, off + 26)[0]
    return {
        "control": control,
        "chunk": chunk_idx,
        "chunks_total": chunks_total,
        "frame": frame,
        "demuxed_size": demux,
        "width": w,
        "height": h,
        "mdec_codes": mdec_half * 2,
        "constant": const,
        "q_scale": q,
        "version": ver,
    }


def cmd_summary(path):
    size = os.path.getsize(path)
    n_sectors = size // SECTOR_SIZE
    rem = size % SECTOR_SIZE
    name = Path(path).name
    print(f"{name}  ({size} bytes)")
    print(f"  data sectors    : {n_sectors} (sector size {SECTOR_SIZE})")
    if rem:
        print(f"  trailing bytes  : {rem} (not sector-aligned)")
    # Walk sectors to classify
    n_video = 0
    n_audio = 0
    frames_seen = set()
    chunks_per_frame = None
    width = height = ver = q = None
    with open(path, "rb") as f:
        for s in range(n_sectors):
            f.seek(s * SECTOR_SIZE)
            head = f.read(32)
            if struct.unpack_from("<I", head, 0)[0] == STR_MAGIC_CONTROL:
                n_video += 1
                vh = parse_video_header(head, 0)
                frames_seen.add(vh["frame"])
                if chunks_per_frame is None:
                    chunks_per_frame = vh["chunks_total"]
                    width = vh["width"]
                    height = vh["height"]
                    ver = vh["version"]
                    q = vh["q_scale"]
            else:
                n_audio += 1
    print(f"  video sectors   : {n_video}")
    print(f"  audio sectors   : {n_audio}")
    if frames_seen:
        print(f"  video frames    : {len(frames_seen)} unique "
              f"(range {min(frames_seen)}..{max(frames_seen)})")
        print(f"  chunks/frame    : {chunks_per_frame}")
        print(f"  resolution      : {width}x{height} px")
        print(f"  MDEC version    : {ver}")
        print(f"  q_scale         : {q}")
    if n_video and n_audio:
        ratio = n_video / (n_video + n_audio)
        print(f"  video/audio ratio: {ratio:.3f} "
              f"({n_video}:{n_audio})")


def cmd_frames(path, n_max):
    size = os.path.getsize(path)
    n_sectors = size // SECTOR_SIZE
    name = Path(path).name
    print(f"{name}: first {n_max} video frame chunks:")
    print(f"  {'sector':>7}  {'frame':>5}  {'chunk':>5}/{'tot':<3}  "
          f"{'demux':>6}  {'w':>3}x{'h':<3}  q ver")
    count = 0
    with open(path, "rb") as f:
        for s in range(n_sectors):
            if count >= n_max:
                break
            f.seek(s * SECTOR_SIZE)
            head = f.read(32)
            if struct.unpack_from("<I", head, 0)[0] != STR_MAGIC_CONTROL:
                continue
            vh = parse_video_header(head, 0)
            print(
                f"  {s:>7}  {vh['frame']:>5}  "
                f"{vh['chunk']:>5}/{vh['chunks_total']:<3}  "
                f"{vh['demuxed_size']:>6}  "
                f"{vh['width']:>3}x{vh['height']:<3}  {vh['q_scale']} {vh['version']}"
            )
            count += 1


def cmd_hex(path, sec_idx, n=128):
    size = os.path.getsize(path)
    n_sectors = size // SECTOR_SIZE
    if not (0 <= sec_idx < n_sectors):
        sys.exit(f"sector {sec_idx} out of range 0..{n_sectors-1}")
    with open(path, "rb") as f:
        f.seek(sec_idx * SECTOR_SIZE)
        blob = f.read(min(n, SECTOR_SIZE))
    is_v = struct.unpack_from("<I", blob, 0)[0] == STR_MAGIC_CONTROL
    kind = "video" if is_v else "audio"
    print(f"Sector {sec_idx} of {Path(path).name} (= byte 0x{sec_idx*SECTOR_SIZE:x}, "
          f"{kind} sector); showing first {len(blob)} bytes:")
    for off in range(0, len(blob), 16):
        chunk = blob[off:off + 16]
        hp = " ".join(f"{b:02x}" for b in chunk)
        ap = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        print(f"  {off:08x}: {hp:<48s}  {ap}")


def main():
    ap = argparse.ArgumentParser(description="Inspect PSX STR streaming video")
    ap.add_argument("target", help="STR file")
    ap.add_argument("--frames", type=int, metavar="N",
                    help="List first N video frame headers")
    ap.add_argument("--hex", type=int, metavar="SECTOR",
                    help="Hex-dump a specific sector index")
    ap.add_argument("--bytes", type=int, default=128,
                    help="Bytes per --hex (default 128)")
    args = ap.parse_args()

    p = Path(args.target)
    if not p.is_file():
        sys.exit(f"{args.target}: not a file")

    if args.frames is not None:
        cmd_frames(p, args.frames)
    elif args.hex is not None:
        cmd_hex(p, args.hex, args.bytes)
    else:
        cmd_summary(p)


if __name__ == "__main__":
    main()
