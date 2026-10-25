#!/usr/bin/env python3
"""Decode the stage collision grid a Q124 capture read from RAM.

A capture whose spec "values" holds {"at": "0x800A33F0", "ptr_off": 0, ...}
reads the grid collision_SetGrid stored (43500.c:557) as raw bytes. This
restates every face of it as grid_tags.tsv in the capture directory, the
layout func_80053E9C / func_80053754 walk (43500.c):
  32x32 u16 byte offsets from the grid base, row-major (index cz*32 + cx;
  0xFFFF = empty cell); cell (cx, cz) spans world x cx*2000-32000 and z
  cz*2000-32000, 2000 units each;
  a cell: s16 face count, then per face 5 s16 (plane nx, ny, nz, then the
  u16 low / s16 high halves of d), 9 s16 (read by func_80052C4C), one u16
  (tag << 8 | n), then n + 1 two-s16 points.
The tag is what collision_GetHit stores to g_collision_hit_tag (43500.c:244)
when the face is hit. Rows are in grid order; nothing is inferred about what
a tag means.

  python tools/trace/scan/collision_grid.py build <capture dir>...
  python tools/trace/scan/collision_grid.py check <capture dir>...
"""

import argparse
import io
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3]))
from tools.trace import store

GRID = 0x800A33F0
HEADER = ["cx", "cz", "x0", "z0", "face", "tag", "points", "nx", "ny", "nz",
          "d"]


def grid_bytes(capture_dir):
    """The first successful pointer read of the grid base in the capture."""
    with store.open_capture_file(capture_dir, "capture.jsonl") as f:
        for line in f:
            e = json.loads(line)
            if (e.get("ev") == "value" and "hex" in e and "ptr" in e
                    and e.get("addr") == GRID):
                return e["ptr"], e.get("phase"), bytes.fromhex(e["hex"])
    raise SystemExit(f"collision_grid: {capture_dir}: no pointer read of "
                     f"0x{GRID:08X}")


def decode(buf):
    """Yields (cx, cz, face, tag, points, nx, ny, nz, d) in grid order;
    raises ValueError if a cell runs past the bytes read."""
    def need(off, size):
        if off + size > len(buf):
            raise ValueError(f"grid read too short: needs byte {off + size}, "
                             f"have {len(buf)}")
    need(0, 2048)
    for cz in range(32):
        for cx in range(32):
            off = struct.unpack_from("<H", buf, (cz * 32 + cx) * 2)[0]
            if off == 0xFFFF:
                continue
            need(off, 2)
            count = struct.unpack_from("<h", buf, off)[0]
            o = off + 2
            for face in range(count):
                need(o, 30)
                nx, ny, nz, dlo, dhi = struct.unpack_from("<hhhHh", buf, o)
                w = struct.unpack_from("<H", buf, o + 28)[0]
                n = w & 0xFF
                need(o + 30, (n + 1) * 4)
                o += 30 + (n + 1) * 4
                yield (cx, cz, face, w >> 8, n + 1, nx, ny, nz,
                       (dhi << 16) | dlo)


def render(capture_dir):
    ptr, phase, buf = grid_bytes(capture_dir)
    out = io.StringIO(newline="")
    out.write(f"# grid base 0x{ptr:08X} read at phase {phase}; "
              f"{len(buf)} bytes\n")
    out.write("\t".join(HEADER) + "\n")
    for cx, cz, face, tag, pts, nx, ny, nz, d in decode(buf):
        out.write("\t".join(str(v) for v in (
            cx, cz, cx * 2000 - 32000, cz * 2000 - 32000, face, tag, pts,
            nx, ny, nz, d)) + "\n")
    return out.getvalue()


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("cmd", choices=["build", "check"])
    ap.add_argument("dirs", nargs="+")
    args = ap.parse_args(argv)
    bad = 0
    for d in args.dirs:
        text = render(d)
        path = Path(d) / "grid_tags.tsv"
        if args.cmd == "build":
            with open(path, "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
        else:
            try:
                have = path.read_text(encoding="utf-8")
            except OSError:
                have = None
            if have != text:
                print(f"collision_grid: {path} is stale or missing")
                bad = 1
    if not bad and args.cmd == "check":
        print(f"collision_grid: {len(args.dirs)} ok")
    return bad


if __name__ == "__main__":
    sys.exit(main())
