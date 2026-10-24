#!/usr/bin/env python3
"""Synthetic, stdlib-only coverage matrix regression fixtures."""

import contextlib
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import sys
import unittest
from unittest.mock import patch
import uuid

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import matrix


class MatrixTest(unittest.TestCase):
    def setUp(self):
        # Keep synthetic fixtures in the workspace, including under a sandbox.
        scratch = Path(__file__).resolve().parents[2] / "tmp"
        scratch.mkdir(parents=True, exist_ok=True)
        self.root = scratch / ("matrix-test-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(self.cleanup)
        root_patch = patch.object(matrix, "REPO_ROOT", self.root)
        root_patch.start()
        self.addCleanup(root_patch.stop)
        self.a = self.capture("a", 10, "function\taddr\tboot\tfight\n"
                              "old\t0x80010000\t1+\t?\n"
                              "quiet\t0x80010004\t.\t.\n")
        self.b = self.capture("b", 20, "function\taddr\tfight\n"
                              "new\t0x80010000\t2\n"
                              "extra\t0x80010008\t?\n")
        self.v = self.capture("v", 30, values=True)
        self.registry = self.root / "no_name.tsv"
        self.write(self.registry, "address\tname\n0x80010000\tignored\n")
        self.paths = [self.v, self.b, self.a]
        self.out = self.root / "matrix"

    def cleanup(self):
        expected_parent = Path(__file__).resolve().parents[2] / "tmp"
        if self.root.resolve().parent != expected_parent or not self.root.name.startswith("matrix-test-"):
            raise RuntimeError("fixture cleanup escaped workspace scratch directory")
        shutil.rmtree(self.root)

    def write(self, path, text):
        with path.open("w", encoding="utf-8", newline="\n") as stream:
            stream.write(text)

    def capture(self, name, time, hitmap=None, values=False, stub=False):
        path = self.root / name
        path.mkdir(parents=True)
        manifest = {"q124_complete": True, "git": {"head": "commit", "dirty": False,
                                                   "tools_trace_status": "clean"},
                    "build": {"exe": {"sha1": "oracle"}}}
        self.write(path / "capture.manifest.json", json.dumps(manifest) + "\n")
        spec = {"values": [{"at": "D_80010010", "ptr_off": 4}]} if values else {}
        if stub:
            spec["stubs"] = [{"at": "recorded"}]
        events = [{"ev": "start", "time": time, "spec": spec}]
        if values:
            reading = {"ev": "value", "i": 0, "phase": "fight", "at": "D_80010010",
                       "hex": "aabb", "frame": 5, "addr": 2147549200,
                       "read_at": 2147549216, "end": False, "pause_frame": 5,
                       "ptr": 2147549212}
            events.extend([reading, reading.copy()])
            events.append({"ev": "value", "i": 0, "phase": "fight", "at": "D_80010010",
                           "end": True, "pause_frame": None, "ptr": 0,
                           "error": "pointer is not RAM or scratchpad"})
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        if hitmap is not None:
            self.write(path / "hitmap.tsv", hitmap)
            self.write(path / "phases.tsv",
                       "phase\tfirst_frame\tlast_frame\tscreen\tinputs\tscreenshots\n"
                       "boot\t0\t4\t-\t-\t-\nfight\t5\t9\t-\t-\t-\n")
        return path

    def cli(self, command, paths=None):
        with contextlib.redirect_stderr(io.StringIO()) as stderr:
            code = matrix.main([command, "--out", str(self.out), "--no-name",
                                str(self.registry)] + [str(p) for p in (paths or self.paths)])
        return code, stderr.getvalue()

    def test_exact_outputs(self):
        expected_coverage = (
            "addr\tcapture_name\tother_capture_names\tno_name\tcaptures_hit\ta\tb\n"
            "0x80010000\tnew\told\t1\t2\tboot:1+;fight:?\tfight:2\n"
            "0x80010004\tquiet\t\t0\t0\t.\t-\n"
            "0x80010008\textra\t\t0\t0\t-\tfight:?\n")
        expected_values = (
            "capture\tphase\tsymbol/entry\toffset\tbytes\terror\tframe\tentry\taddr\tread_at\tend\tpause_frame\tptr\n"
            "v\tfight\tD_80010010\t4\taabb\t\t5\t0\t0x80010010\t0x80010020\tfalse\t5\t0x8001001C\n"
            "v\tfight\tD_80010010\t4\taabb\t\t5\t0\t0x80010010\t0x80010020\tfalse\t5\t0x8001001C\n"
            "v\tfight\tD_80010010\t4\t\tpointer is not RAM or scratchpad\t\t0\t\t\ttrue\tnull\t0x00000000\n")
        expected_sources = ("capture\tkind\tq124_complete\tgit_commit\ttools_trace_status\t"
                            "build_sha1\tstart_time\tinput_file\tinput_sha1\n")
        for path, kind, time in [(self.a, "hitmap", 10), (self.b, "hitmap", 20),
                                 (self.v, "values", 30)]:
            inputs = {p.name: p.read_bytes() for p in path.iterdir()}
            inputs["no_name.tsv"] = self.registry.read_bytes()
            for name, data in sorted(inputs.items()):
                expected_sources += (f"{path.name}\t{kind}\ttrue\tcommit\tclean\toracle\t{time}\t"
                                     f"{name}\t{hashlib.sha1(data).hexdigest()}\n")
        expected = {"coverage.tsv": expected_coverage, "values.tsv": expected_values,
                    "sources.tsv": expected_sources}
        self.assertEqual(matrix.render(self.paths, self.registry), expected)
        self.assertEqual(matrix.render(list(reversed(self.paths)), self.registry), expected)
        self.assertEqual(self.cli("build"), (0, ""))
        for name, content in expected.items():
            self.assertEqual((self.out / name).read_bytes(), content.encode("utf-8"))

    def test_check_and_corruption(self):
        self.assertEqual(self.cli("build"), (0, ""))
        self.assertEqual(self.cli("check"), (0, ""))
        path = self.out / "coverage.tsv"
        self.write(path, path.read_text(encoding="utf-8").replace("new", "New", 1))
        code, error = self.cli("check")
        self.assertEqual(code, 1)
        self.assertIn("coverage.tsv", error)

    def test_q124_refused_without_output(self):
        for marker in [False, "true", None]:
            manifest = {"q124_complete": marker} if marker is not None else {}
            self.write(self.b / "capture.manifest.json", json.dumps(manifest))
            code, error = self.cli("build")
            self.assertEqual(code, 1)
            self.assertIn("q124_complete", error)
            self.assertFalse(self.out.exists())

    def test_malformed_or_missing_tables_refused(self):
        path = self.b / "hitmap.tsv"
        for content in ["wrong\n", "function\taddr\tfight\nf\tbad\t1\n",
                        "function\taddr\tfight\nf\t0x80010000\t0\n",
                        "function\taddr\tfight\nf\t0x80010000\n",
                        "function\taddr\tunknown\nf\t0x80010000\t1\n",
                        "function\taddr\tfight\nf\t0x80010000\t1\n"
                        "g\t0x80010000\t2\n"]:
            with self.subTest(content=content):
                self.write(path, content)
                self.assertEqual(self.cli("build")[0], 1)
        path.unlink()
        self.assertEqual(self.cli("build")[0], 1)
        self.write(path, "function\taddr\tfight\nf\t0x80010000\t1\n")
        self.write(self.b / "phases.tsv", "phase\tfirst_frame\tlast_frame\tscreen\tinputs\tscreenshots\n"
                   "fight\t9\t5\t-\t-\t-\n")
        self.assertEqual(self.cli("build")[0], 1)
        (self.b / "phases.tsv").unlink()
        self.assertEqual(self.cli("build")[0], 1)

    def test_stub_sources_only_and_duplicate_ids(self):
        stub = self.capture("docs/naming/runtime/scenario-stub-1/baseline", 40, stub=True)
        outputs = matrix.render([stub])
        self.assertEqual(outputs["coverage.tsv"],
                         "addr\tcapture_name\tother_capture_names\tno_name\tcaptures_hit\n")
        self.assertIn("scenario-stub-1/baseline\tstub\ttrue", outputs["sources.tsv"])
        with self.assertRaisesRegex(matrix.InvalidCapture, "duplicate capture ID"):
            matrix.render([self.a, self.a])

    def test_manifest_time_wins_over_trace_time(self):
        path = self.a / "capture.manifest.json"
        manifest = json.loads(path.read_text())
        manifest["start_time"] = "2026-01-01T00:00:00Z"
        self.write(path, json.dumps(manifest))
        self.assertIn("0x80010000\told\tnew\t", matrix.render(self.paths)["coverage.tsv"])

    def test_all_discovery_and_refused_leaf(self):
        runtime = self.root / "docs/naming/runtime"
        values = self.capture("docs/naming/runtime/z-values-1", 30, values=True)
        stub = self.capture("docs/naming/runtime/a-stub-1/baseline", 40, stub=True)
        excluded = runtime / "matrix/nested"
        excluded.mkdir(parents=True)
        self.write(excluded / "capture.manifest.json", "{}\n")
        self.assertEqual(matrix.discover(runtime), [stub, values])
        old_cwd = Path.cwd()
        try:
            os.chdir(self.a)  # --all must use the script's repository, not cwd.
            for command in ["build", "check"]:
                with contextlib.redirect_stderr(io.StringIO()) as stderr:
                    code = matrix.main([command, "--all", "--out", str(self.out)])
                self.assertEqual((code, stderr.getvalue()), (0, ""))
            self.assertIn("a-stub-1/baseline\tstub\ttrue",
                          (self.out / "sources.tsv").read_text(encoding="utf-8"))
            before = {p.name: p.read_bytes() for p in self.out.iterdir()}
            self.write(stub / "capture.manifest.json", "{}\n")
            for command in ["build", "check"]:
                with contextlib.redirect_stderr(io.StringIO()) as stderr:
                    code = matrix.main([command, "--all", "--out", str(self.out)])
                self.assertEqual(code, 1)
                self.assertIn("q124_complete", stderr.getvalue())
            self.assertEqual({p.name: p.read_bytes() for p in self.out.iterdir()}, before)
        finally:
            os.chdir(old_cwd)

    def test_not_targeted_is_distinct_from_zero_hits(self):
        rows = list(matrix.csv.DictReader(io.StringIO(matrix.render(self.paths)["coverage.tsv"]),
                                         delimiter="\t"))
        quiet, extra = rows[1:]
        self.assertEqual((quiet["a"], quiet["b"]), (".", "-"))
        self.assertEqual((extra["a"], extra["b"]), ("-", "fight:?"))
        self.assertEqual(extra["captures_hit"], "0")

    def test_checkout_stub_name_does_not_classify_captures(self):
        checkout = self.root / "checkout-stub-worktree"
        capture = self.capture("checkout-stub-worktree/docs/naming/runtime/plain", 10,
                               "function\taddr\tfight\nf\t0x80010000\t1\n")
        with patch.object(matrix, "REPO_ROOT", checkout):
            loaded = matrix.load_capture(capture)
        self.assertEqual(loaded["kind"], "hitmap")
        self.assertEqual(loaded["ident"], "plain")

    def test_non_leaf_manifest_refused(self):
        parent = self.capture("docs/naming/runtime/parent", 10, values=True)
        self.capture("docs/naming/runtime/parent/nested/child", 20, values=True)
        with self.assertRaisesRegex(matrix.InvalidCapture, "not a leaf"):
            matrix.discover(self.root / "docs/naming/runtime")
        with self.assertRaisesRegex(matrix.InvalidCapture, "not a leaf"):
            matrix.render([parent])
        for command in ["build", "check"]:
            with contextlib.redirect_stderr(io.StringIO()) as stderr:
                code = matrix.main([command, "--all", "--out", str(self.out)])
            self.assertEqual(code, 1)
            self.assertIn("not a leaf", stderr.getvalue())

    def test_trace_status_does_not_fall_back_to_dirty(self):
        path = self.a / "capture.manifest.json"
        manifest = json.loads(path.read_text())
        del manifest["git"]["tools_trace_status"]
        self.write(path, json.dumps(manifest))
        rows = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.a])["sources.tsv"]),
                                         delimiter="\t"))
        self.assertTrue(all(row["tools_trace_status"] == "" for row in rows))

    def test_capture_selection_required(self):
        with contextlib.redirect_stderr(io.StringIO()) as stderr:
            with self.assertRaises(SystemExit) as raised:
                matrix.main(["build"])
        self.assertEqual(raised.exception.code, 2)
        self.assertIn("provide capture directories or --all", stderr.getvalue())


if __name__ == "__main__":
    unittest.main(verbosity=2)
