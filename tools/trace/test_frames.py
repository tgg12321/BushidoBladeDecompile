#!/usr/bin/env python3
"""Stdlib-only frame stamp tests (gdbtrace.frame_fields); no emulator."""
from pathlib import Path
import sys
import unittest

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import gdbtrace


class FramesTest(unittest.TestCase):
    def test_next_pause_gives_its_frame(self):
        self.assertEqual(gdbtrace.frame_fields((8, 1234, "99"), 7, False),
                         ({"frame": 1234, "pause_n": 8}, False))

    def test_no_pause_line(self):
        for pause, last in ((None, 7), ((8, 1234, None), None)):
            fields, lagged = gdbtrace.frame_fields(pause, last, False)
            self.assertNotIn("frame", fields)
            self.assertIn("no Lua pause", fields["frame_note"])
            self.assertFalse(lagged)

    def test_unmoved_count_disables_later_stamps(self):
        fields, lagged = gdbtrace.frame_fields((7, 1200, None), 7, False)
        self.assertNotIn("frame", fields)
        self.assertTrue(lagged)
        # A later, apparently fresh line may belong to the earlier stop.
        fields, lagged = gdbtrace.frame_fields((8, 1234, None), 7, lagged)
        self.assertNotIn("frame", fields)
        self.assertIn("lagged", fields["frame_note"])
        self.assertTrue(lagged)

    def test_count_jump_gives_no_frame(self):
        fields, lagged = gdbtrace.frame_fields((10, 1234, None), 7, False)
        self.assertEqual(fields, {"frame_note": "Lua pause count moved 7 -> 10"})
        self.assertFalse(lagged)


if __name__ == "__main__":
    unittest.main()
