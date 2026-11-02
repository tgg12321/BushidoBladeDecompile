#!/usr/bin/env python3
"""Checks for draw_streams.py on hand-built streams and code (no disc needed).

  python tools/trace/scan/test_draw_streams.py
"""

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import draw_streams as ds  # noqa: E402

FAIL = 0x80044650
H24, H36, H4 = 0x80001000, 0x80002000, 0x80003000
TABS = {
    "default": [H24] * 88,
    "packed": [H24] * 32,
    "lit": [H24] * 32,
}
TABS["default"][0x20] = H4
TABS["default"][44 + 0x20] = FAIL
TABS["default"][42] = ds.COLORSCALED[0]
TABS["packed"][0x14] = ds.PACKED14_HANDLER
STRIDES = {H24: 24, H36: 36, H4: 4, FAIL: None, ds.COLORSCALED[0]: 24,
           ds.PACKED14_HANDLER: 36}


def hw(*vals):
    return struct.pack("<%dH" % len(vals), *vals)


def walk(body, hi=False):
    return ds.walk(body, 0, TABS, STRIDES, hi)


class WalkTest(unittest.TestCase):
    def test_mode1_packed_vertices_round_up_to_a_word(self):
        # 1 vertex (6 bytes) -> pos 10, rounded to 12; group 2 x type 0x20
        # (4 bytes each), end, no more
        body = hw(1, 1) + hw(0, 0, 0) + hw(0) + hw(2, 0x20) + hw(0) * 4 + hw(0, 0)
        segs, groups = walk(body)
        self.assertEqual(segs, [1])
        self.assertEqual(groups, [(1, 0x20, H4, 2)])

    def test_mode1_high_half_reaches_a_fault_handler(self):
        body = hw(1, 1) + hw(0, 0, 0) + hw(0) + hw(2, 0x20) + hw(0) * 4 + hw(0, 0)
        with self.assertRaises(ds.Fault):
            walk(body, hi=True)

    def test_mode0_and_mode2_take_24_bytes_per_vertex(self):
        for mode in (0, 2):
            body = hw(2, mode) + b"\0" * 48 + hw(1, 0x4) + b"\0" * 24 + hw(0, 0)
            segs, groups = walk(body)
            self.assertEqual(segs, [mode])
            self.assertEqual(len(body), 4 + 48 + 4 + 24 + 4)

    def test_mode3_has_no_vertex_data(self):
        # mode 3: the group list follows the header word at once
        body = hw(5, 3) + hw(1, 42) + b"\0" * 24 + hw(0, 0)
        segs, groups = walk(body)
        self.assertEqual(segs, [3])
        c = ds.targets(segs, groups)
        self.assertEqual((c["mode3"], c["colorscaled"]), (1, 1))

    def test_mode4_type_0x14_is_a_target(self):
        body = hw(2, 4) + b"\0" * 12 + hw(1, 0x14) + b"\0" * 36 + hw(0, 0)
        segs, groups = walk(body)
        self.assertEqual(ds.targets(segs, groups)["packed14"], 1)

    def test_more_halfword_chains_segments(self):
        # 4 + 12 + 4 + 24 + 2 bytes, then the more halfword: the next
        # header lands on the word boundary at 48
        seg = hw(2, 1) + b"\0" * 12 + hw(1, 0x4) + b"\0" * 24 + hw(0)
        segs, _ = walk(seg + hw(1) + seg + hw(0))
        self.assertEqual(segs, [1, 1])

    def test_zero_count_header_ends_the_stream(self):
        self.assertEqual(walk(hw(0, 9)), ([], []))

    def test_mode_5_and_up_faults(self):
        with self.assertRaises(ds.Fault):
            walk(hw(1, 5) + b"\0" * 32)

    def test_bit15_count_is_named(self):
        with self.assertRaisesRegex(ds.Fault, "bit 15"):
            walk(hw(0x8001, 0) + b"\0" * 32)


class StrideTest(unittest.TestCase):
    def exe(self, words):
        return b"\0" * ds.EXE_HEADER + struct.pack("<%dI" % len(words), *words)

    def test_one_addiu_s0(self):
        exe = self.exe([0, 0x26100024, 0, ds.JR_RA, 0])
        self.assertEqual(ds.handler_stride(exe, ds.EXE_LOAD), 0x24)

    def test_none_without_addiu(self):
        exe = self.exe([0, ds.JR_RA, 0])
        self.assertIsNone(ds.handler_stride(exe, ds.EXE_LOAD))

    def test_two_sizes_are_an_error(self):
        exe = self.exe([0x26100018, 0x26100024, ds.JR_RA])
        with self.assertRaises(ValueError):
            ds.handler_stride(exe, ds.EXE_LOAD)


class TableTest(unittest.TestCase):
    def test_reloc_table_found_from_a_header_word(self):
        # header: 2 sections, offsets 0x10 / 0x30; table at 0x10: count 1,
        # entry at table+8 holding an empty stream
        buf = bytearray(0x40)
        struct.pack_into("<4I", buf, 0, 2, 0x10, 0x30, 0x40)
        struct.pack_into("<2I", buf, 0x10, 1, 8)
        t = ds.reloc_tables(bytes(buf))
        self.assertEqual(t, [(1, [0x18])])


if __name__ == "__main__":
    unittest.main()
