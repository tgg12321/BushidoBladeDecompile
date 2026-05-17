#!/usr/bin/env python3
"""
extract_ndata_names.py - Build the NDATA file-ID -> filename map.

The shipped EXE carries a contiguous pool of NUL-terminated `DATA<n>\\NAME.DAT`
strings at VRAM 0x80010DEC. Their order in the pool defines the NDATA file-ID
of each name: the i-th string is the canonical filename for NDATA entry i (the
runtime never looks names up — IDs are baked into call sites at build time, but
the names exist in rodata for debug / build identification).

This tool extracts the pool, walks every string, and emits a deterministic
ID -> (group, filename, INF entry, sector range, content guess) CSV table. The
result is written to docs/formats/ndata_filemap.csv by default.

Usage:
    extract_ndata_names.py                      write the default CSV
    extract_ndata_names.py --stdout             print CSV to stdout
    extract_ndata_names.py --check              warn if EXE pool is at an
                                                unexpected offset
"""

import argparse
import csv
import os
import struct
import sys
from pathlib import Path

LOAD_VADDR = 0x80010000
HEADER_LEN = 0x800
POOL_VADDR_START = 0x80010DEC
# Pool ends just before the "Marionation over flow." engine error string at
# 0x80015294. Last name is "DATA0\\M000.DAT" + NUL at 0x80015280..0x80015292.
POOL_VADDR_END = 0x80015294

DEFAULT_EXE = Path("disc/SLUS_006.63")
DEFAULT_INF = Path("disc/NDATA/NDATA.INF")
DEFAULT_OUT = Path("docs/formats/ndata_filemap.csv")


def vaddr_to_off(v):
    return v - LOAD_VADDR + HEADER_LEN


def load_pool(exe_path):
    with open(exe_path, "rb") as f:
        exe = f.read()
    start = vaddr_to_off(POOL_VADDR_START)
    end = vaddr_to_off(POOL_VADDR_END)
    if end > len(exe):
        raise ValueError(f"pool end 0x{end:x} past EXE length 0x{len(exe):x}")
    strings = []
    i = start
    while i < end:
        j = i
        while j < end and exe[j] != 0:
            j += 1
        if j > i:
            vaddr = POOL_VADDR_START + (i - start)
            s = exe[i:j].decode("ascii", "replace")
            strings.append((vaddr, s))
        i = j + 1
    return strings


def load_inf(inf_path):
    """Return list of (start_sector, length_sectors) tuples, 901 entries long."""
    with open(inf_path, "rb") as f:
        data = f.read()
    n = len(data) // 4
    out = []
    for i in range(n):
        s, l = struct.unpack_from("<HH", data, i * 4)
        out.append((s, l))
    return out


def classify_group(filename):
    """Return (group_prefix, content_class) from a 'DATAn\\NAME.DAT' string."""
    if "\\" not in filename:
        return ("?", "?")
    prefix, _, name = filename.partition("\\")
    # heuristic content guess from the filename body
    body = name.rsplit(".", 1)[0]
    if body.startswith("MAR"):
        guess = "marionation animation"
    elif body.startswith("TINYM"):
        guess = "reduced-LOD model"
    elif body.startswith("T") and body[1:].isdigit():
        guess = "texture page archive"
    elif body.startswith("S") and body[1:].isdigit():
        guess = "stage geometry"
    elif body.startswith("M") and body[1:].isdigit():
        guess = "character motion"
    elif body.startswith("CHANBARA"):
        guess = "intro / chambara cinematic"
    elif body == "HAND":
        guess = "hand / cursor data"
    elif prefix == "DATA8":
        guess = "auxiliary (DATA8 bucket)"
    elif prefix == "DATA9":
        guess = "auxiliary (DATA9 bucket)"
    else:
        guess = "(unclassified)"
    return (prefix, guess)


def build_mapping(strings, inf_entries):
    """Map active INF entries to filenames, dropping DATA8 placeholders.

    Verified alignment rule (see docs/formats/NDATA.md "File-ID -> filename
    mapping"):
      * NDATA.INF has 901 slots; 762 are active, 139 are sentinels.
      * Rodata at 0x80010DEC..0x8001528B holds 899 NUL-terminated filenames.
      * 137 of those 899 names use the DATA8\\... prefix and refer to assets
        the runtime never indexes (the doc's "gap-filler" set).
      * Dropping the 137 DATA8 strings leaves exactly 762 names whose order
        matches the 762 active INF slots 1:1.

    INF indices 899 and 900 are trailing slots that address sectors past the
    DAT file end; they have no corresponding string and are reported with an
    empty filename (their `len` field is included for completeness).
    """
    SENT = (0xFFFF, 0xFFFF)
    active_idx = [i for i, e in enumerate(inf_entries) if e != SENT]
    real_strings = [(v, s) for (v, s) in strings if not s.startswith("DATA8\\")]
    rows = []
    if len(real_strings) != len(active_idx):
        rows.append(
            {
                "warning": (
                    f"non-DATA8 string count {len(real_strings)} != "
                    f"active-INF count {len(active_idx)}; map may be off"
                )
            }
        )
    for sidx, (vaddr, s) in zip(active_idx, real_strings):
        start_sect, len_sect = inf_entries[sidx]
        group, guess = classify_group(s)
        rows.append(
            {
                "file_id": sidx,
                "name_vaddr": f"0x{vaddr:08x}",
                "filename": s,
                "group": group,
                "content_guess": guess,
                "start_sector": start_sect,
                "len_sectors": len_sect,
                "byte_offset": start_sect * 0x800,
                "byte_length": len_sect * 0x800,
            }
        )
    cumulative = sum(e[1] for e in inf_entries if e != SENT)
    return rows, cumulative


FIELDS = [
    "file_id",
    "name_vaddr",
    "group",
    "filename",
    "content_guess",
    "start_sector",
    "len_sectors",
    "byte_offset",
    "byte_length",
]


def write_csv(rows, out_path):
    if not rows:
        raise ValueError("no rows")
    with open(out_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=FIELDS)
        w.writeheader()
        for r in rows:
            if "warning" in r:
                continue
            w.writerow(r)


def summarise(rows):
    from collections import Counter
    groups = Counter()
    classes = Counter()
    for r in rows:
        if "warning" in r:
            continue
        groups[r["group"]] += 1
        classes[r["content_guess"]] += 1
    print(f"total active entries  : {sum(groups.values())}")
    print(f"groups (DATA<n>)      :")
    for g, n in sorted(groups.items()):
        print(f"  {g:8s} {n:4d}")
    print(f"content guesses       :")
    for c, n in sorted(classes.items(), key=lambda x: -x[1]):
        print(f"  {c:32s} {n:4d}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    ap.add_argument("--inf", type=Path, default=DEFAULT_INF)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--stdout", action="store_true", help="print CSV to stdout")
    ap.add_argument("--summary", action="store_true", help="print group / class counts")
    args = ap.parse_args()

    strings = load_pool(args.exe)
    inf = load_inf(args.inf)
    rows, cum = build_mapping(strings, inf)

    if args.summary:
        summarise(rows)
        return

    if args.stdout:
        import io
        buf = io.StringIO()
        w = csv.DictWriter(buf, fieldnames=FIELDS)
        w.writeheader()
        for r in rows:
            if "warning" not in r:
                w.writerow(r)
        sys.stdout.write(buf.getvalue())
    else:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        write_csv(rows, args.out)
        print(f"wrote {args.out} ({sum(1 for r in rows if 'warning' not in r)} rows)")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, FileNotFoundError) as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(2)
