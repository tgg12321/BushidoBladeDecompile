#!/usr/bin/env python3
"""Unit tests for tools/label_census.py. Run: python tools/test_label_census.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.label_census import parse_asm, census

TARGET = """glabel func_X
    /* 0 80000000 00000000 */  addiu      $sp, $sp, -0x18
    /* 4 80000004 00000000 */  beqz       $a0, .L8000001C
    /* 8 80000008 00000000 */   nop
    /* C 8000000C 00000000 */  lw         $s0, 0x0($a1)
    /* 10 80000010 00000000 */  b          .L80000024
    /* 14 80000014 00000000 */   addu      $s1, $s0, $zero
  .L8000001C:
    /* 1C 8000001C 00000000 */  addiu      $s0, $zero, 0x1
  .L80000024:
    /* 24 80000024 00000000 */  jr         $ra
    /* 28 80000028 00000000 */   addu      $v0, $s0, $zero
"""

BUILD = """\t.ent\tfunc_X
func_X:
\taddiu\t$sp,$sp,-24
\tbeq\t$4,$0,$L3
\tlw\t$16,0($5)
\tj\t$L4
$L3:
\tli\t$16,1
$L4:
\tmove\t$2,$16
\tj\t$31
\t.end\tfunc_X
"""


class TestCensus(unittest.TestCase):
    def test_parse_target_labels_and_branches(self):
        ins, labels = parse_asm(TARGET)
        self.assertEqual(labels, {".L8000001C": 6, ".L80000024": 7})
        self.assertEqual([i["op"] for i in ins][:3], ["addiu", "beqz", "nop"])

    def test_predecessors_and_fallthrough(self):
        c = census(TARGET, "func_X")
        self.assertEqual(sorted(c["labels"][".L8000001C"]["preds"]), ["beqz@1"])
        preds = sorted(c["labels"][".L80000024"]["preds"])
        self.assertEqual(preds, ["b@4", "fallthrough@6"])
        self.assertEqual(c["labels"][".L80000024"]["n_preds"], 2)

    def test_callee_saved_refcounts(self):
        c = census(TARGET, "func_X")
        self.assertEqual(c["reg_refs"]["$s0"], 4)
        self.assertEqual(c["reg_refs"]["$s1"], 1)

    def test_build_style_asm(self):
        c = census(BUILD, "func_X")
        self.assertEqual(c["labels"]["$L4"]["n_preds"], 2)
        self.assertEqual(c["reg_refs"]["$s0"], 3)   # $16 normalised to $s0


if __name__ == "__main__":
    unittest.main()
