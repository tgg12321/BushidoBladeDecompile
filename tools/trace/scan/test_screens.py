#!/usr/bin/env python3
"""Synthetic capture directories; no emulator, build, or index writes."""
import contextlib
import csv
import io
import json
from pathlib import Path
import sys
import shutil
import unittest
import uuid
from unittest.mock import patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[3]))
from tools.trace.scan import screens


class ScreensTest(unittest.TestCase):
    def setUp(self):
        scratch = screens.REPO_ROOT / "tmp"
        scratch.mkdir(exist_ok=True)
        self.root = scratch / ("screens-test-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(shutil.rmtree, self.root)
        self.runtime = self.root / "docs/naming/runtime"
        self.capture = self.runtime / "sample"
        self.capture.mkdir(parents=True)
        self.spec = dict(script=dict(phases=[dict(frame=0, name="p"),
                                            dict(frame=20, name="q")],
                                    screenshots=[dict(frame=10, name="a"),
                                                 dict(frame=15, name="b"),
                                                 dict(frame=30, name="c")]),
                         hitmap=dict(per_phase=1))
        self.hits = [self.hit(5), self.hit(11)]
        self.end = True  # synthesize a matching end record
        self.manifest = dict(q124_complete=True, outputs=[
            dict(path=r"C:\old\capture.a.png", sha1="abcdef0123456789")])
        self.addCleanup(patch.stopall)
        patch.object(screens, "REPO_ROOT", self.root).start()
        patch.object(screens.events, "index_inputs", self.blobs).start()

    def hit(self, frame, phase="p"):
        event = dict(ev="hit", kind=0, addr=0x80010000, at="recorded_function",
                     phase=phase, hm=1)
        if frame is not None:
            event["frame"] = frame
        return event

    def blobs(self, runtime):
        self.assertEqual(runtime, self.runtime)
        records = [dict(ev="start", spec=self.spec), *self.hits]
        if self.end:
            totals = {}
            for e in self.hits:
                key = f"Z{e['kind']}:{e['at']}"
                totals[key] = totals.get(key, 0) + 1
            records.append(dict(ev="end", script_ended=self.end is True,
                                events=len(self.hits), hits=totals))
        (self.capture / "capture.manifest.json").write_bytes(json.dumps(self.manifest).encode())
        (self.capture / "capture.jsonl").write_bytes(
            "".join(json.dumps(e) + "\n" for e in records).encode())
        return {p: p.read_bytes() for p in self.runtime.rglob("*") if p.is_file()}

    def rows(self):
        return list(csv.DictReader(io.StringIO(screens.render()), delimiter="\t"))

    def test_next_screenshot(self):
        row, = self.rows()
        self.assertEqual(row["screens"], "a@10#abcdef012345;b@15")
        self.assertEqual((row["first_frame"], row["last_frame"], row["hits_with_frame"]),
                         ("5", "11", "2"))
        self.assertIn("first 1 hits per phase", row["coverage"])
        self.assertEqual(row["inconclusive"], "0")

    def test_same_frame_shot_precedes_hit(self):
        # A shot on frame F is taken as F starts, before code running in F.
        self.hits = [self.hit(10)]
        row, = self.rows()
        self.assertEqual(row["screens"], "b@15")

    def test_shot_before_first_phase(self):
        self.spec["script"]["phases"] = [dict(frame=5, name="p")]
        self.spec["script"]["screenshots"] = [dict(frame=3, name="early"),
                                              dict(frame=9, name="late")]
        self.hits = [self.hit(1, "start"), self.hit(6)]
        start, p = sorted(self.rows(), key=lambda r: r["phase"] != "start")
        self.assertEqual(start["screens"], "early@3")
        self.assertEqual(p["screens"], "late@9")

    def test_run_limits(self):
        self.end = None
        row, = self.rows()
        self.assertEqual(row["inconclusive"], "1")
        self.assertIn("missing_end_record", row["reason"])
        self.end = "unfinished"
        row, = self.rows()
        self.assertIn("script_not_ended", row["reason"])

    def test_other_phase_unused(self):
        self.hits = [self.hit(16)]
        row, = self.rows()
        self.assertEqual(row["screens"], "")
        self.assertIn("no_later_screenshot", row["reason"])
        self.assertEqual(row["inconclusive"], "1")

    def test_boundary_shot_precedes_phase_change(self):
        self.spec["script"]["screenshots"] = [dict(frame=20, name="edge")]
        self.hits = [self.hit(19), self.hit(20, "q")]
        p, q = self.rows()
        self.assertEqual(p["screens"], "edge@20")
        self.assertEqual(q["screens"], "")

    def test_frameless_hits(self):
        self.hits.append(self.hit(None))
        row, = self.rows()
        self.assertEqual(row["inconclusive"], "1")
        self.assertIn("hits_without_frame", row["reason"])

    def test_frameless_capture(self):
        self.hits = [self.hit(None)]
        self.assertEqual(self.rows(), [])

    def test_capture_time_watch_resolution(self):
        matrix = self.runtime / "matrix"
        matrix.mkdir()
        (matrix / "coverage.tsv").write_bytes(
            b"addr\tcapture_name\tsize\n0x80020000\tobserved_caller\t0x40\n")
        self.hits = [dict(ev="hit", kind=2, addr=0x800A0000, at="D_800A0000",
                          phase="p", frame=5, access=dict(pc=0x80020010))]
        row, = self.rows()
        self.assertEqual((row["addr"], row["name"]),
                         ("0x80020000", "observed_caller"))
        self.assertEqual(row["inconclusive"], "0")

    def test_phase_with_only_frameless_hits_emits_no_row(self):
        self.hits.append(self.hit(None, "q"))
        row, = self.rows()
        self.assertEqual(row["phase"], "p")

    def test_modified_skipped(self):
        for key in ("stubs", "pokes"):
            with self.subTest(key=key):
                self.spec[key] = [dict(at="f")]
                self.assertEqual(self.rows(), [])
                del self.spec[key]
        self.capture = self.runtime / "sample-stub-f"
        self.capture.mkdir()
        self.assertEqual(self.rows(), [])

    def test_incomplete_skipped(self):
        self.manifest["q124_complete"] = False
        self.assertEqual(self.rows(), [])

    def test_check_bytes(self):
        self.assertEqual(screens.main(["build"]), 0)
        self.assertEqual(screens.main(["check"]), 0)
        out = self.runtime / "evidence/screens.tsv"
        out.write_bytes(out.read_bytes() + b"x")
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(screens.main(["check"]), 1)


if __name__ == "__main__":
    unittest.main()
