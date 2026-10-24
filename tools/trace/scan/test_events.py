#!/usr/bin/env python3
"""Synthetic watch/call captures; no emulator or build required."""

import contextlib
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import sys
import shutil
import uuid
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[3]))
from tools.trace.scan import events


class EventsTest(unittest.TestCase):
    def setUp(self):
        scratch = events.REPO_ROOT / "tmp"
        scratch.mkdir(exist_ok=True)
        self.root = scratch / ("events-test-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(self.cleanup)
        self.runtime = self.root / "docs/naming/runtime"
        self.capture = self.runtime / "sample"
        self.capture.mkdir(parents=True)
        root_patch = patch.object(events, "REPO_ROOT", self.root)
        root_patch.start()
        self.addCleanup(root_patch.stop)
        self.tracked = ["docs/naming/runtime/sample/capture.manifest.json"]
        git_patch = patch.object(events.subprocess, "run", side_effect=lambda *a, **kw:
                                 SimpleNamespace(stdout="\0".join(self.tracked).encode()))
        git_patch.start()
        self.addCleanup(git_patch.stop)
        self.records = []
        self.spec = dict(breakpoints=[dict(at="func_80010000", max_hits=100)],
                         watch=[dict(at="D_800A3910+0x2", kind=k, max_hits=100, len=2)
                                for k in ("write", "read")], max_events=1000)
        self.end_changes = {}
        for phase, frame in (("p", 1), ("q", 8)):
            self.records.extend([
                self.watch(2, phase, frame), self.watch(3, phase, frame + 1),
                dict(ev="hit", at="func_80010000", kind=0, ra=0x80020008,
                     phase=phase, frame=frame + 2, args=[1, 2, 3, 4])])
        self.save()

    def cleanup(self):
        expected = Path(__file__).resolve().parents[3] / "tmp"
        if self.root.resolve().parent != expected or not self.root.name.startswith("events-test-"):
            raise ValueError("unsafe scratch cleanup")
        for path in sorted(self.root.rglob("*"), key=lambda p: len(p.parts), reverse=True):
            if path.is_dir():
                path.rmdir()
            else:
                path.unlink()
        self.root.rmdir()

    def watch(self, kind=2, phase="p", frame=1, value=7):
        return dict(ev="hit", at="D_800A3910+0x2", kind=kind, phase=phase,
                    frame=frame, addr=0x800A3912,
                    access=dict(pc=0x80030010, addr=0x800A3912,
                                           width=2, store=kind == 2, value=value))

    def save(self):
        hits = {}
        for r in self.records:
            if r.get("ev") == "hit":
                key = f"Z{r['kind']}:{r['at']}"
                hits[key] = hits.get(key, 0) + 1
        end = dict(ev="end", events=len(self.records), script_ended=True, hits=hits)
        end.update(self.end_changes)
        records = [dict(ev="start", spec=self.spec), *self.records, end]
        raw = "".join(json.dumps(r) + "\n" for r in records).encode()
        (self.capture / "capture.jsonl").write_bytes(raw)
        (self.capture / "capture.manifest.json").write_bytes(
            b'{"q124_complete":true}\n')
        counts = {kind: sum(r.get("kind") == kind for r in self.records)
                  for kind in (0, 2, 3)}
        report = "capture SHA1 " + hashlib.sha1(raw).hexdigest() + "\n\n"
        for kind, n in counts.items():
            if not n:
                continue
            if kind == 0:
                report += (f"## func_80010000: {n} hits\n\nCall sites (ra-8):\n"
                           f"- `caller+0x0` x{n}\n\n")
            else:
                report += (f"## D_800A3910+0x2 (Z{kind} watch): {n} hits\n\n"
                           f"Accessing instructions:\n- `func_80030000+0x10` x{n}\n\n")
        (self.capture / "report.md").write_bytes(report.encode())

    def rows(self):
        return list(csv.DictReader(io.StringIO(events.render()), delimiter="\t"))

    def test_two_phases_write_read_call_and_citations(self):
        rows = self.rows()
        self.assertEqual(len(rows), 6)
        self.assertEqual({r["phase"] for r in rows}, {"p", "q"})
        self.assertEqual({r["kind"] for r in rows}, {"write", "read", "call"})
        digest = hashlib.sha1((self.capture / "capture.jsonl").read_bytes()).hexdigest()
        for row in rows:
            self.assertEqual(row["count"], "1")
            self.assertEqual(row["inconclusive"], "0")
            self.assertEqual(row["captures"], "sample@" + digest[:12])
            self.assertNotIn("first_frame", row)
            if row["kind"] == "call":
                self.assertEqual(row["site_function"], "caller")
                self.assertEqual(row["site"], "0x80020000")
                self.assertEqual(json.loads(row["values"]), [[1, 2, 3, 4]])
            else:
                self.assertEqual(row["target"], "D_800A3910+0x2")
                self.assertEqual(row["site_function"], "func_80030000")
                self.assertEqual(row["site"], "0x80030010")
                self.assertEqual(json.loads(row["values"]), [7])

    def test_value_and_argument_capping_sort_and_counts(self):
        call = self.records[2]
        self.records = [self.watch(frame=v, value=v) for v in range(10, -1, -1)]
        self.records += [dict(call, frame=v, args=[v, 2, 3, 4]) for v in range(10, -1, -1)]
        self.save()
        rows = self.rows()
        self.assertEqual(len(rows), 2)
        for row in rows:
            self.assertEqual(row["count"], "11")
            vals = json.loads(row["values"])
            self.assertEqual(vals[-1], "+3 more")
            self.assertEqual(vals[:-1], list(range(8)) if row["kind"] == "write"
                             else [[v, 2, 3, 4] for v in range(8)])

    def test_partial_data_and_raw_site_are_inconclusive(self):
        self.records[0].pop("frame")
        self.records[1]["access"]["value"] = "?"
        self.records[2]["args"][2] = "?"
        self.records[2]["phase"] = "?"
        self.save()
        report = self.capture / "report.md"
        report.write_bytes(report.read_bytes().replace(b"caller+0x0", b"0x80020000"))
        rows = self.rows()
        self.assertTrue(any(r["kind"] == "write" and r["inconclusive"] == "0"
                            for r in rows))
        self.assertTrue(any("missing_value" in r["reason"] for r in rows))
        self.assertTrue(any("partial_args" in r["reason"] for r in rows))
        self.assertTrue(all(r["inconclusive"] == "1" for r in rows if r["kind"] == "call"))

    def test_committed_complete_only_and_nested_capture(self):
        nested = self.runtime / "family/sample"
        shutil.copytree(self.capture, nested)
        self.tracked.append("docs/naming/runtime/family/sample/capture.manifest.json")
        rows = self.rows()
        self.assertEqual(len(rows), 12)
        self.assertTrue(all(r["count"] == "1" for r in rows))
        self.assertTrue(all(";" not in r["captures"] for r in rows))
        self.assertEqual({r["captures"].split("@")[0] for r in rows},
                         {"sample", "family/sample"})
        self.tracked = []
        self.assertEqual(self.rows(), [])
        self.tracked = ["docs/naming/runtime/sample/capture.manifest.json"]
        (self.capture / "capture.manifest.json").write_bytes(b'{"q124_complete":false}')
        self.assertEqual(self.rows(), [])

    def test_malformed_event_reports_capture_and_line(self):
        for broken in (dict(ev="hit", kind=99, at="x"),
                       dict(self.watch(), access=[]),
                       dict(self.records[2], args=[1]), ["not an event"]):
            with self.subTest(broken=broken):
                (self.capture / "capture.jsonl").write_bytes(
                    (json.dumps(broken) + "\n").encode())
                with contextlib.redirect_stderr(io.StringIO()) as error:
                    self.assertEqual(events.main(["build"]), 1)
                self.assertIn("sample", error.getvalue())
                self.assertIn("capture.jsonl:1:", error.getvalue())

    def test_build_check_staleness_repo_relative_and_lf(self):
        old_cwd = Path.cwd()
        try:
            os.chdir(self.capture)
            self.assertEqual(events.main(["build"]), 0)
            self.assertEqual(events.main(["check"]), 0)
            out = self.runtime / "evidence/events.tsv"
            original = out.read_bytes()
            self.assertNotIn(b"\r", original)
            self.assertEqual(events.main(["build"]), 0)
            self.assertEqual(out.read_bytes(), original)
            out.write_bytes(original + b"stale\n")
            with contextlib.redirect_stderr(io.StringIO()) as error:
                self.assertEqual(events.main(["check"]), 1)
            self.assertIn("events differ", error.getvalue())
        finally:
            os.chdir(old_cwd)

    def test_report_mismatch_fails_and_missing_report_is_partial(self):
        report = self.capture / "report.md"
        report.write_bytes(report.read_bytes().replace(b"x2", b"x3"))
        with self.assertRaisesRegex(ValueError, "report site counts disagree"):
            self.rows()
        report.unlink()
        self.assertTrue(all(r["inconclusive"] == "1" for r in self.rows()))

    def test_hitmap_stub_and_undeclared_breakpoints_are_excluded(self):
        self.records = [dict(self.records[2], **{flag: 1})
                        for flag in ("hm", "stub_call", "stub_leak")]
        self.records.append(dict(self.records[0], at="undeclared"))
        self.save()
        self.assertEqual(self.rows(), [])

    def test_missing_frames_are_not_inconclusive(self):
        for r in self.records:
            r.pop("frame")
        self.save()
        self.assertTrue(all(r["inconclusive"] == "0" for r in self.rows()))

    def test_capture_limits_are_specific_and_target_local(self):
        self.spec["watch"][0]["max_hits"] = 2
        self.save()
        for row in self.rows():
            self.assertEqual(row["inconclusive"], "1" if row["kind"] == "write" else "0")
            if row["kind"] == "write":
                self.assertIn("max_hits_reached:Z2:D_800A3910+0x2:2", row["reason"])
        self.spec["breakpoints"][0]["max_hits"] = 2
        self.spec["max_events"] = 6
        self.end_changes["script_ended"] = False
        self.save()
        for row in self.rows():
            self.assertIn("max_events_reached:6", row["reason"])
            self.assertIn("script_not_ended", row["reason"])
            if row["kind"] == "call":
                self.assertIn("max_hits_reached:Z0:func_80010000:2", row["reason"])

    def test_kseg1_and_bios_normalization(self):
        self.assertEqual(events.canonical(0xA00A3910), events.canonical(0x800A3910))
        self.assertEqual(events.normalized_target("D_A00A3910+0x2"), "D_800A3910+0x2")
        self.assertEqual(events.normalized_target("0xA00A3910"), "0x800A3910")
        watch = self.watch()
        watch["ra"] = 0x80040008  # A watch must use access.pc, never this ra.
        watch["access"]["pc"] = 0xBFC0D864
        key, _, reasons = events.observation(watch, {})
        self.assertEqual(key[2], "BIOS")
        self.assertEqual(key[4:6], ("0xBFC0D864", "bios"))
        self.assertNotIn("unresolved_function", reasons)
        watch["access"]["pc"] = 0xA0089010
        resolved = {}
        events.resolve_ranges([watch], resolved, [(0x80089000, 0x80089100, "sdk_writer")])
        key, _, reasons = events.observation(watch, resolved)
        self.assertEqual(key[2], "sdk_writer")
        self.assertFalse(reasons)

    def test_absolute_manifest_elf_path_is_ignored(self):
        before = events.render()
        manifest = dict(q124_complete=True, build=dict(elf=dict(
            path=str(self.root / "nonexistent.elf"), sha1="0" * 40)))
        (self.capture / "capture.manifest.json").write_bytes(json.dumps(manifest).encode())
        self.assertEqual(events.render(), before)
        (self.root / "nonexistent.elf").write_bytes(b"mismatching local build")
        self.assertEqual(events.render(), before)
        manifest["build"]["elf"]["path"] = "C:/other/worktree/build/bb2.elf"
        (self.capture / "capture.manifest.json").write_bytes(json.dumps(manifest).encode())
        self.assertEqual(events.render(), before)

    def test_exact_wider_and_partial_stores(self):
        entry = events.declarations(self.spec)[("D_800A3910+0x2", 2)]
        resolved = {(("D_800A3910+0x2", 2), 0x80030010): ("writer", "report")}
        for address, width, value, expected, offset, partial in (
                (0x800A3910, 4, 0x00150000, 0x15, "-2", False),
                (0x800A3912, 1, 0x15, 0x15, "0", True),
                (0x800A3912, 2, 0x1234, 0x1234, "0", False),
                (0x800A3911, 2, 0x1500, 0x15, "-1", True)):
            with self.subTest(address=address, width=width):
                watch = self.watch()
                watch["access"].update(addr=address, width=width, value=value)
                key, actual, reasons = events.observation(watch, resolved, entry)
                self.assertEqual(actual, expected)
                self.assertEqual(key[6:], (offset, str(width)))
                self.assertEqual("partial_store" in reasons, partial)
                self.assertFalse(reasons - {"partial_store"})
        self.records = [self.watch(), self.watch()]
        self.records[1]["access"].update(width=1)
        self.save()
        rows = self.rows()
        self.assertEqual(len(rows), 2)
        self.assertTrue(all(r["inconclusive"] == "0" for r in rows))
        self.assertEqual({r["store_width"] for r in rows}, {"1", "2"})
        _, value, reasons = events.observation(self.watch(), resolved, dict(entry, len=4))
        self.assertEqual(value, 7)
        self.assertEqual(reasons, {"partial_store"})

    def test_coverage_attribution_and_mixed_group_completeness(self):
        self.records = [self.watch(), self.watch(), self.watch()]
        self.records[1]["access"]["pc"] = 0x80089010
        self.records[2]["access"]["pc"] = 0x80090010
        self.save()
        # The committed report resolves just the first site; raw labels do not
        # establish a function. The final coverage start has no inferred end.
        report = self.capture / "report.md"
        report.write_bytes(report.read_bytes().replace(
            b"- `func_80030000+0x10` x3",
            b"- `func_80030000+0x10` x1\n- `0x80089010` x1\n- `0x80090010` x1"))
        matrix = self.runtime / "matrix"
        matrix.mkdir()
        (matrix / "coverage.tsv").write_bytes(
            b"addr\tcapture_name\n0x80089000\tsdk_writer\n0x80089100\tlast\n")
        rows = self.rows()
        self.assertEqual(len(rows), 3)
        self.assertEqual({r["attrib"] for r in rows}, {"report", "coverage", "none"})
        self.assertEqual(next(r["site_function"] for r in rows if r["attrib"] == "coverage"),
                         "sdk_writer")
        self.assertTrue(all(r["inconclusive"] == "1" for r in rows))
        self.assertTrue(all("group_has_unresolved_site" in r["reason"] for r in rows))


if __name__ == "__main__":
    unittest.main(verbosity=2)
