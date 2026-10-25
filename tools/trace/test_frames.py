#!/usr/bin/env python3
"""Stdlib-only frame stamp tests (gdbtrace.frame_fields); no emulator."""
from pathlib import Path
import sys
import argparse
import json
import shutil
import uuid
from unittest.mock import patch
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


class TraceLoopTest(unittest.TestCase):
    def test_stop_stamps_and_duplicate_ordering(self):
        class FakeGdb:
            features = {}

            def __init__(self):
                self.armed = set()
                self.console = []
                self.late_stops = []
                self.stop = -1
                self.arming_regs_read = False

            def cmd(self, command):
                return "S05"

            def interrupt(self, *args):
                return "S05"

            def set_bp(self, kind, addr, n=4):
                self.armed.add((kind, addr))
                return True

            def clear_bp(self, kind, addr, n):
                self.armed.remove((kind, addr))

            def regs(self):
                if self.stop == -1:
                    self.arming_regs_read = True
                return [0] * 37 + [0x80010000]

            def mem(self, addr, n):
                return bytes(n)

            def cont(self):
                pass

            def close(self):
                pass

        gdb = FakeGdb()
        initial = (1, 0, "p", False)
        active = (2, 10, "q", False)
        pauses = [(8, 10, "100"), (8, 10, "100"),
                  (9, 11, "200"), (9, 11, "200"), (10, 12, "300")]

        def read_state(out):
            return initial if gdb.stop == -1 else active

        def read_pause(out):
            if gdb.stop == -1:
                # Seed only after both target and canary are armed and the
                # arming registers have been read, after setup round trips.
                self.assertEqual(gdb.armed, {(0, 0x80010000), (0, 0x80020000)})
                self.assertTrue(gdb.arming_regs_read)
                return (7, 9, "90")
            return pauses[gdb.stop]

        def wait_running(*args):
            gdb.stop += 1
            gdb.console.append("console output")
            return "S05" if gdb.stop < len(pauses) else "W00"

        syms = argparse.Namespace(path="fake.map", addr=lambda at: int(at, 0))
        spec = dict(script=dict(phases=[dict(frame=0, name="p"),
                                       dict(frame=10, name="q")]),
                    breakpoints=[dict(at="0x80010000")],
                    polls=[dict(at="0x80030000", every_s=0)])
        scratch = Path(__file__).resolve().parents[2] / "tmp"
        scratch.mkdir(exist_ok=True)
        directory = scratch / ("frames-test-" + uuid.uuid4().hex)
        directory.mkdir()
        self.addCleanup(shutil.rmtree, directory)
        out = directory / "capture.jsonl"
        args = argparse.Namespace(out=str(out), map="fake.map", result={},
                                  expect={}, duration=60, stall=1)
        with patch.object(gdbtrace, "exe_entry", return_value=0x80020000), \
                patch.object(gdbtrace, "wait_running", side_effect=wait_running), \
                patch.object(gdbtrace.runkit, "read_state", side_effect=read_state), \
                patch.object(gdbtrace.runkit, "read_pause", side_effect=read_pause), \
                patch.object(gdbtrace.runkit, "read_pause_frame", return_value=9):
            gdbtrace.trace(gdb, syms, spec, args)
        records = [json.loads(line) for line in out.read_text().splitlines()]
        hits = [r for r in records if r["ev"] == "hit"]
        self.assertEqual(len(hits), 4)  # the duplicate produced no hit
        self.assertEqual([(r["frame"], r["pause_n"]) for r in hits[:2]],
                         [(10, 8), (11, 9)])
        self.assertIn("did not move", hits[2]["frame_note"])
        self.assertIn("lagged", hits[3]["frame_note"])
        for hit in hits[2:]:
            self.assertNotIn("frame", hit)
        for kind in ("poll", "tty"):
            other = [r for r in records if r["ev"] == kind]
            self.assertGreater(len(other), 1)
            for record in other:
                self.assertTrue({"frame", "pause_n", "frame_note"}.isdisjoint(record))


if __name__ == "__main__":
    unittest.main()
