#!/usr/bin/env python3
"""Unit tests for tools/fake_ablate.py (pure text logic; no build). Run: python tools/test_fake_ablate.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.fake_ablate import find_fake_units, remove_units, ablation_masks

BODY = """s32 f(s32 a)
{
    s32 three = 3; /* FAKE: constant holder, mechanism: global.c, lever-exhaustion: s4 */
    s32 r;
    do { /* FAKE: wrap, mechanism: reorg.c LABEL_OUTSIDE_LOOP_P, lever-exhaustion: s6 */
        r = a * three;
    } while (0);
    r = r; /* FAKE: dead store, mechanism: flow.c, lever-exhaustion: s7 */
    return r;
}
"""


class TestUnits(unittest.TestCase):
    def test_finds_three_units_with_kinds(self):
        units = find_fake_units(BODY)
        self.assertEqual([u.kind for u in units], ["line", "wrap", "line"])
        self.assertEqual(units[0].lines, [2])
        self.assertEqual(units[1].lines, [4, 6])      # `do {` and `} while (0);`
        self.assertEqual(units[2].lines, [7])

    def test_remove_wrap_keeps_body(self):
        units = find_fake_units(BODY)
        out = remove_units(BODY, [units[1]])
        self.assertNotIn("do {", out)
        self.assertNotIn("while (0)", out)
        self.assertIn("r = a * three;", out)

    def test_remove_line(self):
        units = find_fake_units(BODY)
        out = remove_units(BODY, [units[2]])
        self.assertNotIn("r = r;", out)
        self.assertIn("return r;", out)

    def test_masks_full_grid_small(self):
        self.assertEqual(len(ablation_masks(3)), 8)
        self.assertIn((True, True, True), ablation_masks(3))

    def test_masks_capped(self):
        m = ablation_masks(6)
        self.assertIn(tuple([True] * 6), m)               # all removed
        self.assertEqual(sum(1 for x in m if sum(x) == 1), 6)   # each singleton
        self.assertLessEqual(len(m), 1 + 6 + 15 + 1)       # none, singles, pairs, all


if __name__ == "__main__":
    unittest.main()
