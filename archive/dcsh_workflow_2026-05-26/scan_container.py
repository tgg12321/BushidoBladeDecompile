#!/usr/bin/env python3
"""
scan_container.py - Scan any binary blob for known BB2 / PSX format magics.

Locates valid TIM headers (with strict header sanity-check), VAB (`pBAV`)
headers, BBM (`MC`/`MW`) magic, and outer-TOC patterns used by BB2's
container BIN files. Useful for poking at undocumented .BIN files in
`disc/TIM2D/` and similar containers.

Usage:
    scan_container.py FILE                    summary of every magic found
    scan_container.py FILE --json             machine-readable findings
    scan_container.py DIR/                    summarise every BIN/DAT in DIR
"""

import argparse
import json
import os
import struct
import sys
from pathlib import Path


def validate_tim(blob, off):
    """Return (info_dict, total_bytes) if a valid TIM starts at `off`, else None."""
    if off + 12 > len(blob):
        return None
    if blob[off : off + 4] != b"\x10\x00\x00\x00":
        return None
    flag = struct.unpack_from("<I", blob, off + 4)[0]
    # Reject anything that uses bits other than the known 4 (pmode + CLUT)
    if (flag & ~0xB) != 0:
        return None
    pmode = flag & 3
    has_clut = bool((flag >> 3) & 1)
    pos = off + 8
    clut_sz = 0
    if has_clut:
        clut_sz = struct.unpack_from("<I", blob, pos)[0]
        if clut_sz < 12 or clut_sz > 0x10000:
            return None
        pos += clut_sz
    if pos + 12 > len(blob):
        return None
    pix_sz = struct.unpack_from("<I", blob, pos)[0]
    if pix_sz < 12 or pix_sz > len(blob) - pos:
        return None
    dx, dy, w, h = struct.unpack_from("<HHHH", blob, pos + 4)
    if w == 0 or h == 0 or w > 2048 or h > 2048:
        return None
    pmode_label = ["4bpp", "8bpp", "16bpp", "24bpp"][pmode]
    real_w = w * (4 if pmode == 0 else 2 if pmode == 1 else 1)
    total = pos + pix_sz - off
    info = {
        "off": off,
        "kind": "tim",
        "pmode": pmode_label,
        "has_clut": has_clut,
        "clut_size": clut_sz,
        "pixel_size": pix_sz,
        "image": f"{real_w}x{h}",
        "vram_dxdy": (dx, dy),
        "total_bytes": total,
    }
    return info


def find_vabs(blob):
    """Find all `pBAV` (VAB-VH) headers. Returns list of dicts."""
    out = []
    i = 0
    while True:
        j = blob.find(b"pBAV", i)
        if j < 0:
            break
        if j + 32 <= len(blob):
            (ver, vid, fsize, _r, pcount, tcount, vcount) = struct.unpack_from(
                "<IIIHHHH", blob, j + 4
            )
            out.append(
                {
                    "off": j,
                    "kind": "vab",
                    "version": ver,
                    "vab_id": vid,
                    "fsize": fsize,
                    "programs": pcount,
                    "tones": tcount,
                    "vags": vcount,
                }
            )
        i = j + 4
    return out


def find_bbms(blob):
    """Find all BBM magic markers (`MC` for COMMON, `MW` for character)."""
    out = []
    # Only at offsets where a 20-byte BBM header could plausibly start
    for i in range(0, len(blob) - 20, 4):
        if blob[i : i + 2] in (b"MC", b"MW"):
            # 4 u32s after the 2-byte magic should be increasing reasonable offsets
            try:
                vals = struct.unpack_from("<IIII", blob, i + 4)
            except struct.error:
                continue
            if all(0 < v < len(blob) for v in vals) and list(vals) == sorted(vals):
                out.append(
                    {
                        "off": i,
                        "kind": "bbm",
                        "magic": blob[i : i + 2].decode("ascii"),
                        "section_ends": list(vals),
                    }
                )
    return out


def scan(blob):
    findings = []
    for i in range(0, len(blob) - 12, 4):
        info = validate_tim(blob, i)
        if info:
            findings.append(info)
    findings.extend(find_vabs(blob))
    findings.extend(find_bbms(blob))
    findings.sort(key=lambda x: x["off"])
    return findings


def summary(path, findings, outer_toc=None):
    print(f"{path}  size={os.path.getsize(path)} (0x{os.path.getsize(path):x})")
    if outer_toc is not None:
        print(
            f"  outer 5-u32 TOC : "
            + ", ".join(f"0x{v:x}" for v in outer_toc)
        )
    if not findings:
        print("  no recognised magics found")
        return
    print(f"  found {len(findings)} embedded resource(s):")
    for f in findings:
        if f["kind"] == "tim":
            clut = "+CLUT" if f["has_clut"] else ""
            print(
                f"    @0x{f['off']:06x}  TIM {f['pmode']}{clut} "
                f"{f['image']} VRAM=({f['vram_dxdy'][0]},{f['vram_dxdy'][1]}) "
                f"size=0x{f['total_bytes']:x}"
            )
        elif f["kind"] == "vab":
            print(
                f"    @0x{f['off']:06x}  VAB-VH v{f['version']} id={f['vab_id']} "
                f"fsize=0x{f['fsize']:x} progs={f['programs']} "
                f"tones={f['tones']} vags={f['vags']}"
            )
        elif f["kind"] == "bbm":
            ends = ", ".join(f"0x{x:x}" for x in f["section_ends"])
            print(f"    @0x{f['off']:06x}  BBM {f['magic']} section ends=[{ends}]")


def get_outer_toc(blob):
    if len(blob) >= 20:
        return list(struct.unpack_from("<5I", blob, 0))
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("target", help="file or directory")
    ap.add_argument("--json", action="store_true", help="machine-readable JSON")
    args = ap.parse_args()

    target = Path(args.target)
    if target.is_dir():
        for p in sorted(target.iterdir()):
            if p.is_file() and p.suffix.upper() in (".BIN", ".DAT", ".SE"):
                with open(p, "rb") as f:
                    blob = f.read()
                findings = scan(blob)
                if args.json:
                    print(json.dumps({"path": str(p), "findings": findings}))
                else:
                    summary(str(p), findings, get_outer_toc(blob))
                    print()
        return

    with open(target, "rb") as f:
        blob = f.read()
    findings = scan(blob)
    if args.json:
        print(json.dumps({"path": str(target), "findings": findings}, indent=2))
    else:
        summary(str(target), findings, get_outer_toc(blob))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, FileNotFoundError) as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(2)
