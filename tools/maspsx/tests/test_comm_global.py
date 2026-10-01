"""Global COMMON model (owner ruling Q62, 2026-09-30, .claude/rules/maspsx-gate-lists.md § The global COMMON
model): our cc1 writes a public tentative definition as three-field `.comm sym,size,align`. maspsx parses it
(the alignment is implicit for COMMON, as in cc1psx's two-field form); with --use-comm-section the symbol
stays COMMON, and gp is used for its base only, never `sym+N` (the measured ASPSX 2.34 rule). An
uninitialized static (`.local sym` + `.comm`) is modelled as cc1psx's `.lcomm` (owner ruling Q65): file-own
storage, never COMMON, a local symbol, gp at every offset when small (tests/test_static_lcomm.py)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments


def run(lines, **kw):
    mp = MaspsxProcessor(lines, sdata_sym_list=["g_cd_atv"], **kw)
    return strip_comments(mp.process_lines())


BODY = [
    ".ent\tcdrom_SetMix",
    "sb\t$4,g_cd_atv",
    "sb\t$5,g_cd_atv+1",
    ".end\tcdrom_SetMix",
]


class TestCommGlobal(unittest.TestCase):
    def test_three_field_comm_is_common_and_emitted_without_space(self):
        res = run([".comm\tg_cd_atv,4,4"] + BODY, use_comm_section=True)
        self.assertIn(".comm g_cd_atv,4", res)  # strip_comments drops the leading tab
        self.assertFalse(any(".space" in l for l in res))

    def test_base_gp_offset_not(self):
        res = [l for l in run([".comm\tg_cd_atv,4,4"] + BODY, use_comm_section=True)
               if not l.startswith(".") and not l.startswith("\t.")]
        self.assertIn("sb\t$4,%gp_rel(g_cd_atv)($gp)", res)
        self.assertIn("sb\t$5,g_cd_atv+1", res)

    def test_local_comm_static_is_not_common(self):
        res = run([".local\tg_cd_atv", ".comm\tg_cd_atv,4,4"] + BODY, use_comm_section=True)
        self.assertNotIn(".comm g_cd_atv,4", res)
        self.assertIn("g_cd_atv:", res)
        self.assertIn("sb\t$5,%gp_rel(g_cd_atv+1)($gp)", res)  # a static takes gp at an offset


if __name__ == "__main__":
    unittest.main()
