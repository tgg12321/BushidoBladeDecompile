#!/usr/bin/env python3
"""Unit tests for tools/nrefs_census.py (pure logic). Run: python tools/test_nrefs_census.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.nrefs_census import priority, reorder, min_lift_to_beat, first_def_insn

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


class TestCensus(unittest.TestCase):
    def test_priority_formula_matches_global_c(self):
        # global.c:615 — floor(log2(nrefs)) * nrefs / livelen * 10000 * size
        self.assertEqual(priority(40, 30, 1), int((5 * 40 / 30) * 10000 * 1))
        self.assertEqual(priority(24, 60, 1), int((4 * 24 / 60) * 10000 * 1))

    def test_reorder_by_priority(self):
        order = [r["pseudo"] for r in reorder(ROWS)]
        self.assertEqual(order, [76, 73])

    def test_min_lift(self):
        k = min_lift_to_beat(ROWS, 73, 76)
        self.assertGreater(k, 0)
        lifted = [dict(r, nrefs=r["nrefs"] + (k if r["pseudo"] == 73 else 0)) for r in ROWS]
        self.assertEqual([r["pseudo"] for r in reorder(lifted)][0], 73)
        lifted2 = [dict(r, nrefs=r["nrefs"] + (k - 1 if r["pseudo"] == 73 else 0)) for r in ROWS]
        self.assertEqual([r["pseudo"] for r in reorder(lifted2)][0], 76)

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


if __name__ == "__main__":
    unittest.main()
