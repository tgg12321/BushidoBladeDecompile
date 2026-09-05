"""Per-function prefill-label gate (owner ruling 2026-09-04, `main`).

ASPSX parity, "retarget iff filled": an UNFILLED (reorder-mode) branch to a
label L whose preceding instruction P is verbatim the delay-slot fill of a
filled branch to L is retargeted to a fresh label emitted before P. Shape taken
from the cc1 output for src/ings.c (tmp/grind/main/s2/ings.s:1761-1994)."""
import unittest

from maspsx import MaspsxProcessor, apply_prefill_label_gate


LOOP = [
    ".ent\tmain",
    "sw\t$0,D_800A36AC",
    "li\t$5,0x00001008\t\t# 4104",
    ".L128:",
    "lw\t$2,D_800A36AC",
    "lh\t$3,D_800A3834",
    ".set\tnoreorder",
    ".set\tnomacro",
    "bne\t$3,$2,.L128",
    "li\t$5,0x00001008\t\t# 4104",
    ".set\tmacro",
    ".set\treorder",
    ".set\tnoreorder",
    ".set\tnomacro",
    "bne\t$19,$0,.L128",
    "li\t$3,0x08000000\t\t# 134217728",
    ".set\tmacro",
    ".set\treorder",
    "lh\t$3,D_800A38DC",
    "#nop",
    "bne\t$3,$2,.L128",
    "andi\t$2,$2,0x00ff",
    "bne\t$2,$0,.L128",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t.L128",
    "li\t$5,0x00001008\t\t# 4104",
    ".set\tmacro",
    ".set\treorder",
    ".end\tmain",
]


class TestPrefillLabelGate(unittest.TestCase):
    def test_unfilled_branches_retarget_to_new_label_before_P(self):
        out, n = apply_prefill_label_gate(LOOP, {"main"})
        self.assertEqual(n, 2)
        i = out.index("li\t$5,0x00001008\t\t# 4104")
        self.assertEqual(out[i - 1], ".L128_pf:")
        self.assertEqual(out[i - 2], "sw\t$0,D_800A36AC")
        self.assertIn("bne\t$3,$2,.L128_pf", out)
        self.assertIn("bne\t$2,$0,.L128_pf", out)
        # filled branches keep their target and their slot
        self.assertEqual(out.count("bne\t$3,$2,.L128"), 1)
        self.assertIn("bne\t$19,$0,.L128", out)
        self.assertIn("j\t.L128", out)
        # nothing else changes: same lines plus exactly one new label
        self.assertEqual(len(out), len(LOOP) + 1)

    def test_inert_for_unlisted_function(self):
        out, n = apply_prefill_label_gate(LOOP, {"other"})
        self.assertEqual((out, n), (LOOP, 0))
        out, n = apply_prefill_label_gate(LOOP, set())
        self.assertEqual((out, n), (LOOP, 0))

    def test_requires_P_to_be_a_filled_slot(self):
        # P is not any filled branch's delay slot -> no retarget
        lines = [l.replace("li\t$5,0x00001008\t\t# 4104", "li\t$5,0x00000001", 1)
                 if i == 2 else l for i, l in enumerate(LOOP)]
        out, n = apply_prefill_label_gate(lines, {"main"})
        self.assertEqual((out, n), (lines, 0))

    def test_requires_an_instruction_before_the_label(self):
        lines = LOOP[:2] + LOOP[3:]          # drop P: the label now follows the sw
        out, n = apply_prefill_label_gate(lines, {"main"})
        self.assertEqual((out, n), (lines, 0))

    def test_processor_end_to_end_emits_label_and_nop_filled_branch(self):
        proc = MaspsxProcessor(LOOP, prefill_label_func_list=["main"])
        res = proc.process_lines()
        text = "\n".join(res)
        self.assertIn(".L128_pf:", text)
        self.assertIn("bne\t$3,$2,.L128_pf", text)
        self.assertLess(text.index(".L128_pf:"), text.index(".L128:"))

    def test_processor_inert_without_list(self):
        proc = MaspsxProcessor(LOOP)
        text = "\n".join(proc.process_lines())
        self.assertNotIn("_pf", text)


if __name__ == "__main__":
    unittest.main()
