#!/usr/bin/env python3
"""Stdlib-only tests for gdbtrace set-and-observe pokes (spec "pokes")."""

from pathlib import Path
import sys
import unittest

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import gdbtrace


class Syms:
    def addr(self, text):
        return gdbtrace.Symbols.addr(self, text)

    by_name = {"D_80101EC8": 0x80101EC8}
    path = "test"


class FakeGdb:
    def __init__(self, refuse=False):
        self.ram = {}
        self.refuse = refuse

    def mem(self, addr, n):
        return bytes(self.ram.get(addr + i, 0xAA) for i in range(n))

    def write(self, addr, data):
        if self.refuse:
            return False
        for i, b in enumerate(data):
            self.ram[addr + i] = b
        return True


SCRIPT = {"phases": [{"frame": 0, "name": "boot"},
                     {"frame": 100, "name": "fight"}]}
NAMES = {"boot", "fight"}


def parse(pokes, script=SCRIPT, stubs=()):
    spec = {"pokes": pokes}
    if script:
        spec["script"] = script
    return gdbtrace.parse_pokes(spec, Syms(), NAMES, "boot", stubs)


class ParseTest(unittest.TestCase):
    def test_valid(self):
        p = parse([{"at": "D_80101EC8+0xE", "hex": "0600", "phase": "fight"}])
        self.assertEqual(p[0]["addr"], 0x80101ED6)
        self.assertEqual(p[0]["data"], b"\x06\x00")
        self.assertEqual(p[0]["when"], "fight")

    def test_refusals(self):
        bad = [
            ([{"at": "D_80101EC8", "hex": "06", "phase": "fight"}], None,
             "need a script"),
            ([{"at": "D_80101EC8", "hex": "06"}], SCRIPT, "not None"),
            ([{"at": "D_80101EC8", "hex": "06", "phase": "boot"}], SCRIPT,
             "not 'boot'"),
            ([{"at": "D_80101EC8", "hex": "06", "phase": "nope"}], SCRIPT,
             "not 'nope'"),
            ([{"at": "D_80101EC8", "hex": "", "phase": "fight"}], SCRIPT,
             "hex must be"),
            ([{"at": "D_80101EC8", "hex": "0g", "phase": "fight"}], SCRIPT,
             "hex must be"),
            ([{"at": "D_80101EC8", "hex": 6, "phase": "fight"}], SCRIPT,
             "hex must be"),
            ([{"at": "0x801FFFFF", "hex": "0000", "phase": "fight"}], SCRIPT,
             "not RAM"),
            ([{"at": "0x1F8003FF", "hex": "0000", "phase": "fight"}], SCRIPT,
             "not RAM"),
            ([{"at": "D_80101EC8", "hex": "0600", "phase": "fight"},
              {"at": "D_80101EC8+0x1", "hex": "00", "phase": "fight"}], SCRIPT,
             "overlaps poke D_80101EC8"),
        ]
        for pokes, script, msg in bad:
            with self.subTest(pokes=pokes), \
                    self.assertRaisesRegex(SystemExit, msg):
                parse(pokes, script)

    def test_stub_overlap_refused(self):
        stub = [{"spec": {"at": "func_80101EC0"}, "addr": 0x80101EC0}]
        with self.assertRaisesRegex(SystemExit, "overlaps stub func_80101EC0"):
            parse([{"at": "0x80101EC7", "hex": "00", "phase": "fight"}],
                  stubs=stub)
        self.assertEqual(len(parse(
            [{"at": "D_80101EC8", "hex": "00", "phase": "fight"}],
            stubs=stub)), 1)  # first byte after the stub's two words

    def test_adjacent_and_scratchpad_ok(self):
        p = parse([{"at": "D_80101EC8", "hex": "0600", "phase": "fight"},
                   {"at": "D_80101EC8+0x2", "hex": "00", "phase": "fight"},
                   {"at": "0x1F8003FE", "hex": "0102", "phase": "fight"}])
        self.assertEqual(len(p), 3)


class ApplyTest(unittest.TestCase):
    def test_applies_once_at_its_phase(self):
        p = parse([{"at": "D_80101EC8+0xE", "hex": "0600", "phase": "fight"}])
        gdb, out = FakeGdb(), []
        regs = [0] * 37 + [0x80012345]
        gdbtrace.apply_pokes(gdb, p, "boot", regs, 0, out.append)
        self.assertEqual(out, [])
        gdbtrace.apply_pokes(gdb, p, "fight", regs, 100, out.append)
        self.assertEqual(gdb.mem(0x80101ED6, 2), b"\x06\x00")
        self.assertEqual(out[0]["ev"], "poke")
        self.assertEqual(out[0]["orig"], "aaaa")
        self.assertEqual(out[0]["frame"], 100)
        self.assertTrue(out[0]["ok"])
        gdb.ram.clear()
        gdbtrace.apply_pokes(gdb, p, "fight", regs, 200, out.append)
        self.assertEqual(len(out), 1)  # once only

    def test_read_back_mismatch_stops_the_run(self):
        class Liar(FakeGdb):
            def mem(self, addr, n):
                return b"\x00" * n

        p = parse([{"at": "D_80101EC8", "hex": "06", "phase": "fight"}])
        out = []
        with self.assertRaisesRegex(SystemExit, "poke write at 0x80101EC8"):
            gdbtrace.apply_pokes(Liar(), p, "fight", [0] * 38, 100,
                                 out.append)
        self.assertFalse(out[0]["ok"])

    def test_refused_write_stops_the_run(self):
        p = parse([{"at": "D_80101EC8", "hex": "06", "phase": "fight"}])
        out = []
        with self.assertRaises(SystemExit):
            gdbtrace.apply_pokes(FakeGdb(refuse=True), p, "fight", [0] * 38,
                                 100, out.append)
        self.assertFalse(out[0]["ok"])
        self.assertFalse(p[0]["applied"]["ok"])


class CompareTextTest(unittest.TestCase):
    """compare names the second run by what it changed."""

    def run_compare(self, root, stubs, pokes):
        import argparse
        import json
        caps = []
        for name, spec, result in (
                ("baseline", {"script": {"screenshots": []}}, {}),
                ("other", dict({"script": {"screenshots": []}},
                               **({"stubs": [{"at": "f"}]} if stubs else {}),
                               **({"pokes": [{"at": "D_80101EC8"}]} if pokes
                                  else {})),
                 {"pokes": [{"at": "D_80101EC8", "addr": 0x80101EC8,
                             "hex": "06", "orig": "02", "when": "fight",
                             "applied": {"frame": 9}, "at_end": "06"}]
                  if pokes else []})):
            d = root / name
            d.mkdir()
            (d / "spec.json").write_text(json.dumps(spec), encoding="utf-8")
            (d / "capture.jsonl").write_text("", encoding="utf-8")
            manifest = {"q124_complete": True, "result": result, "outputs": [],
                        "spec": {"path": str(d / "spec.json"),
                                 "sha1": gdbtrace.runkit.sha1_file(
                                     str(d / "spec.json"))}}
            (d / "capture.manifest.json").write_text(json.dumps(manifest),
                                                    encoding="utf-8")
            caps.append(str(d / "capture.jsonl"))
        out = root / "other" / "compare.md"
        code = gdbtrace.compare(argparse.Namespace(
            base=caps[0], other=caps[1], base_spec=None, other_spec=None,
            context=None, out=str(out)))
        return code, out.read_text(encoding="utf-8")

    def test_titles_labels_and_notes(self):
        import tempfile
        cases = {(False, True): ("Set-and-observe", "- poked:"),
                 (True, False): ("Stub-and-observe", "- stubbed:"),
                 (True, True): ("Stub- and set-and-observe",
                                "- stubbed and poked:")}
        for (stubs, pokes), (title, label) in cases.items():
            with self.subTest(stubs=stubs, pokes=pokes), \
                    tempfile.TemporaryDirectory() as tmp:
                got, text = self.run_compare(Path(tmp), stubs, pokes)
                self.assertEqual(got, 0)
                self.assertTrue(text.startswith(f"# {title}: other against "
                                                "baseline\n"))
                self.assertIn(label, text)
                self.assertEqual("A stub counts only" in text, stubs)
                self.assertEqual("A poke counts only" in text, pokes)


if __name__ == "__main__":
    unittest.main()
