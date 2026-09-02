#!/usr/bin/env python3
"""Unit tests for tools/loop_movables.py parsers. Run: python tools/test_loop_movables.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.loop_movables import parse_loops, count_sets_in_range, function_segment

LOOP_DUMP = """;; Function func_A

Loop from 12 to 88: 56 real insns.
Insn 20: regno 74 (life 1), savings 1  moved to 9
Insn 31: regno 77 (life 1), consec 1, move-insn savings 1  moved to 10
Insn 40: regno 80 (life 15), savings 1 not desirable
Insn 45: regno 81 (life 2), cond not safe

;; Function func_B

Loop from 5 to 9 is phony.

"""

CSE_RTL = """(note 12 11 13 "" NOTE_INSN_LOOP_BEG)
(insn 20 13 21 (set (reg:SI 74) (const_int 42)) -1 (nil) (nil))
(insn 22 21 23 (set (reg:SI 80) (plus:SI (reg:SI 80) (const_int 1))) -1 (nil) (nil))
(insn 24 23 25 (set (reg:SI 80) (const_int 7)) -1 (nil) (nil))
(call_insn 30 25 31 (call (mem:SI (symbol_ref:SI ("f"))) (const_int 0)) -1 (nil) (nil))
(note 88 87 89 "" NOTE_INSN_LOOP_END)
"""

# A verbatim excerpt of a REAL cse dump (tmp/grind/func_80017848/dumps/ings.cse):
# multi-line insns terminated by `) <code> {<template>} (nil)`, blank-line separated.
REAL_CSE = """(insn 4 2 6 (set (reg/v:SI 72)
        (reg:SI 4 a0)) 159 {movsi_internal2} (nil)
    (nil))

(insn 13 10 15 (set (reg:SI 78)
        (plus:SI (reg/v:SI 72)
            (const_int 4))) 3 {addsi3_internal} (nil)
    (nil))

(call_insn 21 19 23 (parallel[ 
            (set (reg:SI 2 v0)
                (call (mem:SI (symbol_ref:SI ("func_80083698")))
                    (const_int 16)))
            (clobber (reg:SI 31 ra))
        ] ) -1 (nil)
    (nil)
    (expr_list (use (reg:SI 6 a2))
        (nil)))

(insn 26 23 27 (set (reg:SI 78)
        (const_int -1)) 159 {movsi_internal2} (nil)
    (expr_list:REG_EQUAL (const_int -1)
        (nil)))
"""


class TestParsers(unittest.TestCase):
    def test_function_segment(self):
        seg = function_segment(LOOP_DUMP, "func_A")
        self.assertIn("56 real insns", seg)
        self.assertNotIn("phony", seg)

    def test_parse_loops(self):
        loops = parse_loops(function_segment(LOOP_DUMP, "func_A"))
        self.assertEqual(len(loops), 1)
        lp = loops[0]
        self.assertEqual((lp["start"], lp["end"], lp["insn_count"]), (12, 88, 56))
        self.assertEqual(len(lp["movables"]), 4)
        m0, m1, m2, m3 = lp["movables"]
        self.assertEqual((m0["insn"], m0["regno"], m0["life"], m0["savings"], m0["decision"]),
                         (20, 74, 1, 1, "moved"))
        self.assertEqual(m1["consec"], 1)
        self.assertTrue(m1["move_insn"])
        self.assertEqual(m2["decision"], "not desirable")
        self.assertTrue(m3["cond"])
        self.assertEqual(m3["decision"], "not safe")

    def test_count_sets_and_calls(self):
        sets, has_call = count_sets_in_range(CSE_RTL, 12, 88)
        self.assertEqual(sets[74], 1)
        self.assertEqual(sets[80], 2)
        self.assertTrue(has_call)

    # --- regression tests against the REAL dump formats (see module docstring) ---

    def test_real_movable_lines(self):
        """Verbatim lines from tmp/grind/*/dumps/*.loop."""
        seg = """Loop from 100 to 405: 85 real insns.
Insn 100: possible biv, reg 73, const = 55
Insn 100: dest address src reg 86 benefit 2 used 1 lifetime 1 replaceable mult 1 add 5
Insn 100: regno 107 (life 2), move-insn savings 2  moved to 409
Insn 106: regno 98 (life 2), done move-insn matches 69 
Insn 115: regno 100 (life 1), move-insn savings 1 not desirable
"""
        lp = parse_loops(seg)[0]
        self.assertEqual(lp["insn_count"], 85)
        # biv / giv / "dest address" lines must NOT be parsed as movables
        self.assertEqual(len(lp["movables"]), 3)
        a, b, c = lp["movables"]
        self.assertEqual((a["regno"], a["savings"], a["decision"], a["moved_to"]),
                         (107, 2, "moved", 409))
        self.assertTrue(a["move_insn"])
        self.assertTrue(b["done"])
        self.assertEqual(b["matches"], 69)
        self.assertIsNone(b["savings"])
        self.assertEqual(b["decision"], "(no decision line)")
        self.assertEqual(c["decision"], "not desirable")

    def test_real_multiline_rtl(self):
        """One count per insn on real multi-line RTL; hard-reg and pseudo dests."""
        sets, has_call = count_sets_in_range(REAL_CSE, 0, 1000)
        self.assertTrue(has_call)
        self.assertEqual(sets[78], 2)   # insns 13 and 26
        self.assertEqual(sets[72], 1)   # insn 4 -> (reg/v:SI 72)
        self.assertEqual(sets[2], 1)    # set inside the call parallel
        self.assertEqual(sets[31], 1)   # (clobber (reg:SI 31 ra))
        # range filtering by UID
        sets2, has_call2 = count_sets_in_range(REAL_CSE, 22, 27)
        self.assertFalse(has_call2)
        self.assertEqual(sets2, {78: 1})


if __name__ == "__main__":
    unittest.main()
