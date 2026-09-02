#!/usr/bin/env python3
"""Unit tests for tools/label_census.py. Run: python tools/test_label_census.py -v"""
import io, os, sys, unittest
from contextlib import redirect_stdout
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.label_census import parse_asm, census, _print

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

# Verbatim excerpt of a real cc1 -da dump: tmp/grind/func_800283D0/dumps/
# code6cac_b.s, function func_800272FC (tabs, .frame/.mask, `# 2` comment
# tails, and .set noreorder blocks in which cc1 HAS filled the delay slots).
BUILD = (
    "\t.align\t2\n"
    "\t.globl\tfunc_800272FC\n"
    "\t.type\t func_800272FC,@function\n"
    "\t.ent\tfunc_800272FC\n"
    "func_800272FC:\n"
    "\t.frame\t$sp,0,$31\t\t# vars= 0, regs= 0/0, args= 0, extra= 0\n"
    "\t.mask\t0x00000000,0\n"
    "\t.fmask\t0x00000000,0\n"
    "\t.set\tnoreorder\n"
    "\t.set\tnomacro\n"
    "\tbgez\t$4,.L7\n"
    "\tslt\t$2,$4,2561\n"
    "\t.set\tmacro\n"
    "\t.set\treorder\n"
    "\n"
    "\tsubu\t$4,$0,$4\n"
    "\tslt\t$2,$4,2561\n"
    ".L7:\n"
    "\tbeq\t$2,$0,.L3\n"
    "\tslt\t$2,$4,1536\n"
    "\t.set\tnoreorder\n"
    "\t.set\tnomacro\n"
    "\tbeq\t$2,$0,.L6\n"
    "\tli\t$3,0x00000002\t\t# 2\n"
    "\t.set\tmacro\n"
    "\t.set\treorder\n"
    "\n"
    "\t.set\tnoreorder\n"
    "\t.set\tnomacro\n"
    "\tj\t.L6\n"
    "\tli\t$3,0x00000001\t\t# 1\n"
    "\t.set\tmacro\n"
    "\t.set\treorder\n"
    "\n"
    ".L3:\n"
    "\tmove\t$3,$0\n"
    ".L6:\n"
    "\t.set\tnoreorder\n"
    "\t.set\tnomacro\n"
    "\tj\t$31\n"
    "\tmove\t$2,$3\n"
    "\t.set\tmacro\n"
    "\t.set\treorder\n"
    "\n"
    "\t.end\tfunc_800272FC\n"
    ".Lfe1:\n"
    "\t.size\t func_800272FC,.Lfe1-func_800272FC\n"
)

# splat emits jlabel (jump-table target), alabel (data address) and dlabel
# lines with NO trailing colon; endlabel closes the function.
JTBL = """glabel func_J
    /* 0 80000000 00000000 */  jr         $v0
    /* 4 80000004 00000000 */   nop
  jlabel .L80000008
    /* 8 80000008 00000000 */  addiu      $v0, $zero, 0x1
    /* C 8000000C 00000000 */  beqz       $v0, .L80000008
    /* 10 80000010 00000000 */   nop
    /* 14 80000014 00000000 */  jr         $v0
    /* 18 80000018 00000000 */   nop
  jlabel .L8000001C
    /* 1C 8000001C 00000000 */  lh         $v1, %lo(D_8010278E)($s1)
  alabel D_8008D070
    /* 20 80000020 00000000 */  jr         $ra
    /* 24 80000024 00000000 */   move      $fp, $30
  endlabel func_J
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
        c = census(BUILD, "func_800272FC")
        self.assertEqual(c["labels"][".L6"]["n_preds"], 3)
        self.assertEqual(c["reg_refs"]["$s0"], 0)

    # --- C1: splat jlabel / alabel / dlabel directives -----------------------

    def test_jlabel_is_a_label_not_an_instruction(self):
        ins, labels = parse_asm(JTBL, "func_J")
        # 10 real instructions; jlabel/alabel/endlabel are NOT instructions
        self.assertEqual(len(ins), 10)
        self.assertEqual([i["op"] for i in ins],
                         ["jr", "nop", "addiu", "beqz", "nop", "jr", "nop",
                          "lh", "jr", "move"])
        self.assertEqual(labels, {".L80000008": 2, ".L8000001C": 7})

    def test_jlabel_target_gets_branch_pred(self):
        c = census(JTBL, "func_J")
        # prev insn is the delay slot of an unconditional jr -> no fallthrough
        self.assertEqual(c["labels"][".L80000008"]["preds"], ["beqz@3"])

    def test_alabel_is_not_a_label(self):
        c = census(JTBL, "func_J")
        self.assertNotIn("D_8008D070", c["labels"])

    def test_build_insn_regex_not_applied_to_target_format(self):
        # "  endlabel func_J" looks like a build instruction; it must not be one
        ins, _ = parse_asm(JTBL, "func_J")
        self.assertNotIn("endlabel", [i["op"] for i in ins])

    # --- C2: absent function -------------------------------------------------

    def test_missing_function_yields_empty_census(self):
        c = census(BUILD, "func_NOT_HERE")
        self.assertTrue(c["missing"])
        self.assertEqual(c["n_insns"], 0)
        self.assertEqual(c["labels"], {})
        self.assertEqual(c["reg_refs"]["$s0"], 0)

    def test_present_function_is_not_missing(self):
        self.assertFalse(census(BUILD, "func_800272FC")["missing"])

    # --- I1: cc1 fills delay slots inside .set noreorder ---------------------

    def test_noreorder_delay_slot_suppresses_fallthrough(self):
        c = census(BUILD, "func_800272FC")
        # `j .L6` + filled slot `li $3,1` sit in a noreorder block right before
        # .L3 -> .L3 is reachable only via the beq, never by fall-through.
        self.assertEqual(c["labels"][".L3"]["preds"], ["beq@4"])
        self.assertEqual(c["labels"][".L3"]["n_preds"], 1)

    def test_noreorder_conditional_branch_still_falls_through(self):
        c = census(BUILD, "func_800272FC")
        # bgez + filled slot, but bgez is CONDITIONAL: .L7 keeps fall-through
        self.assertEqual(sorted(c["labels"][".L7"]["preds"]),
                         ["bgez@0", "fallthrough@3"])

    def test_lfe_label_filtered(self):
        _, labels = parse_asm(BUILD, "func_800272FC")
        self.assertNotIn(".Lfe1", labels)

    # --- M4/M5 ---------------------------------------------------------------

    def test_conditional_branch_delay_slot_keeps_fallthrough(self):
        text = ("glabel f\n"
                "    /* 0 80000000 00000000 */  beqz       $a0, .L80000010\n"
                "    /* 4 80000004 00000000 */   nop\n"
                "  .L80000008:\n"
                "    /* 8 80000008 00000000 */  addiu      $v0, $zero, 0x1\n"
                "  .L80000010:\n"
                "    /* 10 80000010 00000000 */  jr         $ra\n"
                "    /* 14 80000014 00000000 */   nop\n")
        c = census(text, "f")
        self.assertEqual(c["labels"][".L80000008"]["preds"], ["fallthrough@1"])

    def test_zero_pred_label_flagged_as_possible_jtbl_target(self):
        c = census(JTBL, "func_J")
        self.assertEqual(c["labels"][".L8000001C"]["n_preds"], 0)
        buf = io.StringIO()
        with redirect_stdout(buf):
            _print("t", c)
        self.assertIn("jtbl target?", buf.getvalue())

    def test_lo_reloc_operand_register_counted(self):
        c = census(JTBL, "func_J")
        self.assertEqual(c["reg_refs"]["$s1"], 1)   # %lo(D_8010278E)($s1)

    def test_numeric_fp_and_named_fp_normalised(self):
        # $30 -> $fp, $29 -> $sp
        text = "\t.ent\tf\nf:\n\tmove\t$30,$29\n\t.end\tf\n"
        ins2, _ = parse_asm(text, "f")
        self.assertEqual(ins2[0]["args"], "$fp,$sp")

    def test_preds_sorted_by_kind_then_numeric_index(self):
        text = ("glabel f\n"
                "    /* 0 80000000 00000000 */  beqz       $a0, .L80000030\n"
                "    /* 4 80000004 00000000 */   nop\n"
                "    /* 8 80000008 00000000 */  beqz       $a1, .L80000030\n"
                "    /* C 8000000C 00000000 */   nop\n")
        # 10 more instructions so the second branch index is >= 10 numerically
        for k in range(10):
            text += ("    /* %X %08X 00000000 */  nop\n"
                     % (0x10 + 4 * k, 0x80000010 + 4 * k))
        text += ("    /* 38 80000038 00000000 */  beqz       $a2, .L80000030\n"
                 "    /* 3C 8000003C 00000000 */   nop\n"
                 "  .L80000030:\n"
                 "    /* 40 80000040 00000000 */  jr         $ra\n"
                 "    /* 44 80000044 00000000 */   nop\n")
        c = census(text, "f")
        self.assertEqual(c["labels"][".L80000030"]["preds"],
                         ["beqz@0", "beqz@2", "beqz@14", "fallthrough@15"])


if __name__ == "__main__":
    unittest.main()
