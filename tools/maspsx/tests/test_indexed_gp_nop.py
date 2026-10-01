"""An indexed store `sym($reg)` of a gp-eligible symbol is expanded through $at, never
gp-relative, so it does not make the preceding load of the stored register need a
load-delay nop. Sony ASPSX 2.34 (psyq3.5) assembles
`lbu $2,0($4); sb $2,sym($3)` with no nop when `sym` is small data; a direct
`sb $2,sym` is gp-relative and does take the nop (owner ruling Q65)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments


def run(store):
    lines = [".ent\tf", "lbu\t$2,0($4)", store, ".end\tf"]
    mp = MaspsxProcessor(lines, sdata_sym_list=["sym"])
    return [l for l in strip_comments(mp.process_lines()) if not l.startswith(".")]


class TestIndexedGpNop(unittest.TestCase):
    def test_indexed_store_takes_no_nop(self):
        res = run("sb\t$2,sym($3)")
        self.assertEqual(res[0], "lbu\t$2,0($4)")
        self.assertNotIn("nop", res)

    def test_direct_store_is_gp_and_takes_the_nop(self):
        res = run("sb\t$2,sym")
        self.assertEqual(res, ["lbu\t$2,0($4)", "nop", "sb\t$2,%gp_rel(sym)($gp)"])


if __name__ == "__main__":
    unittest.main()
