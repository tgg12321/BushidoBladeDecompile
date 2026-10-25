#!/usr/bin/env python3
"""Tests for collision_grid.py: a synthetic two-cell grid."""

import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import collision_grid  # noqa: E402


def face(nx, ny, nz, d, tag, pts):
    b = struct.pack("<hhhHh", nx, ny, nz, d & 0xFFFF, d >> 16)
    b += bytes(18) + struct.pack("<H", (tag << 8) | (len(pts) - 1))
    return b + b"".join(struct.pack("<hh", x, z) for x, z in pts)


def grid():
    table = [0xFFFF] * 1024
    cells = b""
    table[0] = 2048 + len(cells)
    cells += struct.pack("<h", 1) + face(0, -4096, 0, -70000, 3,
                                         [(0, 0), (1, 0), (1, 1)])
    table[5 * 32 + 17] = 2048 + len(cells)
    cells += struct.pack("<h", 2) + face(0, -4096, 0, 5, 6, [(0, 0)] * 4) \
        + face(4096, 0, 0, -1, 7, [(2, 2), (3, 3)])
    return struct.pack("<1024H", *table) + cells


class CollisionGridTest(unittest.TestCase):
    def test_decode(self):
        rows = list(collision_grid.decode(grid()))
        self.assertEqual(rows, [
            (0, 0, 0, 3, 3, 0, -4096, 0, -70000),
            (17, 5, 0, 6, 4, 0, -4096, 0, 5),
            (17, 5, 1, 7, 2, 4096, 0, 0, -1)])

    def test_short_read(self):
        with self.assertRaises(ValueError):
            list(collision_grid.decode(grid()[:-1]))

    def test_build_check(self):
        with tempfile.TemporaryDirectory() as d:
            rec = {"ev": "value", "addr": collision_grid.GRID, "ptr": 0x800C0000,
                   "phase": "fight", "hex": grid().hex()}
            Path(d, "capture.jsonl").write_text(json.dumps(rec) + "\n",
                                                encoding="utf-8")
            self.assertEqual(collision_grid.main(["check", d]), 1)
            self.assertEqual(collision_grid.main(["build", d]), 0)
            self.assertEqual(collision_grid.main(["check", d]), 0)
            lines = Path(d, "grid_tags.tsv").read_text().splitlines()
            self.assertEqual(lines[2].split("\t"),
                             ["0", "0", "-32000", "-32000", "0", "3", "3", "0",
                              "-4096", "0", "-70000"])
            self.assertEqual(len(lines), 5)


if __name__ == "__main__":
    unittest.main()
