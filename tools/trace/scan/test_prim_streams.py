#!/usr/bin/env python3
"""Checks for prim_streams.py on hand-built streams (no disc needed).

  python tools/trace/scan/test_prim_streams.py
"""

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import prim_streams as ps  # noqa: E402

SIZES = [4] * ps.N_TYPES


def hw(*vals):
    return struct.pack("<%dH" % len(vals), *vals)


class WalkTest(unittest.TestCase):
    def test_mode1_textured_group(self):
        # count 1, mode 1, 3 halfwords of header data, then (pos & 3) pad,
        # group count 2 type 0x4 (FT3: 12 halfwords each), end, no more
        body = hw(1, 1, 0, 0, 0) + hw(0) + hw(2, 0x4) + hw(*[0] * 24) + hw(0, 0)
        segs, groups = ps.walk(body, 0, SIZES)
        self.assertEqual(segs, [1])
        self.assertEqual(groups, [(1, 0x4, 2)])

    def test_mode0_textured_group_uses_size_table(self):
        sizes = list(SIZES)
        sizes[0xC] = 7
        # count 0 header items, mode 0; group 1 x type 0xC (kind 1), 7 hw
        body = hw(0, 0) + hw(1, 0xC) + hw(*[0] * 7) + hw(0, 0)
        segs, groups = ps.walk(body, 0, sizes)
        self.assertEqual(segs, [0])
        self.assertEqual(groups, [(0, 0xC, 1)])

    def test_untextured_group_skipped_by_size(self):
        body = hw(0, 2) + hw(3, 0x1) + hw(*[0] * 12) + hw(0, 0)
        segs, groups = ps.walk(body, 0, SIZES)
        self.assertEqual((segs, groups), ([2], []))

    def test_second_segment(self):
        seg = hw(0, 1) + hw(0)
        body = seg + hw(1) + seg + hw(0)
        segs, _ = ps.walk(body, 0, SIZES)
        self.assertEqual(segs, [1, 1])

    def test_packed_segment_reads_twelve_more(self):
        # bit 15: packed mode 2; (count & 0x7FFF) + 1 halfwords skipped, then
        # aligned to 4; after the groups, 3-halfword records up to 0xFFFF;
        # the 13th packed segment returns without reading further
        body = b""
        for i in range(13):
            seg = hw(0x8000, 0)
            if (len(body) + 4) & 3:
                seg += hw(0)
            seg += hw(0) + hw(7, 7, 7) + hw(0xFFFF)
            body += seg + (hw(1) if i < 12 else b"")
        segs, _ = ps.walk(body, 0, SIZES)
        self.assertEqual(segs, [2] * 13)

    def test_faults(self):
        with self.assertRaises(ps.Fault):
            ps.walk(hw(0, 4) + hw(0, 0), 0, SIZES)
        with self.assertRaises(ps.Fault):
            ps.walk(hw(0, 1) + hw(1, 0x30) + hw(0, 0), 0, SIZES)
        with self.assertRaises(ps.Fault):
            ps.walk(hw(0, 1), 0, SIZES)


class BlockTest(unittest.TestCase):
    def build(self, streams):
        c = len(streams)
        table = struct.pack("<I", c)
        off = 4 + 4 * c
        offs = []
        for s in streams:
            offs.append(off)
            off += len(s)
        sec1 = table + struct.pack("<%dI" % c, *offs) + b"".join(streams)
        head = struct.pack("<III", 2, 12, 12 + len(sec1))
        return head + sec1 + b"\0" * 4

    def test_scan_block_counts(self):
        s = hw(0, 2) + hw(1, 0x26) + hw(*[0] * 10) + hw(0, 0)
        row = ps.scan_block(self.build([s, s]), SIZES)
        self.assertEqual(row["entries"], 2)
        self.assertEqual(row["fault"], "")
        self.assertEqual((row["seg2"], row["tex2"], row["tex0"]), (2, 2, 0))

    def test_scan_block_records_first_fault(self):
        bad = hw(0, 9) + hw(0, 0)
        good = hw(0, 1) + hw(0) + hw(0, 0)
        row = ps.scan_block(self.build([good, bad]), SIZES)
        self.assertEqual(row["fault"], "entry 1: segment mode 9")
        self.assertEqual(row["seg1"], 1)

    def test_not_a_table(self):
        self.assertIsNone(ps.scan_block(b"\x01\0\0\0" + b"\0" * 12, SIZES))

    def test_callers(self):
        self.assertEqual(ps.caller_of(0, None), "slot")
        self.assertEqual(ps.caller_of(36, None), "slot")
        self.assertEqual(ps.caller_of(0x25, None), "-")
        self.assertEqual(ps.caller_of(0x26, 4), "slot")
        self.assertEqual(ps.caller_of(0x27, None), "-")
        self.assertEqual(ps.caller_of(0x83, None), "block6")


if __name__ == "__main__":
    unittest.main()
