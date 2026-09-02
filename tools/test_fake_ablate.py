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


BLOCK = """s32 g(s32 a)
{
    s32 r;
    /* FAKE: single-level wrap; loop notes fence sched1 so the final
     * sll stays ahead of the lw. */
    do {
        r = a;
    } while (0);
    /* FAKE: constant holder */
    s32 three = 3;
    return r * three;
}
"""

ONELINE = """void h(void)
{
    do { g(); } while (0); /* FAKE: wrap */
    do { do { *p = 0; } while (0); } while (0); /* FAKE: NESTED wrap */
    x = 1; // FAKE: dead store
    do {
        y = 2;
    } while (0); /* FAKE: marker on the closer */
}
"""

PROSE = """/*
 * Notes: a previous form used a
 * /* FAKE *\/ annotation and self_vet.md cites it.
 */
s32 k(void) { return 0; }
"""

NESTED_MULTI = """void n(void)
{
    do { /* FAKE: outer */
        do { /* FAKE: inner */
            q();
        } while (0);
    } while (0);
}
"""


class TestCommentBlockMarkers(unittest.TestCase):
    def test_comment_block_above_wrap(self):
        units = find_fake_units(BLOCK)
        self.assertEqual([u.kind for u in units], ["wrap", "line"])
        # 2 comment lines (3,4) + do{ (5) + }while(0); (7)
        self.assertEqual(units[0].lines, [3, 4, 5, 7])
        out = remove_units(BLOCK, [units[0]])
        self.assertNotIn("do {", out)
        self.assertNotIn("while (0)", out)
        self.assertNotIn("single-level wrap", out)   # comment block gone too
        self.assertIn("r = a;", out)

    def test_comment_block_above_statement(self):
        units = find_fake_units(BLOCK)
        self.assertEqual(units[1].kind, "line")
        self.assertEqual(units[1].lines, [8, 9])   # comment line + the decl
        out = remove_units(BLOCK, [units[1]])
        self.assertNotIn("three = 3", out)
        self.assertIn("do {", out)

    def test_prose_fake_inside_comment_is_not_a_unit(self):
        self.assertEqual(find_fake_units(PROSE), [])


class TestOneLineAndCloser(unittest.TestCase):
    def test_units_found(self):
        units = find_fake_units(ONELINE)
        self.assertEqual([u.kind for u in units], ["wrap", "wrap", "line", "wrap"])

    def test_oneline_wrap_rewritten_keeps_body(self):
        units = find_fake_units(ONELINE)
        out = remove_units(ONELINE, [units[0]])
        self.assertIn("g();", out)
        self.assertNotIn("do { g();", out)
        self.assertNotIn("/* FAKE: wrap */", out)

    def test_nested_oneliner_peels_one_level(self):
        units = find_fake_units(ONELINE)
        out = remove_units(ONELINE, [units[1]])
        line = [l for l in out.splitlines() if "*p = 0" in l][0]
        self.assertEqual(line.strip(), "do { *p = 0; } while (0);")

    def test_inline_slashslash_marker(self):
        units = find_fake_units(ONELINE)
        out = remove_units(ONELINE, [units[2]])
        self.assertNotIn("x = 1;", out)

    def test_marker_on_closer_resolves_to_wrap(self):
        units = find_fake_units(ONELINE)
        self.assertEqual(len(units[3].lines), 2)
        out = remove_units(ONELINE, [units[3]])
        self.assertIn("y = 2;", out)
        self.assertNotIn("marker on the closer", out)

    def test_nested_multiline_wraps(self):
        units = find_fake_units(NESTED_MULTI)
        self.assertEqual([u.kind for u in units], ["wrap", "wrap"])
        self.assertEqual(units[0].lines, [2, 6])
        self.assertEqual(units[1].lines, [3, 5])


class TestInvariants(unittest.TestCase):
    def test_remove_nothing_is_identity(self):
        for src in (BODY, BLOCK, ONELINE, NESTED_MULTI):
            self.assertEqual(remove_units(src, []), src)

    def test_masks_six_is_23(self):
        m = ablation_masks(6)
        self.assertEqual(len(m), 23)
        self.assertEqual(len(set(m)), 23)


if __name__ == "__main__":
    unittest.main()
