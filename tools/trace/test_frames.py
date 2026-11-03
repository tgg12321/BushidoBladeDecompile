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

    def test_capped_watch_keeps_same_address_watch(self):
        # PCSX-Redux's z finds breakpoints by address alone: the fake stub's
        # clear removes every kind set on an overlapping range.
        addr = 0x800A38A8

        class FakeGdb:
            features = {}

            def __init__(self):
                self.set = []
                self.console = []
                self.late_stops = []
                self.cur = None

            def cmd(self, command):
                return "S05"

            def interrupt(self, *args):
                return "S05"

            def set_bp(self, kind, a, n=4):
                self.set.append((kind, a, n))
                return True

            def clear_bp(self, kind, a, n=4):
                self.set = [s for s in self.set
                            if not (s[1] <= a < s[1] + s[2])]
                return True

            def regs(self):
                return [0] * 37 + [0x80040000]

            def mem(self, a, n):
                return bytes(n)

            def cont(self):
                pass

            def close(self):
                pass

        gdb = FakeGdb()
        accesses = ["w", "r"] * 6  # the game's alternating store / load
        pos = [0]

        def wait_running(*args):
            kinds = {s[0] for s in gdb.set if s[1] == addr}
            while pos[0] < len(accesses):
                a = accesses[pos[0]]
                pos[0] += 1
                if (2 if a == "w" else 3) in kinds:
                    gdb.cur = (0x80040000, addr, 4, a == "w", 0)
                    return "T05"
            return "W00"

        syms = argparse.Namespace(path="fake.map", addr=lambda at: int(at, 0))
        spec = dict(watch=[dict(at=hex(addr), kind="write", max_hits=2),
                           dict(at=hex(addr), kind="read")])
        scratch = Path(__file__).resolve().parents[2] / "tmp"
        scratch.mkdir(exist_ok=True)
        directory = scratch / ("frames-test-" + uuid.uuid4().hex)
        directory.mkdir()
        self.addCleanup(shutil.rmtree, directory)
        out = directory / "capture.jsonl"
        args = argparse.Namespace(out=str(out), map="fake.map", result={},
                                  expect={}, duration=60, stall=1)
        with patch.object(gdbtrace, "wait_running", side_effect=wait_running), \
                patch.object(gdbtrace, "decode_access",
                             side_effect=lambda *a: [gdb.cur]):
            gdbtrace.trace(gdb, syms, spec, args)
        records = [json.loads(line) for line in out.read_text().splitlines()]
        hits = [r["kind"] for r in records if r["ev"] == "hit"]
        self.assertEqual(hits.count(2), 2)
        self.assertEqual(hits.count(3), 6)  # every load, after the cap too
        self.assertEqual([r["after_hits"] for r in records
                          if r["ev"] == "disarmed"], [2])
        self.assertFalse(any(r["ev"] == "unmatched-stop" for r in records))
        self.assertEqual(gdb.set, [])  # the end clears both

    KICK_PC = 0x8007CB3C  # after _otc's `sw v1, 0(v0)` to DMA6 CHCR

    def dma_gdb(self, chcr=0x11000002, chcr_addr=0x1F8010E8):
        kick = self.KICK_PC - 4

        class FakeGdb:
            features = {}

            def __init__(self):
                self.set = []
                self.console = []
                self.late_stops = []

            def cmd(self, command):
                return "S05"

            def interrupt(self, *args):
                return "S05"

            def set_bp(self, kind, a, n=4):
                self.set.append((kind, a, n))
                return True

            def clear_bp(self, kind, a, n=4):
                self.set = [s for s in self.set
                            if not (s[1] <= a < s[1] + s[2])]
                return True

            def regs(self):
                r = [0] * 38
                r[2], r[3], r[37] = chcr_addr, chcr, kick + 4
                return r

            def mem(self, a, n):
                if a == kick:  # sw v1, 0(v0)
                    return (0xAC430000).to_bytes(4, "little")
                return bytes(n)

            def cont(self):
                pass

            def close(self):
                pass
        return FakeGdb()

    def run_dma(self, gdb, watch, stops, fires):
        # The stub stops only while a watch of a kind in `fires` (the DMA's
        # direction, or access) is set.
        left = [stops]

        def wait_running(*args):
            if left[0] and any(s[0] in fires for s in gdb.set):
                left[0] -= 1
                return "T05"
            return "W00"

        syms = argparse.Namespace(path="fake.map", addr=lambda at: int(at, 0))
        scratch = Path(__file__).resolve().parents[2] / "tmp"
        scratch.mkdir(exist_ok=True)
        directory = scratch / ("frames-test-" + uuid.uuid4().hex)
        directory.mkdir()
        self.addCleanup(shutil.rmtree, directory)
        out = directory / "capture.jsonl"
        args = argparse.Namespace(out=str(out), map="fake.map", result={},
                                  expect={}, duration=60, stall=1)
        with patch.object(gdbtrace, "wait_running", side_effect=wait_running):
            gdbtrace.trace(gdb, syms, dict(watch=watch), args)
        return ([json.loads(line) for line in out.read_text().splitlines()],
                args.result)

    def test_dma_write_credits_the_write_watch(self):
        # ClearOTagR's OTC DMA writes a watched OT word: PCSX-Redux stops
        # after the CHCR store, with no load or store of the word at pc.
        ot = 0x800F94A8
        records, _ = self.run_dma(self.dma_gdb(), [
            dict(at=hex(ot), kind="write", max_hits=2),
            dict(at=hex(ot), kind="read")], stops=5, fires={2, 4})
        hits = [r for r in records if r["ev"] == "hit"]
        self.assertEqual([r["kind"] for r in hits], [2, 2])
        self.assertEqual(hits[0]["dma"]["ch"], 6)
        self.assertEqual(hits[0]["dma"]["kick_pc"], self.KICK_PC - 4)
        self.assertNotIn("access", hits[0])
        self.assertEqual([r["after_hits"] for r in records
                          if r["ev"] == "disarmed"], [2])
        self.assertFalse(any(r["ev"] in ("unmatched-stop", "dma-stop")
                             for r in records))

    def test_dma_from_ram_credits_the_read_watch(self):
        # GPU block DMA (ch2, CHCR 0x01000201: start, from RAM).
        buf = 0x80118800
        records, _ = self.run_dma(
            self.dma_gdb(chcr=0x01000201, chcr_addr=0x1F8010A8), [
                dict(at=hex(buf), kind="write"),
                dict(at=hex(buf), kind="read", max_hits=1)], stops=3,
            fires={3, 4})
        hits = [r for r in records if r["ev"] == "hit"]
        self.assertEqual([(r["kind"], r["dma"]["ch"]) for r in hits], [(3, 2)])

    def test_dma_stop_on_two_fitting_watches_credits_none(self):
        records, result = self.run_dma(self.dma_gdb(), [
            dict(at="0x800F94A8", kind="write"),
            dict(at="0x800F9000", kind="access")], stops=2, fires={2, 4})
        self.assertFalse(any(r["ev"] == "hit" for r in records))
        dma = [r for r in records if r["ev"] == "dma-stop"]
        self.assertEqual(len(dma), 2)
        self.assertEqual(sorted(dma[0]["dma"]["candidates"]),
                         ["Z2:0x800F94A8", "Z4:0x800F9000"])
        self.assertEqual(result["dma_unattributed"], 2)

    def test_dma_kick_needs_the_start_bit_and_a_chcr(self):
        gdb = self.dma_gdb(chcr=0x00000002)  # start bit clear
        self.assertIsNone(gdbtrace.dma_kick(gdb, gdb.regs(), self.KICK_PC))
        gdb = self.dma_gdb(chcr_addr=0x1F8010E0)  # ch6 MADR, not CHCR
        self.assertIsNone(gdbtrace.dma_kick(gdb, gdb.regs(), self.KICK_PC))
        gdb = self.dma_gdb()
        self.assertEqual(gdbtrace.dma_kick(gdb, gdb.regs(), self.KICK_PC),
                         {"ch": 6, "chcr": 0x11000002,
                          "kick_pc": self.KICK_PC - 4, "from_ram": False})


if __name__ == "__main__":
    unittest.main()
