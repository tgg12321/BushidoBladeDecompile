"""Per-function COMMON gate (maspsx_comm_syms.txt; owner ruling 2026-09-26, fourth batch).

Sony ASPSX 2.34 gives a COMMON (tentative-definition, `.comm`) symbol gp for its
base only, never for `sym+N`. Our C declares such variables `extern` and the
sdata_syms list makes them gp, so maspsx needs the storage class per function:
inside a listed function an operand `sym+N` of a listed symbol keeps maspsx's
ordinary non-gp form; the base access, unlisted symbols and unlisted functions
are untouched. Shape taken from cdrom_SetMix (asm/funcs/cdrom_SetMix.s)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments


def body(func):
    return [
        f".ent\t{func}",
        "sb\t$4,g_cd_atv",
        "sb\t$5,g_cd_atv+1",
        "lbu\t$2,g_cd_atv+3",
        "sb\t$6,D_800A36B8+2",
        f".end\t{func}",
    ]


def run(lines, comm_sym_map=None):
    mp = MaspsxProcessor(
        lines,
        sdata_sym_list=["g_cd_atv", "D_800A36B8"],
        comm_sym_map=comm_sym_map,
    )
    return [l for l in strip_comments(mp.process_lines()) if not l.startswith(".")]


class TestCommSymsGate(unittest.TestCase):
    def test_listed_symbol_offset_is_not_gp_base_is(self):
        res = run(body("cdrom_SetMix"), {"cdrom_SetMix": {"g_cd_atv"}})
        self.assertEqual(res[0], "sb\t$4,%gp_rel(g_cd_atv)($gp)")
        self.assertEqual(res[1], "sb\t$5,g_cd_atv+1")
        self.assertIn("lbu\t$2,g_cd_atv+3", res)
        # an unlisted symbol in the same function keeps gp at its offset
        self.assertIn("sb\t$6,%gp_rel(D_800A36B8+2)($gp)", res)

    def test_inert_for_unlisted_function(self):
        stock = run(body("other"))
        self.assertEqual(run(body("other"), {"cdrom_SetMix": {"g_cd_atv"}}), stock)
        self.assertIn("sb\t$5,%gp_rel(g_cd_atv+1)($gp)", stock)

    def test_inert_without_list(self):
        self.assertEqual(run(body("cdrom_SetMix"), {}), run(body("cdrom_SetMix")))

    def test_only_removes_gp(self):
        # every line the gate changes is a gp-relative line in the stock output
        stock = run(body("cdrom_SetMix"))
        gated = run(body("cdrom_SetMix"), {"cdrom_SetMix": {"g_cd_atv", "D_800A36B8"}})
        changed = [(a, b) for a, b in zip(stock, gated) if a != b]
        self.assertEqual(len(stock), len(gated))
        self.assertTrue(changed)
        for a, b in changed:
            self.assertIn("%gp_rel(", a)
            self.assertNotIn("%gp_rel(", b)
            self.assertIn("+", b)


if __name__ == "__main__":
    unittest.main()
