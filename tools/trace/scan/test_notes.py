#!/usr/bin/env python3
"""Synthetic evidence notes and TU boundaries; no committed trace inputs."""

import contextlib
import csv
import io
import json
from pathlib import Path
import shutil
import sys
import unittest
from unittest.mock import patch
import uuid

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[3]))
from tools.trace.scan import notes


class NotesTest(unittest.TestCase):
    def setUp(self):
        scratch = notes.REPO_ROOT / "tmp"
        scratch.mkdir(exist_ok=True)
        self.root = scratch / ("notes-test-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(self.cleanup)
        root_patch = patch.object(notes, "REPO_ROOT", self.root)
        root_patch.start()
        self.addCleanup(root_patch.stop)
        self.write("bb2.ld", "\n".join(
            f"build/src/{tu}.o(.text);" for tu in
            ["main/800", "main/named", "main/900", "main/unresolved"]))
        self.write("src/main/unresolved.c", "/* no address */\n")
        self.write("src/main/named.c",
                   "/* 0x80000000 before .text 0x80010020..0x80010040 */\n"
                   "int x; /* .text 0x80010004 */\n")
        self.write("docs/naming/runtime/matrix/sources.tsv",
                   "capture\tinput_file\tinput_sha1\n" + "".join(
                       f"{c}\tcapture.jsonl\t{c * 40}\n"
                       f"{c}\tphases.tsv\t{'0' * 40}\n" for c in "abcd"))
        self.facts = []

    def cleanup(self):
        expected = Path(__file__).resolve().parents[3] / "tmp"
        if self.root.resolve().parent != expected or not self.root.name.startswith("notes-test-"):
            raise RuntimeError("fixture cleanup escaped workspace scratch directory")
        shutil.rmtree(self.root)

    def write(self, name, content):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content.encode("utf-8"))

    def add(self, addr, kind, fact, captures="a;b", inconclusive=False):
        self.facts.append(dict(kind=kind, addr=f"0x{addr:08X}", capture_name="recorded",
                               no_name="1", fact=json.dumps(fact), captures=captures,
                               inconclusive=str(int(inconclusive))))

    def save(self):
        stream = io.StringIO(newline="\n")
        writer = csv.DictWriter(stream, ["kind", "addr", "capture_name", "no_name",
                                       "fact", "captures", "inconclusive"],
                                delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(self.facts)
        self.write("docs/naming/runtime/matrix/facts.tsv", stream.getvalue())

    def rows(self, tu="main/800"):
        self.save()
        return list(csv.DictReader(io.StringIO(notes.render(
            tu, self.root / "docs/naming/runtime/matrix/facts.tsv",
            self.root / "docs/naming/runtime/matrix/sources.tsv",
            self.root / "bb2.ld", self.root / "src")), delimiter="\t"))

    def reach(self, addr, hit, targeted=None, unknown=None):
        self.add(addr, "reach", dict(targeted=targeted or ["a", "b"], hit=hit,
                 unknown=unknown or {}, hit_phases={c: ["z", "p"] for c in hit}),
                 captures=";".join(targeted or ["a", "b"]))

    def test_statuses_sorting_and_data_ignored(self):
        self.add(0x8001000C, "phase_only", dict(hit_phases={}, covered_phases={}))
        self.reach(0x80010008, ["a"])
        self.reach(0x80010004, ["a", "b"])
        self.reach(0x80010000, [])
        self.add(0x80010000, "never_hit", dict(targeted=["a", "b"]))
        self.add(0x80010010, "value_constant", {})
        rows = self.rows()
        self.assertEqual([r["status"] for r in rows],
                         ["never_hit", "hit_all", "capture_only", "untargeted+phase_only"])
        self.assertEqual(rows[0]["captures"], f"a@{'a' * 12};b@{'b' * 12}")
        self.assertEqual((rows[0]["capture_name"], rows[0]["no_name"]), ("recorded", "1"))

    def test_untargeted_and_phase_order(self):
        self.add(0x80010000, "never_hit", dict(targeted=[]), captures="")
        self.reach(0x80010004, ["a"])
        self.add(0x80010004, "phase_only", dict(hit_phases={"a": ["z", "p"], "b": []},
                 covered_phases={"a": ["z", "p", "q"], "b": ["z", "p", "q"]}))
        rows = self.rows()
        self.assertEqual(rows[0]["status"], "untargeted")
        self.assertEqual(rows[1]["status"], "capture_only+phase_only")
        self.assertIn("phases: a z,p of 3 covered; b (none) of 3 covered", rows[1]["summary"])

    def test_smaller_side_and_tie(self):
        for addr, hit, quiet in [(0x80010000, ["a"], ["b", "c"]),
                                  (0x80010004, ["a", "b"], ["c"]),
                                  (0x80010008, ["a"], ["b"])]:
            self.reach(addr, hit, hit + quiet)
            self.add(addr, "capture_only", dict(hit=hit, armed_never_hit=quiet, unknown=[]),
                     captures=";".join(hit + quiet))
        summaries = [r["summary"] for r in self.rows()]
        self.assertEqual(summaries, [
            "hit in 1/3 captures (a); armed, never hit in 2",
            "hit in 2/3 captures; armed, never hit in 1 (c)",
            "hit in 1/2 captures (a); armed, never hit in 1"])

    def test_unknown_cells_and_reasons(self):
        self.add(0x80010000, "reach", dict(targeted=["a", "b"], hit=["a"],
                 unknown={"a": ["z", "p"], "b": ["p"]}), inconclusive=True)
        self.facts[-1]["fact"] = json.dumps(dict(targeted=["a", "b"], hit=["a"],
            unknown={"a": ["z", "p"], "b": ["p"]}, reason="unknown_cells"))
        self.add(0x80010000, "capture_only", dict(hit=["a"], armed_never_hit=[],
                 unknown=["b"], reason="unknown_captures"), inconclusive=True)
        row = self.rows()[0]
        self.assertEqual(row["inconclusive"], "1")
        self.assertIn("unknown (?) cells: a z,p; b p", row["summary"])
        self.assertIn("unknown (?) captures: b", row["summary"])
        self.assertIn("inconclusive: unknown_captures, unknown_cells", row["summary"])
        self.assertNotIn("armed, never hit", row["summary"])

    def test_unknown_status_needs_quiet_captures(self):
        self.reach(0x80010000, [], unknown={"a": ["p"], "b": ["p"]})
        self.reach(0x80010004, [], unknown={"b": ["p"]})
        self.reach(0x80010008, ["a"], unknown={"b": ["p"]})
        self.add(0x80010008, "capture_only", dict(hit=["a"], armed_never_hit=[],
                 unknown=["b"]))
        self.reach(0x8001000C, [], targeted=[])
        self.facts[-1]["fact"] = json.dumps(dict(targeted=[], hit=[], unknown={},
                                                 hit_phases={}))
        self.assertEqual([r["status"] for r in self.rows()],
                         ["unknown", "unknown", "unknown", "untargeted"])

    def test_ranges_and_unresolvable_neighbour(self):
        self.assertEqual(notes.tu_range("main/800", self.root / "bb2.ld", self.root / "src"),
                         (0x80010000, 0x80010020))
        self.assertEqual(notes.tu_range("main/named", self.root / "bb2.ld", self.root / "src"),
                         (0x80010020, 0x80010100))
        with self.assertRaisesRegex(ValueError, "unresolvable TU after main/900"):
            notes.tu_range("main/900", self.root / "bb2.ld", self.root / "src")
        with self.assertRaisesRegex(ValueError, "unresolvable"):
            notes.tu_range("main/unresolved", self.root / "bb2.ld", self.root / "src")
        self.write("bb2.ld", "build/src/main/900.o(.text);\n")
        self.assertEqual(notes.tu_range("main/900", self.root / "bb2.ld", self.root / "src"),
                         (0x80010100, None))
        self.write("bb2.ld", "\n".join(
            f"build/src/{tu}.o(.text);" for tu in ["main/800", "main/named", "main/900"]))
        for addr in (0x8000FFFF, 0x80010000, 0x8001001C, 0x80010020, 0x80010100):
            self.reach(addr, ["a"])
        self.assertEqual([r["addr"] for r in self.rows()], ["0x80010000", "0x8001001C"])
        self.assertEqual([r["addr"] for r in self.rows("main/named")], ["0x80010020"])

    def test_sdk_module_name_that_looks_like_hex(self):
        self.write("src/main/psxsdk/libapi/c67.c",
                   "/* SDK module. .text 0x80078948..0x80078970 */\n")
        self.assertEqual(notes.tu_start("main/psxsdk/libapi/c67", self.root / "src"),
                         0x80078948)

    def test_missing_source(self):
        self.add(0x80010000, "never_hit", dict(targeted=["missing"]), captures="missing")
        with self.assertRaisesRegex(ValueError, "missing capture.jsonl source"):
            self.rows()

    def test_build_check_options_and_byte_edit(self):
        self.reach(0x80010000, ["a"])
        self.save()
        args = ["--facts", "docs/naming/runtime/matrix/facts.tsv", "--sources",
                "docs/naming/runtime/matrix/sources.tsv", "--out-dir", "output",
                "--ld", "bb2.ld", "--src", "src"]
        self.assertEqual(notes.main(["build", "--tu", "main/800", "--tu", "main/named"] + args), 0)
        self.write("output/events.tsv", "shared event evidence\n")
        self.write("output/other.tsv", "non-TU table\n")
        self.assertEqual(notes.main(["check"] + args), 0)
        path = self.root / "output/main__800.tsv"
        original = path.read_bytes()
        self.assertNotIn(b"\r", original)
        self.assertEqual(notes.main(["build", "--tu", "main/800"] + args), 0)
        self.assertEqual(path.read_bytes(), original)
        path.write_bytes(original.replace(b"recorded", b"Recorded", 1))
        with contextlib.redirect_stderr(io.StringIO()) as stderr:
            self.assertEqual(notes.main(["check"] + args), 1)
        self.assertIn("notes differ", stderr.getvalue())


if __name__ == "__main__":
    unittest.main(verbosity=2)
