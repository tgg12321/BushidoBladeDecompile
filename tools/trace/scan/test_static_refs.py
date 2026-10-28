#!/usr/bin/env python3
"""Tests for static_refs.py: a synthetic EXE and two hit maps."""

from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import static_refs  # noqa: E402

LOAD = 0x80010000


def exe(words, load=LOAD):
    head = bytearray(static_refs.HEADER_SIZE)
    struct.pack_into("<I", head, 0x18, load)
    return bytes(head) + struct.pack("<%dI" % len(words), *words)


def lui(rt, imm):
    return (0x0F << 26) | (rt << 16) | imm


def addiu(rt, rs, imm):
    return (0x09 << 26) | (rs << 21) | (rt << 16) | imm


def ori(rt, rs, imm):
    return (0x0D << 26) | (rs << 21) | (rt << 16) | imm


class References(unittest.TestCase):
    def test_kinds(self):
        f = 0x80010040
        words = [0x0C000000 | (f >> 2) & 0x3FFFFFF,   # jal f
                 0x08000000 | (f >> 2) & 0x3FFFFFF,   # j f
                 f,                                   # pointer word
                 lui(4, 0x8001), 0, addiu(4, 4, 0x0040),
                 lui(5, 0x8001), addiu(6, 6, 0x0040),  # other register
                 0]
        counts, sites = static_refs.references(
            [static_refs.load_exe(exe(words))], f)
        self.assertEqual(counts, {"jal": 1, "j": 1, "word": 1, "hilo": 1})
        self.assertEqual(sites, ["jal@80010000", "j@80010004",
                                 "word@80010008", "hilo@8001000C"])

    def test_hi_carry(self):
        f = 0x8001A000  # %lo >= 0x8000: %hi rounds up
        words = [lui(2, 0x8002), addiu(2, 2, 0xA000)]
        counts, _ = static_refs.references(
            [static_refs.load_exe(exe(words))], f)
        self.assertEqual(counts["hilo"], 1)

    def test_ori_no_carry(self):
        f = 0x8001A000  # ori builds (upper << 16) | lo: no carry
        good = [lui(2, 0x8001), ori(2, 2, 0xA000)]
        bad = [lui(2, 0x8002), ori(2, 2, 0xA000)]  # 0x8002A000
        for words, n in ((good, 1), (bad, 0)):
            counts, _ = static_refs.references(
                [static_refs.load_exe(exe(words))], f)
            self.assertEqual(counts["hilo"], n)

    def test_pair_window(self):
        f = 0x80010040
        far = [lui(4, 0x8001)] + [0] * static_refs.PAIR_WINDOW + \
            [addiu(4, 4, 0x0040)]
        counts, _ = static_refs.references(
            [static_refs.load_exe(exe(far))], f)
        self.assertEqual(counts["hilo"], 0)

    def test_overlay_load_address(self):
        f = 0x80010040
        image = static_refs.load_exe(exe([0, f], load=0x801D8800))
        self.assertEqual(image[0], 0x801D8800)
        _, sites = static_refs.references([image], f)
        self.assertEqual(sites, ["word@801D8804"])


class Rows(unittest.TestCase):
    def test_unreached_only_and_names(self):
        with tempfile.TemporaryDirectory() as d:
            a = Path(d) / "a.tsv"
            b = Path(d) / "b.tsv"
            a.write_text("function\taddr\tp0\n"
                         "func_80010040\t0x80010040\t.\n"
                         "func_80010080\t0x80010080\t1+\n", encoding="utf-8")
            b.write_text("function\taddr\tp0\tp1\n"
                         "func_80010040\t0x80010040\t.\t.\n"
                         "func_800100C0\t0x800100C0\t.\t2\n",
                         encoding="utf-8")
            sym = Path(d) / "symbol_addrs.txt"
            sym.write_text("old_name = 0x80010040;\n"
                           "new_name = 0x80010040; // later wins\n",
                           encoding="utf-8")
            targets, reached = static_refs.read_hitmaps([a, b])
            names = static_refs.read_symbol_names(sym)
        self.assertEqual(reached, {0x80010080, 0x800100C0})
        images = [static_refs.load_exe(exe([0x80010040]))]
        got = list(static_refs.rows(targets, reached, names, images))
        self.assertEqual(got, [["new_name", "0x80010040", 0, 0, 1, 0,
                                "word@80010000"]])
        every = list(static_refs.rows(targets, reached, names, images, True))
        self.assertEqual([r[1] for r in every],
                         ["0x80010040", "0x80010080", "0x800100C0"])


if __name__ == "__main__":
    unittest.main()
