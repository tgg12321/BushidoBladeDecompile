#!/usr/bin/env python3
"""Unit tests for tools/nrefs_census.py (pure logic). Run: python tools/test_nrefs_census.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.nrefs_census import (priority, reorder, min_lift_to_beat, first_def_insn,
                                mode_size, banner_segment, format_census, parse_lift)

ROWS = [
    {"ord": 0, "pseudo": 76, "hardreg": 2, "nrefs": 40, "livelen": 30, "pri": 0},
    {"ord": 1, "pseudo": 73, "hardreg": 16, "nrefs": 24, "livelen": 60, "pri": 0},
]
RTL = """(insn 5 4 6 (set (reg:SI 73) (mem:SI (reg:SI 70))) -1 (nil) (nil))
(insn 9 8 10 (set (reg:SI 76) (plus:SI (reg:SI 73) (const_int 4))) -1 (nil) (nil))
"""

# A REAL .greg/.lreg insn block: multi-line, ends with `<code> {template} (nil)`
# plus a REG_* note list, and blocks are separated by blank lines.
REAL_RTL = """;; Function sys_VSync

(note 3 1 5 "" NOTE_INSN_DELETED)

(insn 10 8 12 (set (reg:SI 73)
        (const:SI (plus:SI (symbol_ref:SI ("D_800A14D0"))
                (const_int 48)))) 159 {movsi_internal2} (nil)
    (expr_list:REG_EQUIV (const:SI (plus:SI (symbol_ref:SI ("D_800A14D0"))
                (const_int 48)))
        (nil)))

(insn 12 10 17 (set (reg/v:SI 76)
        (mem/s/v:SI (reg:SI 73))) 159 {movsi_internal2} (insn_list 10 (nil))
    (nil))

(call_insn 20 17 22 (parallel[
            (set (reg:SI 90)
                (call (mem:SI (symbol_ref:SI ("foo"))) (const_int 16)))
        ] ) 214 {call_value_internal0} (nil)
    (nil))
"""

MODE_RTL = ("(insn 1 0 2 (set (reg:DI 76) (const_int 0)) -1 (nil) (nil))\n"
            "(insn 2 1 3 (set (reg/v:SI 73) (reg:SI 4 a0)) -1 (nil) (nil))\n"
            "(insn 3 2 4 (set (reg:TI 80) (const_int 0)) -1 (nil) (nil))\n"
            "(insn 4 3 5 (set (reg/v/f:DF 81) (const_int 0)) -1 (nil) (nil))\n")


class TestCensus(unittest.TestCase):
    def test_priority_formula_matches_global_c(self):
        # global.c:615 — floor(log2(nrefs)) * nrefs / livelen * 10000 * size
        self.assertEqual(priority(40, 30, 1), int((5 * 40 / 30) * 10000 * 1))
        self.assertEqual(priority(24, 60, 1), int((4 * 24 / 60) * 10000 * 1))

    def test_priority_size_scales(self):
        self.assertEqual(priority(40, 30, 2), int((5 * 40 / 30) * 10000 * 2))
        # truncation happens AFTER the *size multiply, so size=2 is not exactly 2x
        self.assertEqual(priority(40, 30, 2), 133333)
        self.assertEqual(priority(40, 30, 1), 66666)  # 2x66666 = 133332 < 133333

    def test_priority_zero_livelen_is_large_negative(self):
        # global.c:569-572 rewrites live length 0 -> -1 (a deliberately low priority)
        self.assertEqual(priority(40, 0, 1), int((5 * 40 / -1) * 10000))
        self.assertLess(priority(40, 0, 1), 0)
        self.assertEqual(priority(0, 10, 1), 0)   # nrefs<=0 short-circuits

    def test_reorder_by_priority(self):
        order = [r["pseudo"] for r in reorder(ROWS)]
        self.assertEqual(order, [76, 73])

    def test_reorder_ties_by_ascending_allocno(self):
        # global.c:651-655 — equal priority resolves by ascending allocno number
        tied = [{"pseudo": 90, "nrefs": 8, "livelen": 4, "pri": 0},
                {"pseudo": 73, "nrefs": 8, "livelen": 4, "pri": 0}]
        self.assertEqual(priority(8, 4), priority(8, 4))
        self.assertEqual([r["pseudo"] for r in reorder(tied)], [73, 90])

    def test_min_lift(self):
        k = min_lift_to_beat(ROWS, 73, 76)
        self.assertGreater(k, 0)
        lifted = [dict(r, nrefs=r["nrefs"] + (k if r["pseudo"] == 73 else 0)) for r in ROWS]
        self.assertEqual([r["pseudo"] for r in reorder(lifted)][0], 73)
        lifted2 = [dict(r, nrefs=r["nrefs"] + (k - 1 if r["pseudo"] == 73 else 0)) for r in ROWS]
        self.assertEqual([r["pseudo"] for r in reorder(lifted2)][0], 76)

    def test_min_lift_unknown_pseudo_errors(self):
        with self.assertRaises(SystemExit) as cm:
            min_lift_to_beat(ROWS, 999, 76)
        self.assertIn("999", str(cm.exception))
        self.assertIn("[73, 76]", str(cm.exception))

    def test_first_def_insn(self):
        self.assertIn("(mem:SI (reg:SI 70))", first_def_insn(RTL, 73))
        self.assertIn("plus:SI", first_def_insn(RTL, 76))
        self.assertEqual(first_def_insn(RTL, 99), "")

    def test_first_def_insn_real_multiline_block(self):
        d73 = first_def_insn(REAL_RTL, 73)
        self.assertIn("(insn 10 8 12", d73)
        self.assertIn("symbol_ref:SI (\"D_800A14D0\")", d73)
        self.assertNotIn("\n", d73)              # collapsed to one line
        # the block must not bleed into the NEXT insn
        self.assertNotIn("(insn 12 10 17", d73)
        # reg with flags (reg/v:SI 76) is still a definition of pseudo 76
        self.assertIn("mem/s/v:SI", first_def_insn(REAL_RTL, 76))
        # call_insn destinations count too
        self.assertIn("call_insn 20", first_def_insn(REAL_RTL, 90))
        self.assertEqual(first_def_insn(REAL_RTL, 999), "")

    def test_first_def_insn_ignores_uses(self):
        # pseudo 70 only ever appears as a SOURCE in RTL -> no def
        self.assertEqual(first_def_insn(RTL, 70), "")

    def test_mode_size(self):
        # PSEUDO_REGNO_SIZE = ceil(mode bytes / 4); plain (reg:DI N) must match
        self.assertEqual(mode_size(MODE_RTL, 76), 2)
        self.assertEqual(mode_size(MODE_RTL, 73), 1)
        self.assertEqual(mode_size(MODE_RTL, 80), 4)
        self.assertEqual(mode_size(MODE_RTL, 81), 2)   # flagged (reg/v/f:DF 81)
        self.assertEqual(mode_size(MODE_RTL, 999), 1)  # absent -> default 1

    def test_banner_segment(self):
        dump = ";; Function a\nline a\n\n;; Function b\nline b\n"
        self.assertIn("line a", banner_segment(dump, "a"))
        self.assertNotIn("line b", banner_segment(dump, "a"))
        self.assertIn("line b", banner_segment(dump, "b"))
        self.assertEqual(banner_segment(dump, "c"), "")

    def test_format_census_flags_pri_mismatch(self):
        rows = [{"ord": 0, "pseudo": 76, "hardreg": 2, "nrefs": 40, "livelen": 30,
                 "pri": priority(40, 30, 1)},
                {"ord": 1, "pseudo": 73, "hardreg": 16, "nrefs": 24, "livelen": 60,
                 "pri": 999999}]   # hook disagrees -> must be flagged
        lines, bad = format_census(rows)
        self.assertEqual(bad, [73])
        self.assertTrue(any("MISMATCH" in ln for ln in lines))
        self.assertEqual(sum("MISMATCH" in ln for ln in lines), 1)

    def test_parse_lift(self):
        self.assertEqual(parse_lift("73=+72"), (73, 72))
        self.assertEqual(parse_lift(" 73 = 72 "), (73, 72))
        with self.assertRaises(SystemExit):
            parse_lift("73+72")


if __name__ == "__main__":
    unittest.main()
