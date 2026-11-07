#!/usr/bin/env python3
"""
extract_ndata_names.py - Build the NDATA file-ID -> filename map.

The shipped EXE carries a contiguous pool of 899 NUL-terminated
`DATA<n>\\NAME.DAT` strings at VRAM 0x80010DEC, written last-id-first: INF
slot i (0..898) is named by pool string 898 - i, sentinels included (rule and
evidence: docs/formats/NDATA.md "Pool-to-INF rule"). The runtime never looks
names up; every caller passes a numeric id.

This tool extracts the pool and emits one deterministic CSV row per INF slot
(0..900): name, group, sentinel flag, sector range, content class. Slots 899
and 900 have no name. It refuses to write if the pool is not 899 strings, the
code's id bases (ID_BASES) do not land on their group's first name, or the
EXE's name pointer table D_800955E0 disagrees with any slot's name. The
result is written to docs/formats/ndata_filemap.csv by default.

Usage:
    extract_ndata_names.py                      write the default CSV
    extract_ndata_names.py --stdout             print CSV to stdout
    extract_ndata_names.py --summary            print slot / group / class counts
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
# Pool ends where the "Marionation over flow." engine error string starts.
# Last name is "DATA0\\M000.DAT" + NUL at 0x8001527C..0x8001528B.
POOL_VADDR_END = 0x8001528C

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
    # content class from the filename body (docs/formats/NDATA.md "File-ID layout")
    body = name.rsplit(".", 1)[0]
    if body.startswith("MAR"):
        guess = "motion playback file"
    elif body.startswith("TINYM"):
        guess = "tiny model (grayscale)"
    elif body.startswith("T") and body[1:].isdigit():
        guess = "tiny model"
    elif body.startswith("S") and body[1:].isdigit():
        guess = "stage"
    elif body.startswith("M") and body[1:].isdigit():
        guess = "slot asset"
    elif body.startswith("CHANBARA"):
        guess = "DATA0 pack block"
    elif body.startswith("WEP"):
        guess = "weapon pair"
    else:
        guess = "(unclassified)"
    return (prefix, guess)


SENTINEL = (0xFFFF, 0xFFFF)
INF_SLOTS = 901
NAMED_SLOTS = 899  # slots 0..898; 899 and 900 have no pool string


def build_mapping(strings, inf_entries):
    """One row per INF slot; slot i is named by pool string 898 - i.

    See docs/formats/NDATA.md "Pool-to-INF rule". Sentinel slots (0xFFFF,
    0xFFFF) keep their name (a file the build left out) with sentinel=1 and
    no byte range. Slots 899 and 900 have no name.
    """
    if len(strings) != NAMED_SLOTS:
        raise ValueError(f"pool has {len(strings)} strings, expected {NAMED_SLOTS}")
    if len(inf_entries) != INF_SLOTS:
        raise ValueError(f"INF has {len(inf_entries)} slots, expected {INF_SLOTS}")
    rows = []
    for sidx, (start_sect, len_sect) in enumerate(inf_entries):
        sentinel = (start_sect, len_sect) == SENTINEL
        if sidx < NAMED_SLOTS:
            vaddr, s = strings[NAMED_SLOTS - 1 - sidx]
            group, guess = classify_group(s)
            name_vaddr = f"0x{vaddr:08x}"
        else:
            s, group, guess, name_vaddr = "", "", "(no name; leftover slot)", ""
        rows.append(
            {
                "file_id": sidx,
                "name_vaddr": name_vaddr,
                "filename": s,
                "group": group,
                "content_guess": guess,
                "sentinel": int(sentinel),
                "start_sector": start_sect,
                "len_sectors": len_sect,
                "byte_offset": "" if sentinel else start_sect * 0x800,
                "byte_length": "" if sentinel else len_sect * 0x800,
            }
        )
    return rows


# Code id bases (docs/formats/NDATA.md) and the name each must land on.
ID_BASES = {
    0x25: "DATA0\\CHANBARA0.DAT",
    0x27: "DATA2\\S00.DAT",
    0x4D: "DATA5\\T00.DAT",
    0x68: "DATA6\\TINYM00.DAT",
    0x83: "DATA8\\WEP0000.DAT",
    0x10C: "DATA9\\WEP0000.DAT",
    0x131: "DATA7\\MAR00_00.DAT",
}


def check_bases(rows):
    bad = [
        f"0x{i:x}: {rows[i]['filename']!r} != {want!r}"
        for i, want in ID_BASES.items()
        if rows[i]["filename"] != want
    ]
    if bad:
        raise ValueError("id bases off: " + "; ".join(bad))


# D_800955E0 (src/main/35000.c): 899 pointers into the pool, entry i -> the name
# of INF slot i. The EXE's own statement of the reverse rule.
NAME_TABLE_VADDR = 0x800955E0


def check_name_table(exe_path, rows):
    with open(exe_path, "rb") as f:
        exe = f.read()
    off = vaddr_to_off(NAME_TABLE_VADDR)
    bad = [
        i
        for i in range(NAMED_SLOTS)
        if f"0x{struct.unpack_from('<I', exe, off + 4 * i)[0]:08x}"
        != rows[i]["name_vaddr"]
    ]
    if bad:
        raise ValueError(f"name table D_800955E0 disagrees at slots {bad[:8]}")


FIELDS = [
    "file_id",
    "name_vaddr",
    "group",
    "filename",
    "content_guess",
    "sentinel",
    "start_sector",
    "len_sectors",
    "byte_offset",
    "byte_length",
]


def write_csv(rows, out_path):
    if not rows:
        raise ValueError("no rows")
    with open(out_path, "w", newline="", encoding="ascii") as f:
        w = csv.DictWriter(f, fieldnames=FIELDS)
        w.writeheader()
        for r in rows:
            w.writerow(r)


def summarise(rows):
    from collections import Counter
    groups = Counter()
    classes = Counter()
    live = 0
    for r in rows:
        groups[r["group"] or "-"] += 1
        classes[r["content_guess"]] += 1
        live += r["filename"] != "" and not r["sentinel"]
    print(f"INF slots             : {len(rows)}")
    print(f"live named entries    : {live}")
    print(f"sentinels             : {sum(r['sentinel'] for r in rows)}")
    print(f"id bases              : ok ({len(ID_BASES)})")
    print(f"name table D_800955E0 : ok ({NAMED_SLOTS})")
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
    ap.add_argument("--summary", action="store_true", help="print slot / group / class counts")
    args = ap.parse_args()

    strings = load_pool(args.exe)
    inf = load_inf(args.inf)
    rows = build_mapping(strings, inf)
    check_bases(rows)
    check_name_table(args.exe, rows)

    if args.summary:
        summarise(rows)
        return

    if args.stdout:
        import io
        buf = io.StringIO()
        w = csv.DictWriter(buf, fieldnames=FIELDS, lineterminator="\n")
        w.writeheader()
        for r in rows:
            w.writerow(r)
        sys.stdout.write(buf.getvalue())
    else:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        write_csv(rows, args.out)
        print(f"wrote {args.out} ({len(rows)} rows)")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, FileNotFoundError) as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(2)
