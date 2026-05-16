#!/usr/bin/env python3
"""
unpack_all.py - Walk disc/ and inspect every BB2 asset, dispatching to the
right inspect_*.py per file type.

For each file the tool prints one line: path, size, detected format, and a
short content summary. Failures don't abort the walk — they're printed and
the next file is processed.

Usage:
    unpack_all.py disc/                       walk every asset in disc/
    unpack_all.py disc/ --only NDATA,BBM      filter by format
    unpack_all.py disc/ --csv out.csv         dump CSV instead of human text
"""

import argparse
import csv
import os
import struct
import sys
from pathlib import Path


# ---------------------------------------------------------------------------
# Per-format probe functions. Each returns (format_label, summary_str) or
# (None, msg) if the file isn't of that format.
# ---------------------------------------------------------------------------

def probe_ndata(path, blob):
    """NDATA.INF / NDATA.DAT pair."""
    name = path.name
    if name.upper() == "NDATA.INF":
        if len(blob) % 4 != 0:
            return None, ""
        n_entries = len(blob) // 4
        n_sentinels = sum(
            1 for i in range(n_entries)
            if struct.unpack_from("<HH", blob, i * 4) == (0xFFFF, 0xFFFF)
        )
        return ("NDATA.INF",
                f"{n_entries} directory entries, {n_sentinels} sentinels")
    if name.upper() == "NDATA.DAT":
        size = path.stat().st_size
        return ("NDATA.DAT",
                f"{size} bytes ({size // 0x800} sectors)")
    return None, ""


def probe_bbm(path, blob):
    if len(blob) < 20:
        return None, ""
    magic = blob[:4]
    if magic[:2] not in (b"MC", b"MW"):
        return None, ""
    e1, e2, e3, e4 = struct.unpack_from("<IIII", blob, 4)
    # probe_bbm receives just the head bytes (up to 64 KB) — file size must
    # come from the filesystem, not len(blob).
    file_size = path.stat().st_size
    if any(off > file_size for off in (e1, e2, e3, e4)):
        return None, ""
    role = {b"MC": "common", b"MW": "character"}.get(magic[:2], "?")
    return ("BBM",
            f"{role}: sec1={e1-0x14}, sec5(bulk)={file_size-e4}")


def probe_tim(path, blob):
    if len(blob) < 12 or blob[0] != 0x10 or blob[1] != 0:
        return None, ""
    flags = struct.unpack_from("<I", blob, 4)[0]
    pmode = flags & 3
    has_clut = bool((flags >> 3) & 1)
    label = ["4-bit", "8-bit", "16-bit", "24-bit"][pmode]
    off = 8
    if has_clut:
        if off + 12 > len(blob):
            return None, ""
        clut_len = struct.unpack_from("<I", blob, off)[0]
        off += clut_len
    if off + 12 > len(blob):
        return None, ""
    ix, iy, iw, ih = struct.unpack_from("<HHHH", blob, off + 4)
    if pmode == 0:
        pw = iw * 4
    elif pmode == 1:
        pw = iw * 2
    elif pmode == 2:
        pw = iw
    else:
        pw = (iw * 2) // 3
    clut_str = "with CLUT" if has_clut else "no CLUT"
    return ("TIM",
            f"{label} {pw}x{ih}px @ VRAM({ix},{iy}), {clut_str}")


def probe_stage_bin(path, blob):
    if len(blob) < 0x34 or struct.unpack_from("<I", blob, 0)[0] != 0x34:
        return None, ""
    try:
        words = struct.unpack_from("<" + "I" * 13, blob, 0)
    except struct.error:
        return None, ""
    sizes = [words[1 + 2 * i] for i in range(6)]
    offs = [0x34] + [words[2 + 2 * i] for i in range(5)]
    file_size = path.stat().st_size
    if not (all(0 < o <= file_size for o in offs)
            and all(offs[i] < offs[i + 1] for i in range(5))
            and all(o + s <= file_size + 2048 for o, s in zip(offs, sizes))):
        return None, ""
    return ("STAGE.BIN",
            f"6 records, total payload 0x{sum(sizes):x} B")


def probe_stg_bin(path, blob):
    if len(blob) < 16:
        return None, ""
    if struct.unpack_from("<II", blob, 0) != (8, 0xB48):
        return None, ""
    if blob[8] != 0x10 or blob[9] != 0:
        return None, ""
    return ("STG.BIN", "2-TIM wrapper (stage select-screen thumbnail)")


def probe_se(path, blob):
    if len(blob) < 0x40:
        return None, ""
    sub_toc_count = struct.unpack_from("<I", blob, 0)[0]
    if not (4 <= sub_toc_count <= 256):
        return None, ""
    vh_off = struct.unpack_from("<I", blob, 4)[0]
    if vh_off + 4 > len(blob) or blob[vh_off:vh_off + 4] != b"pBAV":
        return None, ""
    nprogs_m1, ntones, nvags_m1 = struct.unpack_from(
        "<HHH", blob, vh_off + 0x12)
    return ("SE",
            f"VH @ 0x{vh_off:x}: {nprogs_m1+1}p/{ntones}t/{nvags_m1+1}v")


def probe_bnk(path, blob):
    # Only files actually named .BNK should match this (otherwise BIN
    # containers with similar ADPCM-shaped sub-blocks would mis-classify).
    if path.suffix.upper() != ".BNK":
        return None, ""
    file_size = path.stat().st_size
    if file_size % 16 != 0 or file_size == 0:
        return None, ""
    n_blocks = file_size // 16
    loop_ends = sum(1 for i in range(len(blob) // 16) if blob[i * 16 + 1] & 1)
    return ("BNK", f"{n_blocks} ADPCM blocks (~{loop_ends} sample boundaries "
                   f"in first {len(blob)} B)")


def probe_wrap5_bin(path, blob):
    """5-outer-u32 + secondary TOC container (SEL/NAR/MOD/STAFF/D_SEL)."""
    if len(blob) < 0x40:
        return None, ""
    file_size = path.stat().st_size
    u32s = struct.unpack_from("<IIIII", blob, 0)
    if not all(1024 < x <= file_size for x in u32s):
        return None, ""
    return ("WRAP5.BIN",
            f"5-outer-u32 TOC: offsets " + ",".join(f"0x{v:x}" for v in u32s))


def probe_str(path, blob):
    if len(blob) < 32 or len(blob) % 2048 != 0:
        return None, ""
    control = struct.unpack_from("<I", blob, 0)[0]
    if control != 0x80010160:
        return None, ""
    chunks_total = struct.unpack_from("<H", blob, 6)[0]
    w, h = struct.unpack_from("<HH", blob, 16)
    ver = struct.unpack_from("<H", blob, 26)[0]
    return ("STR",
            f"{w}x{h}px MDEC v{ver}, {chunks_total} chunks/frame, "
            f"{len(blob)//2048} sectors")


def probe_xa(path, blob):
    if not path.suffix.upper() == ".XA":
        return None, ""
    if len(blob) == 0 or len(blob) % 2048 != 0:
        return None, ""
    # XA sectors start with a valid SHDR group (sound group). The first
    # 16 bytes are 4 SHDR entries; for a valid SHDR the high nibble of byte
    # 0 (filter index) must be 0..4.
    sf = blob[0]
    if (sf >> 4) > 4:
        return None, ""
    return ("XA",
            f"{len(blob)//2048} sectors, raw XA-ADPCM stream")


def probe_psx_exe(path, blob):
    if blob[:8] == b"PS-X EXE":
        load = struct.unpack_from("<I", blob, 0x18)[0]
        entry = struct.unpack_from("<I", blob, 0x10)[0]
        sz = struct.unpack_from("<I", blob, 0x1C)[0]
        return ("PSX-EXE",
                f"load=0x{load:08x} entry=0x{entry:08x} text+data=0x{sz:x}")
    return None, ""


def probe_system_cnf(path, blob):
    if path.name.upper() == "SYSTEM.CNF":
        text = blob.decode("ascii", errors="replace").strip()
        # Single line summary
        return ("SYSTEM.CNF", text.replace("\r", " ").replace("\n", " | "))
    return None, ""


PROBES = [
    # Order matters: most specific first so files don't accidentally match
    # a less-specific probe (e.g. SE files start with bytes that look like a
    # TIM tag; STAGE.BIN files have ADPCM-shaped block patterns).
    probe_system_cnf,
    probe_psx_exe,
    probe_ndata,
    probe_bbm,        # MC/MW magic — distinct
    probe_str,        # STR magic 0x80010160 is very specific
    probe_stg_bin,    # exact 8-byte wrapper signature
    probe_stage_bin,  # 0x34 + 13-u32 TOC + record-end sanity
    probe_se,         # sub-TOC + pBAV at predicted offset
    probe_wrap5_bin,  # 5-u32 outer TOC heuristic (SEL/NAR/MOD/...)
    probe_tim,        # TIM tag 0x10 with valid flags
    probe_xa,         # by .XA extension + sector alignment
    probe_bnk,        # least specific: any 16-byte-block file with non-zero SF
]


def classify(path):
    """Return (format_label, summary_str). Reads as little as needed."""
    try:
        with open(path, "rb") as f:
            head = f.read(min(os.path.getsize(path), 65536))
        # Some probes need the full file (BNK loops, XA sectors)
        # — load the rest lazily only if needed by trying head-only first.
        for probe in PROBES:
            fmt, summary = probe(path, head)
            if fmt:
                # If only head was inspected, double-check by retrying with
                # full file for length-dependent classifiers
                if fmt in ("BNK", "XA", "STR"):
                    with open(path, "rb") as f:
                        full = f.read()
                    fmt2, summary2 = probe(path, full)
                    if fmt2:
                        return fmt2, summary2
                return fmt, summary
    except Exception as e:
        return ("ERROR", str(e))
    return ("UNKNOWN", "")


def walk(root, only=None):
    rows = []
    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        for fn in sorted(filenames):
            p = Path(dirpath) / fn
            # Skip symlinks (some disc trees contain a recursive disc -> .)
            try:
                if p.is_symlink():
                    continue
                rel = p.relative_to(root)
                size = p.stat().st_size
            except OSError:
                continue
            fmt, summary = classify(p)
            if only and fmt not in only:
                continue
            rows.append({
                "path": str(rel).replace("\\", "/"),
                "size": size,
                "format": fmt,
                "summary": summary,
            })
    return rows


def main():
    ap = argparse.ArgumentParser(
        description="Inventory every BB2 asset under disc/")
    ap.add_argument("root", help="Path to disc/ (the extracted disc tree)")
    ap.add_argument("--only", help="Comma-separated list of format labels")
    ap.add_argument("--csv", help="Write rows as CSV to this file")
    args = ap.parse_args()

    only = None
    if args.only:
        only = {x.strip().upper() for x in args.only.split(",")}

    if not Path(args.root).is_dir():
        sys.exit(f"{args.root}: not a directory")

    rows = walk(args.root, only=only)

    if args.csv:
        with open(args.csv, "w", newline="", encoding="utf-8") as g:
            w = csv.DictWriter(g, fieldnames=["path", "size", "format", "summary"])
            w.writeheader()
            for r in rows:
                w.writerow(r)
        print(f"Wrote {len(rows)} rows to {args.csv}")
        return

    # Pretty print
    print(f"{'path':<40}{'size':>10}  {'format':<10}  summary")
    print("-" * 100)
    for r in rows:
        print(f"{r['path']:<40}{r['size']:>10}  "
              f"{r['format']:<10}  {r['summary']}")

    # Tally
    print()
    tally = {}
    for r in rows:
        tally[r["format"]] = tally.get(r["format"], 0) + 1
    print("Totals by format:")
    for fmt in sorted(tally):
        print(f"  {fmt:<12} {tally[fmt]}")


if __name__ == "__main__":
    main()
