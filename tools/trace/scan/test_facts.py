#!/usr/bin/env python3
"""Small synthetic trace matrices, kept inside repository scratch space."""

import contextlib
import csv
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
sys.path.insert(0, str(Path(__file__).resolve().parents[3]))
from tools.trace.scan import facts


class FactsTest(unittest.TestCase):
    def setUp(self):
        scratch = facts.REPO_ROOT / "tmp"
        scratch.mkdir(parents=True, exist_ok=True)
        self.root = scratch / ("facts-test-" + uuid.uuid4().hex)
        self.root.mkdir(parents=True)
        self.addCleanup(self.cleanup)
        root_patch = patch.object(facts, "REPO_ROOT", self.root)
        root_patch.start()
        self.addCleanup(root_patch.stop)
        self.matrix = self.root / "docs/naming/runtime/matrix"
        self.registry = self.root / "docs/naming/phase3/no_name.tsv"
        self.write(self.registry, "address\n0x80010000\n0x80010010\n")
        self.write(self.matrix / "coverage.tsv",
                   "addr\tcapture_name\tother_capture_names\tno_name\tcaptures_hit\ta\tb\tc\n"
                   "0x80010000\trecorded\t\t0\t1\tp:1+\t.\t-\n"
                   "0x80010004\tquiet\t\t0\t0\t.\t-\t-\n"
                   "0x80010008\tpartial\t\t0\t1\tp:1;q:?\t.\t-\n"
                   "0x8001000C\tunknown\t\t0\t0\tp:?\t-\t-\n")
        self.write(self.matrix / "sources.tsv",
                   "capture\tkind\tinput_file\n"
                   "a\thitmap\tphases.tsv\nb\thitmap\tphases.tsv\nc\thitmap\tphases.tsv\n"
                   "v\tvalues\tcapture.jsonl\n")
        self.write(self.matrix / "values.tsv",
                   "capture\tphase\tsymbol/entry\toffset\tbytes\terror\tend\tlen\tentry\tframe\n"
                   "v\tp\tD_80010010\t4\taa\t\tfalse\t1\t0\t0\n"
                   "v\tq\tD_80010010\t4\taa\t\ttrue\t1\t0\t5\n"
                   "v\tp\tD_80010010\t8\taa\t\tfalse\t1\t1\t0\n"
                   "v\tq\tD_80010010\t8\tbb\t\ttrue\t1\t1\t5\n"
                   "v\tq\tD_80010014\t\t\tread failed\ttrue\t1\t2\t5\n")
        for capture in "abc":
            path = self.matrix.parent / capture
            self.write(path / "phases.tsv",
                       "phase\tfirst_frame\tlast_frame\tscreen\n"
                       "p\t0\t4\tliteral screen\nq\t5\t9\t-\nr\t10\t14\t-\n")
            self.write(path / "hitmap.tsv", "function\taddr\tp\tq\n")

    def cleanup(self):
        expected = Path(__file__).resolve().parents[3] / "tmp"
        if self.root.resolve().parent != expected or not self.root.name.startswith("facts-test-"):
            raise RuntimeError("fixture cleanup escaped workspace scratch directory")
        shutil.rmtree(self.root)

    def write(self, path, text):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(text.encode("utf-8"))

    def rows(self):
        return list(csv.DictReader(io.StringIO(facts.render(self.matrix, self.registry)),
                                   delimiter="\t"))

    def get(self, kind, addr):
        return next(r for r in self.rows() if r["kind"] == kind and r["addr"] == addr)

    def test_reach_and_not_targeted(self):
        row = self.get("reach", "0x80010000")
        self.assertEqual(json.loads(row["fact"]), dict(targeted=["a", "b"], hit=["a"],
                         hit_phases={"a": ["p"]}, unknown={}))
        self.assertEqual((row["captures"], row["no_name"], row["capture_name"]),
                         ("a;b", "1", "recorded"))

    def test_never_hit(self):
        row = self.get("never_hit", "0x80010004")
        self.assertEqual((row["captures"], row["inconclusive"]), ("a", "0"))
        self.assertEqual(self.get("never_hit", "0x8001000C")["inconclusive"], "1")
        self.assertFalse(any(r["kind"] == "never_hit" and r["addr"] == "0x80010000"
                             for r in self.rows()))

    def test_reach_unknown_map_is_inconclusive(self):
        row = self.get("reach", "0x80010008")
        self.assertEqual(json.loads(row["fact"])["unknown"], {"a": ["q"]})
        self.assertEqual(row["inconclusive"], "1")

    def test_phase_only_and_unknown(self):
        row = self.get("phase_only", "0x80010000")
        self.assertEqual(row["inconclusive"], "0")
        self.assertEqual(json.loads(row["fact"])["covered_phases"],
                         {"a": ["p", "q"], "b": ["p", "q"]})
        row = self.get("phase_only", "0x80010008")
        self.assertEqual(row["inconclusive"], "1")
        self.assertIn("unknown_cells", json.loads(row["fact"])["reason"])

    def test_phase_only_requires_strict_subset_in_every_capture(self):
        item = dict(addr="0x80010000", cells={"a": {"p": "1", "q": "2"},
                                              "b": {"x": "."}})
        self.assertEqual(facts.phase_only(item), [])
        item["cells"]["a"] = {"p": "1", "q": "."}
        item["cells"]["b"] = {"x": "1"}
        self.assertEqual(facts.phase_only(item), [])

    def test_capture_only(self):
        row = self.get("capture_only", "0x80010000")
        self.assertEqual(json.loads(row["fact"]),
                         dict(hit=["a"], armed_never_hit=["b"], unknown=[]))
        item = dict(addr="x", cells={"a": {"p": "1"}, "b": {"q": "?"}})
        self.assertEqual(facts.capture_only(item)[0]["inconclusive"], "1")
        item["cells"].pop("b")
        self.assertEqual(facts.capture_only(item), [])

    def test_value_constant_and_changes_offsets_and_errors(self):
        constant = self.get("value_constant", "D_80010010")
        self.assertEqual(constant["no_name"], "1")
        detail = json.loads(constant["fact"])
        self.assertEqual(detail["offset"], "4")
        self.assertEqual(detail["values"], {"aa": {"v": ["p", "q"]}})
        self.assertFalse(detail["end"][0]["differs"])
        changes = self.get("value_changes", "D_80010010")
        detail = json.loads(changes["fact"])
        self.assertEqual(detail["offset"], "8")
        self.assertTrue(detail["end"][0]["differs"])
        error = self.get("value_constant", "D_80010014")
        self.assertEqual(error["inconclusive"], "1")
        self.assertEqual(json.loads(error["fact"])["errors"][0]["error"], "read failed")
        self.assertIsNone(json.loads(error["fact"])["end"][0]["differs"])

    def test_value_cross_capture_and_failed_comparison(self):
        item = dict(addr="x", offset="", len="1")
        readings = [dict(capture="a", phase="p", bytes="aa", error="", end="false"),
                    dict(capture="b", phase="q", bytes="bb", error="", end="true"),
                    dict(capture="a", phase="q", bytes="", error="failed", end="false"),
                    dict(capture="a", phase="r", bytes="bb", error="", end="true")]
        for row in readings:
            row["entry"] = "0"
        row = facts.value_changes(item, readings)[0]
        self.assertEqual(row["captures"], "a;b")
        self.assertTrue(all(e["differs"] is None for e in json.loads(row["fact"])["end"]))

    def test_different_watch_lengths_do_not_claim_changes(self):
        self.write(self.matrix / "values.tsv",
                   "capture\tphase\tsymbol/entry\toffset\tbytes\terror\tend\tlen\tentry\tframe\n"
                   "v\tp\tD_80010010\t\t01020304\t\tfalse\t4\t0\t0\n"
                   "v\tq\tD_80010010\t\t01020304\t\ttrue\t4\t0\t5\n"
                   "w\tp\tD_80010010\t\t0102030405060708090a0b0c0d0e0f10\t\tfalse\t16\t0\t0\n"
                   "w\tq\tD_80010010\t\t0102030405060708090a0b0c0d0e0f10\t\ttrue\t16\t0\t5\n")
        rows = [r for r in self.rows() if r["kind"].startswith("value_")]
        self.assertEqual(len(rows), 2)
        self.assertEqual({r["kind"] for r in rows}, {"value_constant"})
        self.assertEqual({json.loads(r["fact"])["len"] for r in rows}, {"4", "16"})
        self.assertTrue(all(json.loads(r["fact"])["end"][0]["differs"] is False for r in rows))

    def test_end_readings_stay_with_same_watch_index(self):
        self.write(self.matrix / "values.tsv",
                   "capture\tphase\tsymbol/entry\toffset\tbytes\terror\tend\tlen\tentry\tframe\n"
                   "v\tp\tD_80010010\t4\taa\t\tfalse\t1\t0\t0\n"
                   "v\tp\tD_80010010\t4\tbb\t\tfalse\t1\t1\t0\n"
                   "v\tq\tD_80010010\t4\taa\t\ttrue\t1\t0\t5\n"
                   "v\tq\tD_80010010\t4\tbb\t\ttrue\t1\t1\t5\n"
                   "v\tq\tD_80010010\t4\tbb\t\ttrue\t1\t2\t5\n")
        detail = json.loads(self.get("value_changes", "D_80010010")["fact"])
        self.assertEqual([(r["entry"], r["differs"]) for r in detail["end"]],
                         [("0", False), ("1", False), ("2", None)])

    def test_phase_lists_follow_phase_table_order(self):
        for capture in ("a", "b", "v"):
            self.write(self.matrix.parent / capture / "phases.tsv",
                       "phase\tfirst_frame\tlast_frame\tscreen\n"
                       "q\t0\t4\t-\np\t5\t9\t-\nr\t10\t14\t-\n")
        self.write(self.matrix.parent / "a" / "hitmap.tsv", "function\taddr\tp\tq\tr\n")
        path = self.matrix / "coverage.tsv"
        self.write(path, path.read_text().replace("p:1+", "p:1+;q:1"))
        detail = json.loads(self.get("reach", "0x80010000")["fact"])
        self.assertEqual(detail["hit_phases"]["a"], ["q", "p"])
        detail = json.loads(self.get("phase_only", "0x80010000")["fact"])
        self.assertEqual(detail["covered_phases"]["a"], ["q", "p", "r"])
        detail = json.loads(self.get("value_constant", "D_80010010")["fact"])
        self.assertEqual(detail["values"]["aa"]["v"], ["q", "p"])
        item = dict(addr="x", cells={"a": {"z": "?", "a": "?"}})
        self.assertEqual(json.loads(facts.reach(item)[0]["fact"])["unknown"],
                         {"a": ["z", "a"]})

    def test_value_range_words_trend_and_errors(self):
        self.write(self.matrix / "values.tsv",
                   "capture\tphase\tsymbol/entry\toffset\tbytes\terror\tend\tlen\tentry\tframe\n"
                   "v\tp\tD_80010010\t\t0100000000000000\t\tfalse\t8\t0\t0\n"
                   "v\tq\tD_80010010\t\t0300000000000000\t\tfalse\t8\t0\t5\n"
                   "v\tr\tD_80010010\t\t0200000000000000\t\ttrue\t8\t0\t10\n"
                   "w\tq\tD_80010010\t\tFF000000000000FF\t\tfalse\t8\t0\t9\n"
                   "w\tp\tD_80010010\t\t02000000000000FF\t\tfalse\t8\t0\t2\n"
                   "w\tr\tD_80010010\t\t\tread failed\ttrue\t8\t0\t12\n"
                   "v\tp\tD_80010014\t\t0102\t\tfalse\t2\t1\t0\n"
                   "v\tq\tD_80010014\t\t0102\t\ttrue\t2\t1\t5\n"
                   "v\tp\tD_80010018\t\t010203\t\tfalse\t3\t2\t0\n"
                   "v\tq\tD_80010018\t\t010204\t\ttrue\t3\t2\t5\n")
        rows = [r for r in self.rows() if r["kind"] == "value_range"]
        self.assertEqual([r["addr"] for r in rows], ["D_80010010", "D_80010010", "D_80010018"])
        first, second = sorted((json.loads(r["fact"]) for r in rows[:2]),
                               key=lambda d: d["word_offset"])
        self.assertEqual((first["word_offset"], first["width"], first["distinct"],
                          first["min"], first["max"]), (0, 4, 4, "00000001", "000000FF"))
        self.assertEqual(first["trend"], {"v": "mixed", "w": "nondecreasing"})
        self.assertEqual(first["readings"], {"v": 3, "w": 2})
        self.assertEqual(first["errors"], 1)
        self.assertEqual(rows[0]["inconclusive"], "1")
        self.assertEqual((second["word_offset"], second["min"], second["max"]),
                         (4, "00000000", "FF000000"))
        self.assertEqual(second["trend"], {"v": "constant", "w": "constant"})
        odd = json.loads(rows[2]["fact"])
        self.assertEqual((odd["width"], odd["word_offset"], odd["min"], odd["max"],
                          odd["trend"]), (1, 2, "03", "04", {"v": "nondecreasing"}))

    def test_value_range_rejects_bad_lengths(self):
        item = dict(addr="x", offset="", len="2")
        readings = [dict(capture="v", phase="p", bytes="0102", error="", frame="0", entry="0"),
                    dict(capture="v", phase="q", bytes="010203", error="", frame="1", entry="0")]
        with self.assertRaises(ValueError):
            facts.value_range(item, readings)
        self.assertEqual(facts.value_range(item, readings[:1]), [])
        readings[1].update(bytes="0103", frame="")
        with self.assertRaises(ValueError):
            facts.value_range(item, readings)
        self.assertEqual(facts.word_width(16), 4)
        self.assertEqual(facts.trend([3, 2, 2]), "nonincreasing")

    def test_build_check_corruption_and_repo_relative_paths(self):
        old_cwd = Path.cwd()
        try:
            os.chdir(self.matrix.parent / "a")
            for command in ["build", "check"]:
                self.assertEqual(facts.main([command]), 0)
            path = self.matrix / "facts.tsv"
            original = path.read_bytes()
            self.assertNotIn(b"\r", original)
            self.assertEqual(facts.main(["build", "--matrix", "docs/naming/runtime/matrix",
                                        "--no-name", "docs/naming/phase3/no_name.tsv",
                                        "--out", "docs/naming/runtime/matrix/facts.tsv"]), 0)
            self.assertEqual(original, path.read_bytes())
            path.write_bytes(original.replace(b"recorded", b"Recorded", 1))
            with contextlib.redirect_stderr(io.StringIO()) as stderr:
                self.assertEqual(facts.main(["check"]), 1)
            self.assertIn("facts differ", stderr.getvalue())
        finally:
            os.chdir(old_cwd)

    def test_determinism_and_order(self):
        rows = self.rows()
        self.assertEqual([(r["kind"], r["addr"]) for r in rows],
                         sorted((r["kind"], r["addr"]) for r in rows))
        self.assertEqual(facts.render(self.matrix, self.registry),
                         facts.render(self.matrix, self.registry))


if __name__ == "__main__":
    unittest.main(verbosity=2)
