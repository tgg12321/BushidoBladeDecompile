#!/usr/bin/env python3
"""
inspect_stage.py - Inspect BB2 stage / container .BIN files.

Three on-disc variants:
  * STG*.BIN     (U_PIC/, TIM2D/STG30..31) — 8-byte wrapper around 2 inline TIMs.
  * STAGE*.BIN   (LOADSE/, LOADSE1/)     — 52-byte header + 6 (offset, size) record pairs.
  * SEL/NAR/MOD/STAFF/D_SEL.BIN (TIM2D/) — 5-u32 outer TOC + secondary inner table.

See docs/formats/STAGE_BIN.md for the full format spec.

Usage:
    inspect_stage.py FILE.BIN                      auto-detect + summary
    inspect_stage.py FILE.BIN --dump OUTDIR        extract every record
    inspect_stage.py FILE.BIN --hex N              hex preview of record N
    inspect_stage.py DIR/                          one-line summary for every BIN in DIR
"""

import argparse
import os
import struct
import sys
from pathlib import Path


def detect_format(blob):
    """Return one of 'stg', 'stage', 'wrap5', 'unknown' plus context."""
    if len(blob) < 16:
        return ("unknown", {})
    w = struct.unpack_from("<II", blob, 0)
    # STG\*.BIN — 8-byte wrapper: (8, 0xB48) and an inline TIM at offset 8
    if w == (8, 0xB48) and len(blob) >= 12 and blob[8] == 0x10 and blob[9] == 0:
        return ("stg", {})
    # STAGE\*.BIN — header layout (13 u32s = 52 bytes):
    #   [hdr_size=0x34, size0, off1, size1, off2, size2, ..., off5, size5]
    # Record 0 implicitly starts at offset 0x34; records 1..5 use their off.
    if w[0] == 0x34 and len(blob) >= 0x34:
        words = struct.unpack_from("<" + "I" * 13, blob, 0)
        # words[0] = 0x34 (header size); words[1] = size of record 0;
        # words[2] = off of record 1; words[3] = size; etc.
        sizes = [words[1 + 2 * i] for i in range(6)]
        offs = [0x34] + [words[2 + 2 * i] for i in range(5)]
        ok = (
            all(0 < o <= len(blob) for o in offs)
            and all(offs[i] < offs[i + 1] for i in range(len(offs) - 1))
            and all(o + s <= len(blob) + 2048 for o, s in zip(offs, sizes))
        )
        if ok:
            sizes = [min(s, len(blob) - o) for o, s in zip(offs, sizes)]
            return ("stage", {"offs": offs, "sizes": sizes})
    # Generic "5 outer u32 + secondary inner table" container
    # Heuristic: first 5 u32s are large offsets, all <= file size
    if len(blob) >= 0x40:
        u32s = struct.unpack_from("<" + "I" * 5, blob, 0)
        if all(1024 < x <= len(blob) for x in u32s) and sorted(set(u32s)) == sorted(set(u32s)):
            # Add a hint: the inner sub-table starts at +0x14
            return ("wrap5", {"outer": u32s})
    return ("unknown", {})


def cmd_summary(path):
    with open(path, "rb") as f:
        blob = f.read()
    fmt, ctx = detect_format(blob)
    name = Path(path).name
    print(f"{name}  size={len(blob)}  format={fmt}")
    if fmt == "stg":
        print("  STG*.BIN 2-TIM wrapper")
        for n, off in enumerate((8, 0xB48), start=1):
            if blob[off] == 0x10 and blob[off + 1] == 0:
                flags = struct.unpack_from("<I", blob, off + 4)[0]
                pmode = flags & 3
                clut = bool((flags >> 3) & 1)
                print(
                    f"    TIM #{n} @ 0x{off:x}: PMODE={pmode} "
                    f"({['4-bit','8-bit','16-bit','24-bit'][pmode]}), "
                    f"CLUT={'yes' if clut else 'no'}"
                )
        return
    if fmt == "stage":
        print("  STAGE*.BIN 6-record container")
        offs, sizes = ctx["offs"], ctx["sizes"]
        for i, (o, s) in enumerate(zip(offs, sizes)):
            # Look at first 16 bytes of record to identify VAB/TIM/etc.
            head = blob[o:o + 4]
            note = ""
            # Records typically start with their own 16-word sub-TOC then a VAB
            # Check for pBAV anywhere in first 256 bytes
            scan = blob[o:o + 256]
            vab_off = scan.find(b"pBAV")
            if vab_off >= 0:
                note = f"  VAB @ +0x{vab_off:x}"
            print(
                f"    record {i}: off=0x{o:08x}  size=0x{s:x} "
                f"({s} B)  head={head.hex()}{note}"
            )
        return
    if fmt == "wrap5":
        print("  5-u32 TOC + secondary table (SEL/NAR/MOD/STAFF/D_SEL family)")
        for i, v in enumerate(ctx["outer"]):
            print(f"    +0x{i*4:02x}  outer_off[{i}] = 0x{v:08x} ({v})")
        # Show the inner table (9 u32s starting at 0x14)
        if len(blob) >= 0x40:
            print("    inner table (next 9 u32 entries):")
            for i in range(9):
                v = struct.unpack_from("<I", blob, 0x14 + i * 4)[0]
                print(f"      +0x{0x14 + i*4:02x}  inner[{i}] = 0x{v:08x} ({v})")
        return
    print("  (unknown layout) first 32 bytes:")
    print("  " + " ".join(f"{b:02x}" for b in blob[:32]))


def cmd_dump(path, outdir):
    with open(path, "rb") as f:
        blob = f.read()
    fmt, ctx = detect_format(blob)
    out = Path(outdir)
    out.mkdir(parents=True, exist_ok=True)
    stem = Path(path).stem
    if fmt == "stg":
        # Extract the two TIMs
        first = blob[8:0xB48]
        (out / f"{stem}_tim0.tim").write_bytes(first)
        second = blob[0xB48:]  # may include trailing residual data
        # Trim to expected TIM size by parsing
        end = _tim_end(second)
        if end is not None:
            tim2 = second[:end]
            (out / f"{stem}_tim1.tim").write_bytes(tim2)
            if len(second) > end:
                (out / f"{stem}_trailing.bin").write_bytes(second[end:])
                print(f"Wrote {stem}_tim0.tim, {stem}_tim1.tim, "
                      f"{stem}_trailing.bin into {out}")
                return
        else:
            (out / f"{stem}_tim1_raw.bin").write_bytes(second)
        print(f"Wrote 2 TIMs into {out}")
        return
    if fmt == "stage":
        for i, (o, s) in enumerate(zip(ctx["offs"], ctx["sizes"])):
            (out / f"{stem}_rec{i}.bin").write_bytes(blob[o:o + s])
        print(f"Wrote {len(ctx['offs'])} records into {out}")
        return
    if fmt == "wrap5":
        offs = list(ctx["outer"])
        # Treat outer pointers as segment starts; segment ends = next pointer or EOF
        boundaries = sorted(set(offs))
        if boundaries[-1] != len(blob):
            boundaries.append(len(blob))
        for i in range(len(boundaries) - 1):
            s = boundaries[i]
            e = boundaries[i + 1]
            if e > s:
                (out / f"{stem}_seg_{i:02d}.bin").write_bytes(blob[s:e])
        print(f"Wrote {len(boundaries)-1} segments into {out}")
        return
    sys.exit("Unknown layout — cannot extract")


def _tim_end(blob):
    """Return end offset of a TIM at blob[0..], or None."""
    if len(blob) < 12 or blob[0] != 0x10 or blob[1] != 0:
        return None
    flags = struct.unpack_from("<I", blob, 4)[0]
    has_clut = bool((flags >> 3) & 1)
    pos = 8
    if has_clut:
        if pos + 12 > len(blob):
            return None
        clut_len = struct.unpack_from("<I", blob, pos)[0]
        pos += clut_len
    if pos + 12 > len(blob):
        return None
    img_len = struct.unpack_from("<I", blob, pos)[0]
    return pos + img_len


def cmd_hex(path, sec_index, n=128):
    with open(path, "rb") as f:
        blob = f.read()
    fmt, ctx = detect_format(blob)
    if fmt == "stage":
        if not (0 <= sec_index < len(ctx["offs"])):
            sys.exit(f"--hex record index out of range 0..{len(ctx['offs'])-1}")
        o = ctx["offs"][sec_index]
        s = ctx["sizes"][sec_index]
        data = blob[o:o + min(n, s)]
        print(f"Record {sec_index} of {Path(path).name}: offset 0x{o:x} size {s}, "
              f"showing first {len(data)}")
    elif fmt == "stg":
        if not (1 <= sec_index <= 2):
            sys.exit("--hex section must be 1 or 2 for STG*.BIN")
        o = 8 if sec_index == 1 else 0xB48
        data = blob[o:o + n]
        print(f"TIM #{sec_index} of {Path(path).name} at 0x{o:x}, first {len(data)}:")
    elif fmt == "wrap5":
        # Treat sec_index as segment 0..N
        offs = sorted(set(ctx["outer"]))
        boundaries = offs + [len(blob)]
        if not (0 <= sec_index < len(boundaries) - 1):
            sys.exit(f"segment {sec_index} out of range")
        s = boundaries[sec_index]
        e = boundaries[sec_index + 1]
        data = blob[s:s + min(n, e - s)]
        print(f"Segment {sec_index} of {Path(path).name}: 0x{s:x}..0x{e:x}, "
              f"first {len(data)}")
    else:
        sys.exit("Unknown layout — cannot --hex")
    for off in range(0, len(data), 16):
        chunk = data[off:off + 16]
        hp = " ".join(f"{b:02x}" for b in chunk)
        ap = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        print(f"  {off:08x}: {hp:<48s}  {ap}")


def cmd_dir(dirpath):
    p = Path(dirpath)
    files = sorted({f.resolve() for f in p.glob("*.BIN")} |
                   {f.resolve() for f in p.glob("*.bin")})
    if not files:
        print(f"No *.BIN files in {dirpath}")
        return
    print(f"{'name':<24}{'size':>10}  format     notes")
    for fpath in files:
        try:
            with open(fpath, "rb") as f:
                blob = f.read(64)
        except Exception as e:
            print(f"{fpath.name:<24}  ERROR: {e}")
            continue
        with open(fpath, "rb") as f:
            full = f.read()
        fmt, ctx = detect_format(full)
        size = len(full)
        note = ""
        if fmt == "stage":
            note = f"6 records, total size 0x{sum(ctx['sizes']):x}"
        elif fmt == "wrap5":
            note = f"outer offs: " + ",".join(f"0x{v:x}" for v in ctx["outer"])
        elif fmt == "stg":
            note = "2-TIM container"
        print(f"{fpath.name:<24}{size:>10}  {fmt:<10}  {note}")


def main():
    ap = argparse.ArgumentParser(description="Inspect BB2 stage / container BINs")
    ap.add_argument("target", help="BIN file or directory")
    ap.add_argument("--dump", metavar="OUTDIR",
                    help="Extract every record/segment to OUTDIR")
    ap.add_argument("--hex", type=int, metavar="N",
                    help="Hex-dump record N (1- or 0-indexed depending on format)")
    ap.add_argument("--bytes", type=int, default=128,
                    help="Bytes for --hex (default 128)")
    args = ap.parse_args()

    p = Path(args.target)
    if p.is_dir():
        cmd_dir(p)
        return
    if not p.is_file():
        sys.exit(f"{args.target}: not a file or directory")

    if args.dump:
        cmd_dump(p, args.dump)
    elif args.hex is not None:
        cmd_hex(p, args.hex, args.bytes)
    else:
        cmd_summary(p)


if __name__ == "__main__":
    main()
