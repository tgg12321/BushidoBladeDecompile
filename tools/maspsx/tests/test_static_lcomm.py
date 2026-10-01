"""An uninitialized static (`.local S` + `.comm S,size,align`; cc1psx `.lcomm`) is the file's own storage:
never COMMON, a local symbol, gp at EVERY offset when small; a tentative definition (`.comm`) only at its
base (owner ruling Q65; Sony ASPSX 2.34 probes lcomm4 / comm4)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments


def run(lines, g=8):
    return [l for l in strip_comments(MaspsxProcessor(lines, sdata_limit=g).process_lines()) if l.strip()]


class TestStaticLcomm(unittest.TestCase):
    def test_static_is_gp_at_every_offset_and_local(self):
        res = run([".text", ".ent\tf", "lb\t$4,st", "lb\t$5,st+1", ".end\tf", ".local\tst", ".comm\tst,4,4"])
        self.assertIn("lb\t$4,%gp_rel(st)($gp)", res)
        self.assertIn("lb\t$5,%gp_rel(st+1)($gp)", res)
        self.assertIn("st:", res)
        self.assertNotIn(".globl st", " ".join(res))

    def test_tentative_is_gp_at_base_only(self):
        res = run([".text", ".ent\tf", "lb\t$4,cm", "lb\t$5,cm+1", ".end\tf", ".comm\tcm,4,4"])
        self.assertIn("lb\t$4,%gp_rel(cm)($gp)", res)
        self.assertIn("lb\t$5,cm+1", res)

    def test_static_is_4_aligned_whatever_its_size(self):
        # Sony ASPSX 2.34 + PSYLINK (lcomm_align_probe, owner ruling Q79 / A8): a 1-byte static, then an
        # 8-byte one at +4: every `.lcomm` static is 4-aligned
        res = run([".text", ".ent\tf", "lb\t$4,a1", "lw\t$5,b8", ".end\tf",
                   ".local\ta1", ".comm\ta1,1,1", ".local\tb8", ".comm\tb8,8,4"])
        self.assertEqual(res[res.index("a1:") - 1], ".align 2")
        self.assertEqual(res[res.index("b8:") - 1], ".align 2")

    def test_static_without_g_is_local_bss_not_gp(self):
        res = run([".text", ".ent\tf", "lb\t$4,st", ".end\tf", ".local\tst", ".comm\tst,4,4"], g=0)
        self.assertNotIn("lb\t$4,%gp_rel(st)($gp)", res)
        self.assertNotIn(".globl st", " ".join(res))


if __name__ == "__main__":
    unittest.main()
