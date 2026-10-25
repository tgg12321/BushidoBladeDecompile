#!/usr/bin/env python3
"""Tests for story_streams.py: a synthetic EXE with two streams."""

from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import story_streams  # noqa: E402

S0 = 0x8008F220
S1 = S0 + 0x20
TABLE = S1 + 0x10


def exe():
    body = bytearray(TABLE + 32 * 4 - story_streams.LOAD)
    s0 = bytes([1, 8, 16, 7, 16, 3, 0x33, 0x24, 18, 17, 0x1C, 0, 0xC0,
                19, 0x15, 0x15, 4, 3, 18, 3, 1, 0xFF, 0x42, 0])
    s1 = bytes([20]) + bytes(12) + bytes([0])
    for addr, s in ((S0, s0), (S1, s1)):
        off = addr - story_streams.LOAD
        body[off:off + len(s)] = s
    ptrs = [S0] * 32
    ptrs[3] = S1
    body[TABLE - story_streams.LOAD:] = struct.pack("<32I", *ptrs)
    return bytes(story_streams.HEADER_SIZE) + bytes(body)


class Decode(unittest.TestCase):
    def test_commands(self):
        b = exe()
        got = list(story_streams.decode(
            story_streams.read(b, S0, 0x20), S0))
        self.assertEqual([op for _, op, _ in got],
                         [1, 16, 18, 17, 19, 18, 3, 0x42, 0])
        self.assertEqual(got[3], (S0 + 9, 17, bytes([0x1C, 0, 0xC0])))
        self.assertEqual(got[7], (S0 + 22, 0x42, b""))

    def test_overrun(self):
        with self.assertRaises(ValueError):
            list(story_streams.decode(bytes([17, 1]), S0))

    def test_no_end(self):
        with self.assertRaises(ValueError):
            list(story_streams.decode(bytes([18, 18]), S0))

    def test_rows(self):
        b = exe()
        ends = {S0: S1, S1: TABLE}
        rows = list(story_streams.rows(b, TABLE, ends.__getitem__))
        opp = [(r[0], r[3], r[5]) for r in rows if r[5]]
        self.assertIn(("0", "16", "0x0E|0x1D"), opp)
        self.assertIn(("0", "17", "0x1C"), opp)
        self.assertIn(("0", "3", "0x01"), opp)
        self.assertIn(("3", "20", "0x1F"), opp)
        self.assertEqual(sum(1 for r in rows if r[0] == "3"), 2)

    def test_main(self):
        with tempfile.TemporaryDirectory() as d:
            e = Path(d) / "exe"
            e.write_bytes(exe())
            m = Path(d) / "map"
            m.write_text(f"                0x{TABLE:08x}"
                         f"                        D_800900EC = "
                         f"0x{TABLE:08x}\n", encoding="utf-8")
            import io
            import contextlib
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                story_streams.main(["--exe", str(e), "--map", str(m),
                                    "--stream", "3"])
            lines = out.getvalue().splitlines()
            self.assertEqual(lines[0].split("\t"), story_streams.COLUMNS)
            self.assertEqual(len(lines), 3)


if __name__ == "__main__":
    unittest.main()
