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
        scenario_patch = patch.object(matrix.scen, "HERE", str(self.root / "tools/trace/scenarios"))
        scenario_patch.start()
        self.addCleanup(scenario_patch.stop)
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
        spec = {"values": [{"at": "D_80010010", "ptr_off": 4, "len": 2}]} if values else {}
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
            "capture\tphase\tsymbol/entry\toffset\tbytes\terror\tframe\tentry\taddr\tread_at\tend\tpause_frame\tptr\tlen\n"
            "v\tfight\tD_80010010\t4\taabb\t\t5\t0\t0x80010010\t0x80010020\tfalse\t5\t0x8001001C\t2\n"
            "v\tfight\tD_80010010\t4\taabb\t\t5\t0\t0x80010010\t0x80010020\tfalse\t5\t0x8001001C\t2\n"
            "v\tfight\tD_80010010\t4\t\tpointer is not RAM or scratchpad\t\t0\t\t\ttrue\tnull\t0x00000000\t2\n")
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
                    "polls.tsv": "capture\tphase\tsymbol/entry\taddr\tt\tbytes\tlen\n",
                    "sources.tsv": expected_sources,
                    "scenarios.tsv": "capture\tscenario\textends\tcovers\tkind\tmanifest_sha1\tmatch\n"
                    + "".join(f"{p.name}\t-\t-\t-\thitmap\t"
                              f"{hashlib.sha1((p / 'capture.manifest.json').read_bytes()).hexdigest()}\t-\n"
                              for p in [self.a, self.b])}
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

    def scenario(self, name, **spec):
        library = self.root / "tools/trace/scenarios"
        library.mkdir(parents=True, exist_ok=True)
        spec.setdefault("end_frame", 10)
        self.write(library / (name + ".json"), json.dumps(spec) + "\n")

    def test_scenario_index_claimed_and_unclaimed(self):
        self.scenario("base", covers=["mode:base"])
        self.scenario("middle", extends="base")
        self.scenario("run", extends="middle", covers=["menu:top", "event:win"],
                      evidence=["a/capture.manifest.json"])
        outputs = matrix.render([self.b, self.a])
        rows = list(matrix.csv.DictReader(io.StringIO(outputs["scenarios.tsv"]), delimiter="\t"))
        sha1 = hashlib.sha1((self.a / "capture.manifest.json").read_bytes()).hexdigest()
        self.assertEqual(rows, [
            dict(capture="a", scenario="run", extends="base<middle<run",
                 covers="event:win,menu:top", kind="hitmap", manifest_sha1=sha1, match="evidence"),
            dict(capture="b", scenario="-", extends="-", covers="-",
                 kind="hitmap", manifest_sha1=sha1, match="-")])
        self.assertEqual([r["capture"] for r in rows],
                         outputs["coverage.tsv"].splitlines()[0].split("\t")[5:])
        # The scenario loader's native spec.json evidence joins the same manifest.
        self.scenario("another", covers=["menu:top", "mode:versus"], evidence=["a/spec.json"])
        row = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.a, self.b])[
            "scenarios.tsv"]), delimiter="\t"))[0]
        self.assertEqual(row["scenario"], "another,run")
        self.assertEqual(row["covers"], "event:win,menu:top,mode:versus")
        self.assertEqual(row["extends"], "base<middle<run")

    def test_scenario_missing_base_bad_evidence_and_cycle(self):
        self.scenario("run", extends="missing", evidence=["a/capture.manifest.json"])
        code, error = self.cli("build")
        self.assertEqual(code, 1)
        self.assertIn("no scenario 'missing'", error)
        for evidence in ["absent/capture.manifest.json", "a/unrelated.json", "v/spec.json"]:
            self.scenario("run", evidence=[evidence])
            with self.assertRaisesRegex(matrix.InvalidCapture,
                                        "scenario run: evidence .*is not a capture read"):
                matrix.render([self.a, self.b], all_evidence=True)
        self.scenario("run", extends="run")
        self.assertIn("extends cycle", self.cli("build")[1])
        self.assertFalse(self.out.exists())

    def test_explicit_capture_skips_other_evidence_but_all_refuses(self):
        self.scenario("run", evidence=["b/spec.json"])
        self.assertEqual(self.cli("build", [self.a]), (0, ""))
        self.assertEqual(self.cli("check", [self.a]), (0, ""))
        with contextlib.redirect_stderr(io.StringIO()) as stderr:
            code = matrix.main(["build", str(self.a), "--all", "--out", str(self.out)])
        self.assertEqual(code, 1)
        self.assertIn("scenario run: evidence", stderr.getvalue())

    def test_scenario_evidence_uses_normalized_case(self):
        self.scenario("run", evidence=["A/SPEC.JSON"])
        # Exercise Windows-style case normalization even on a POSIX test host.
        with patch.object(matrix.os.path, "normcase", side_effect=lambda p: str(p).lower()):
            rows = list(matrix.csv.DictReader(io.StringIO(matrix.render(
                [self.a], all_evidence=True)["scenarios.tsv"]), delimiter="\t"))
        self.assertEqual((rows[0]["scenario"], rows[0]["match"]), ("run", "evidence"))

    def test_scenario_staleness(self):
        self.assertEqual(self.cli("build"), (0, ""))
        self.assertEqual(self.cli("check"), (0, ""))
        self.scenario("run", evidence=["a/spec.json"], covers=["menu:top"])
        code, error = self.cli("check")
        self.assertEqual((code, error.strip()), (1, "matrix: matrix differs: scenarios.tsv"))
        self.assertEqual(self.cli("build"), (0, ""))
        self.assertEqual(self.cli("check"), (0, ""))
        (self.out / "scenarios.tsv").unlink()
        self.assertEqual(self.cli("check")[0], 1)

    def test_scenario_combined_kinds(self):
        path = self.a / "capture.jsonl"
        events = [json.loads(line) for line in path.read_text().splitlines()]
        events[0]["spec"].update(values=[{"at": "recorded"}], stubs=[{"at": "recorded"}])
        self.write(path, "".join(json.dumps(e) + "\n" for e in events))
        row = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.a])[
            "scenarios.tsv"]), delimiter="\t"))[0]
        self.assertEqual(row["kind"], "hitmap,stub,values")

    def test_scenario_script_match_and_precedence(self):
        self.scenario("run", inputs=["1:CROSS/1"], covers=["menu:top"])
        script = matrix.scen.compile_script("run")
        path = self.b / "capture.jsonl"
        events = [json.loads(line) for line in path.read_text().splitlines()]
        # Instrumentation is outside the script and cannot prevent a match.
        events[0]["spec"] = {"script": script, "values": [{"at": "recorded"}],
                              "hitmap": {"tus": ["main/*"]}}
        self.write(path, "".join(json.dumps(e) + "\n" for e in events))
        def row():
            return list(matrix.csv.DictReader(io.StringIO(matrix.render([self.b])[
                "scenarios.tsv"]), delimiter="\t"))[0]
        self.assertEqual((row()["scenario"], row()["match"]), ("run", "script"))
        self.scenario("explicit", evidence=["b/spec.json"])
        self.assertEqual((row()["scenario"], row()["match"]), ("explicit", "evidence"))

    def test_scenario_dirname_fallback_requires_existing_scenario(self):
        self.scenario("run")
        hitmap = "function\taddr\tfight\nf\t0x80010000\t1\n"
        known = self.capture("lib-run-2", 10, hitmap)
        unknown = self.capture("lib-missing-1", 20, hitmap)
        rows = list(matrix.csv.DictReader(io.StringIO(matrix.render([known, unknown])[
            "scenarios.tsv"]), delimiter="\t"))
        self.assertEqual([(r["scenario"], r["match"]) for r in rows],
                         [("-", "-"), ("run", "dirname")])

    def test_scenario_script_comparison_is_exact(self):
        self.scenario("run")
        script = matrix.scen.compile_script("run")
        script["end_frame"] = float(script["end_frame"])
        path = self.a / "capture.jsonl"
        events = [json.loads(line) for line in path.read_text().splitlines()]
        events[0]["spec"]["script"] = script
        self.write(path, "".join(json.dumps(e) + "\n" for e in events))
        row = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.a])[
            "scenarios.tsv"]), delimiter="\t"))[0]
        self.assertEqual((row["scenario"], row["match"]), ("-", "-"))

    def test_value_length_from_watch_and_byte_fallback(self):
        path = self.v / "capture.jsonl"
        events = [json.loads(line) for line in path.read_text().splitlines()]
        events[0]["spec"]["values"][0]["len"] = 16
        self.write(path, "".join(json.dumps(e) + "\n" for e in events))
        rows = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.v])["values.tsv"]),
                                         delimiter="\t"))
        self.assertEqual([r["len"] for r in rows], ["16", "16", "16"])
        self.assertEqual([r["entry"] for r in rows], ["0", "0", "0"])
        del events[0]["spec"]["values"][0]["len"]
        self.write(path, "".join(json.dumps(e) + "\n" for e in events))
        rows = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.v])["values.tsv"]),
                                         delimiter="\t"))
        # no len in the watch: gdbtrace's default 4, for the failed reading too
        self.assertEqual([r["len"] for r in rows], ["4", "4", "4"])
        for event in events:
            if event["ev"] == "value":
                event["i"] = 99
        self.write(path, "".join(json.dumps(e) + "\n" for e in events))
        rows = list(matrix.csv.DictReader(io.StringIO(matrix.render([self.v])["values.tsv"]),
                                         delimiter="\t"))
        # an unresolved watch index: the length of the recorded bytes
        self.assertEqual([r["len"] for r in rows], ["2", "2", "0"])

    def test_repo_relative_defaults_and_derived_facts(self):
        old_cwd = Path.cwd()
        try:
            os.chdir(self.a)
            args = ["--no-name", "no_name.tsv"] + [str(p) for p in self.paths]
            self.assertEqual(matrix.main(["build"] + args), 0)
            out = self.root / "docs/naming/runtime/matrix"
            self.assertTrue((out / "coverage.tsv").is_file())
            self.assertFalse((self.a / "docs").exists())
            self.write(out / "facts.tsv", "derived facts\n")
            self.assertEqual(matrix.main(["check"] + args), 0)
            self.write(out / "unexpected.tsv", "unexpected\n")
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(matrix.main(["check"] + args), 1)
        finally:
            os.chdir(old_cwd)

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

    def test_poke_capture_is_sources_only_like_a_stub(self):
        path = self.capture("poked", 45)
        events = [json.loads(line) for line in (path / "capture.jsonl").read_text().splitlines()]
        events[0]["spec"] = {"pokes": [{"at": "D_80010010", "hex": "06", "phase": "fight"}]}
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        outputs = matrix.render([path])
        self.assertIn("poked\tstub\ttrue", outputs["sources.tsv"])
        self.assertEqual(outputs["coverage.tsv"],
                         "addr\tcapture_name\tother_capture_names\tno_name\tcaptures_hit\n")

    def test_events_capture_sources_only(self):
        path = self.capture("events", 50)
        events = [json.loads(line) for line in (path / "capture.jsonl").read_text().splitlines()]
        events[0]["spec"] = {"breakpoints": [{"at": "f"}], "watch": [{"at": "D_80010010"}]}
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        phases = "phase\tfirst_frame\tlast_frame\tscreen\tinputs\tscreenshots\nfight\t0\t9\t-\t-\t-\n"
        self.write(path / "phases.tsv", phases)
        outputs = matrix.render([path, self.a])
        self.assertEqual(outputs["coverage.tsv"].splitlines()[0],
                         "addr\tcapture_name\tother_capture_names\tno_name\tcaptures_hit\ta")
        rows = [r.split("\t") for r in outputs["sources.tsv"].splitlines()[1:]]
        self.assertEqual({(r[1], r[7]) for r in rows if r[0] == "events"},
                         {("events", "capture.jsonl"), ("events", "capture.manifest.json"),
                          ("events", "phases.tsv")})
        # a hit-map declaration still needs both TSVs
        events[0]["spec"]["hitmap"] = {"tus": ["main/*"]}
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        with self.assertRaises(OSError):
            matrix.load_capture(path)
        # breakpoints alone with no phases.tsv: still sources only
        del events[0]["spec"]["hitmap"]
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        (path / "phases.tsv").unlink()
        self.assertEqual(matrix.load_capture(path)["kind"], "events")
        self.assertFalse(matrix.load_capture(path)["mapped"])

    def test_poll_rows_in_order_with_length(self):
        path = self.capture("polls", 60)
        events = [json.loads(line) for line in (path / "capture.jsonl").read_text().splitlines()]
        events[0]["spec"] = {"polls": [{"at": "0x80010010", "len": 4, "every_s": 0.5},
                                       {"at": "D_80010020"}]}
        events += [{"ev": "poll", "t": 0.0, "at": "0x80010010", "addr": 2147549200,
                    "hex": "01020304", "phase": "boot"},
                   {"ev": "poll", "t": 0.5, "at": "D_80010020", "addr": 2147549216,
                    "hex": "aabbccdd"},
                   {"ev": "poll", "t": 1.0, "at": "unknown", "addr": 2147549232, "hex": "ee"}]
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        outputs = matrix.render([path, self.a])
        self.assertEqual(outputs["polls.tsv"],
                         "capture\tphase\tsymbol/entry\taddr\tt\tbytes\tlen\n"
                         "polls\tboot\t0x80010010\t0x80010010\t0.0\t01020304\t4\n"
                         "polls\t\tD_80010020\t0x80010020\t0.5\taabbccdd\t4\n"
                         "polls\t\tunknown\t0x80010030\t1.0\tee\t1\n")
        # polls stay out of coverage and values
        self.assertEqual(outputs["coverage.tsv"].splitlines()[0].split("\t")[-1], "a")
        self.assertEqual(outputs["values.tsv"].count("\n"), 1)
        self.assertEqual(matrix.load_capture(path)["kind"], "events")
        del events[-1]["hex"]
        self.write(path / "capture.jsonl", "".join(json.dumps(e) + "\n" for e in events))
        with self.assertRaisesRegex(matrix.InvalidCapture, "malformed poll event"):
            matrix.render([path])

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
