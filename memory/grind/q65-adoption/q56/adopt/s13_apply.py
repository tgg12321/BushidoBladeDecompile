#!/usr/bin/env python3
"""Step 13 (Q65; needs the rule amendment A3, PLAN.md): maspsx models cc1psx -G8's section choice for small
initialized objects. cc1psx -G8 emits an initialized object of <= 8 bytes in `.sdata`; our cc1 emits it in
`.data` even at -G8 (calibration tmp/q56/cc1psx_decl_probe.sh: `int z1 = 0; int w1 = 5;`). Under -G<n>
maspsx moves each such `.data` object (its `.align`, label and data directives) into `.sdata`, where it is the
file's own small data (gp at every offset). Global, names no symbol; inert without -G (byte-neutral today).
usage: s13_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
P = "tools/maspsx/maspsx/__init__.py"
sub1(P, """        self.bss_entries = {}
        self.sbss_entries = {}
        self.sdata_entries = {}

        self.preprocess_lines()""",
"""        self.bss_entries = {}
        self.sbss_entries = {}
        self.sdata_entries = {}

        if self.sdata_limit > 0:
            self.lines = self._small_data_to_sdata(self.lines)
        self.preprocess_lines()""")
sub1(P, """    def process_lines(self):""",
'''    _DATA_DIRECTIVES = (".word", ".half", ".short", ".byte", ".space", ".ascii", ".asciz")

    def _data_size(self, line: str) -> int:
        if line.startswith(".space"):
            return int(line.split()[1], 0)
        if line.startswith(".word"):
            return 4 * (line.count(",") + 1)
        if line.startswith(".half") or line.startswith(".short"):
            return 2 * (line.count(",") + 1)
        if line.startswith(".byte"):
            return line.count(",") + 1
        if line.startswith(".asciz"):
            return len(line) - 10 + 1
        if line.startswith(".ascii"):
            return len(line) - 9
        raise Exception(f"Unable to size data directive: {line}")

    def _small_data_to_sdata(self, lines):
        """cc1psx -G<n> emits an initialized object of <= n bytes in `.sdata`; our cc1
        emits it in `.data` (owner ruling Q65). Move each such `.data` object (its
        `.align`, label and data directives) into `.sdata`, then return to `.data`."""
        out, sec, i, n = [], None, 0, len(lines)
        while i < n:
            line = lines[i]
            s = line.strip()
            if s.startswith(".data"):
                sec = "data"
            elif s.startswith((".sdata", ".rdata", ".text", ".section", ".bss", ".sbss")):
                sec = "other"
            j = i
            if sec == "data" and s.startswith(".align") and i + 1 < n and re.match(r"^[A-Za-z_][\\w.$]*:$", lines[i + 1].strip()):
                j = i + 1
            label = lines[j].strip()
            if sec == "data" and re.match(r"^[A-Za-z_][\\w.$]*:$", label) and not label.startswith(("$", ".")):
                k, size = j + 1, 0
                while k < n and lines[k].strip().startswith(self._DATA_DIRECTIVES):
                    size += self._data_size(lines[k].strip())
                    k += 1
                if 0 < size <= self.sdata_limit:
                    out += [".sdata"] + lines[i:k] + [".data"]
                else:
                    out += lines[i:k]
                i = k
                continue
            out.append(line)
            i += 1
        return out

    def process_lines(self):''')
wr("tools/maspsx/tests/test_small_data_sdata.py", '''"""cc1psx -G8 emits an initialized object of <= 8 bytes in `.sdata`; our cc1 emits `.data`. Under -G8 maspsx
moves it to `.sdata`, where it is the file's own small data (gp); larger objects stay in `.data`; without -G
nothing moves (owner ruling Q65)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments

SRC = [".globl\\tz1", ".data", ".align\\t2", "z1:", ".word\\t0", ".globl\\tbig", ".align\\t2", "big:",
       ".word\\t1", ".word\\t2", ".word\\t3", ".text", ".ent\\tf", "lw\\t$4,z1", "lw\\t$5,big", ".end\\tf"]


def run(g):
    return [l for l in strip_comments(MaspsxProcessor(SRC, sdata_limit=g).process_lines()) if l.strip()]


class TestSmallDataSdata(unittest.TestCase):
    def test_small_initialized_object_is_sdata_and_gp(self):
        res = run(8)
        self.assertIn("lw\\t$4,%gp_rel(z1)($gp)", res)
        self.assertNotIn("lw\\t$5,%gp_rel(big)($gp)", res)
        self.assertLess(res.index(".section .sdata"), res.index("z1:"))

    def test_nothing_moves_without_g(self):
        res = run(0)
        self.assertNotIn(".section .sdata", res)
        self.assertNotIn("lw\\t$4,%gp_rel(z1)($gp)", res)


if __name__ == "__main__":
    unittest.main()
''')
sub1("engine/test_engine.py", """def test_maspsx_fingerprint() -> None:""", '''def test_maspsx_small_data_sdata() -> None:
    """Owner ruling Q65 (amendment A3): under -G8 a <= 8-byte initialized object our cc1 put in `.data` moves
    to `.sdata` (cc1psx -G8's choice) and is gp; without -G nothing moves (byte-neutral today)."""
    import sys
    sys.path.insert(0, str(Path("tools/maspsx").resolve()))
    try:
        from maspsx import MaspsxProcessor
    finally:
        sys.path.pop(0)
    sd = [".globl\\tz1", ".data", ".align\\t2", "z1:", ".word\\t0", ".text", ".ent\\tf", "lw\\t$4,z1", ".end\\tf"]

    def run(g):
        return [l.split("#")[0].strip() for l in MaspsxProcessor(sd, sdata_limit=g).process_lines() if l.split("#")[0].strip()]
    check("maspsx -G8: a 4-byte initialized object moves to .sdata and is gp",
          ".section .sdata" in run(8) and "lw\\t$4,%gp_rel(z1)($gp)" in run(8))
    check("maspsx without -G: no .sdata move, no gp",
          ".section .sdata" not in run(0) and "lw\\t$4,%gp_rel(z1)($gp)" not in run(0))


def test_maspsx_fingerprint() -> None:''')
sub1("engine/test_engine.py", "    test_maspsx_fingerprint()\n", "    test_maspsx_small_data_sdata()\n    test_maspsx_fingerprint()\n")
print("step 13 applied")
