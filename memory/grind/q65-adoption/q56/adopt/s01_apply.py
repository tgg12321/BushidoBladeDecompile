#!/usr/bin/env python3
"""Step 1 (Q65 adoption): maspsx `_uses_gp` ignores an INDEXED operand `sym($reg)`.
Applied to the scratch clone given as argv[1]."""
import sys
from pathlib import Path
R = Path(sys.argv[1])

def sub1(p, a, b):
    t = (R / p).read_text()
    assert t.count(a) == 1, (p, a[:80])
    (R / p).write_text(t.replace(a, b))

sub1("tools/maspsx/maspsx/__init__.py",
"""    def _uses_gp(self, line: str) -> bool:
        if self.sdata_limit == 0 and not self.sdata_sym_list:
            return False

        line = strip_comments(line)
        if uses_at(line):
            op, *rest = line.split("\\t")
            if op in load_mnemonics or op in store_mnemonics:
                (
                    _,
                    _,
                    operand,
                    _,
                    _,
                ) = parse_load_or_store(" ".join(rest))

                if operand.count("+") == 1:""",
"""    def _uses_gp(self, line: str) -> bool:
        if self.sdata_limit == 0 and not self.sdata_sym_list:
            return False

        line = strip_comments(line)
        if uses_at(line):
            op, *rest = line.split("\\t")
            if op in load_mnemonics or op in store_mnemonics:
                (
                    r_source,
                    _,
                    operand,
                    _,
                    _,
                ) = parse_load_or_store(" ".join(rest))

                # An indexed operand `sym($reg)` is expanded through $at
                # (lui $at; addu $at,$at,$reg; op 0($at)), never gp-relative,
                # so it never makes the preceding load need a delay nop. Sony
                # ASPSX 2.34 agrees: `lbu $2,0($4); sb $2,sym($3)` with `sym`
                # small data defined in the file assembles with no nop
                # (tmp/q56/aspsx_probe_results.txt, owner ruling Q65).
                if r_source is not None:
                    return False

                if operand.count("+") == 1:""")

(R / "tools/maspsx/tests/test_indexed_gp_nop.py").write_text('''"""An indexed store `sym($reg)` of a gp-eligible symbol is expanded through $at, never
gp-relative, so it does not make the preceding load of the stored register need a
load-delay nop. Sony ASPSX 2.34 (psyq3.5) assembles
`lbu $2,0($4); sb $2,sym($3)` with no nop when `sym` is small data; a direct
`sb $2,sym` is gp-relative and does take the nop (owner ruling Q65)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments


def run(store):
    lines = [".ent\\tf", "lbu\\t$2,0($4)", store, ".end\\tf"]
    mp = MaspsxProcessor(lines, sdata_sym_list=["sym"])
    return [l for l in strip_comments(mp.process_lines()) if not l.startswith(".")]


class TestIndexedGpNop(unittest.TestCase):
    def test_indexed_store_takes_no_nop(self):
        res = run("sb\\t$2,sym($3)")
        self.assertEqual(res[0], "lbu\\t$2,0($4)")
        self.assertNotIn("nop", res)

    def test_direct_store_is_gp_and_takes_the_nop(self):
        res = run("sb\\t$2,sym")
        self.assertEqual(res, ["lbu\\t$2,0($4)", "nop", "sb\\t$2,%gp_rel(sym)($gp)"])


if __name__ == "__main__":
    unittest.main()
''')

sub1("engine/test_engine.py",
"""def test_maspsx_fingerprint() -> None:""",
'''def test_maspsx_indexed_operand_not_gp() -> None:
    """Owner ruling Q65 (2026-09-30), step 1: maspsx's load-delay helper `_uses_gp`
    treated an INDEXED operand `sym($reg)` of a gp-eligible symbol as gp-relative, so
    `lbu $2,..; sb $2,sym($3)` got a spurious nop (the expansion itself goes through
    $at). Sony ASPSX 2.34 emits no nop there; a direct `sb $2,sym` stays gp with the nop.
    Byte-neutral under the sdata lists (verify-oracle); load-bearing under Q65's
    per-file model (func_8003047C)."""
    import sys
    sys.path.insert(0, str(Path("tools/maspsx").resolve()))
    try:
        from maspsx import MaspsxProcessor
    finally:
        sys.path.pop(0)

    def run(store):
        mp = MaspsxProcessor([".ent\\tf", "lbu\\t$2,0($4)", store, ".end\\tf"], sdata_sym_list=["sym"])
        return [l.split("#")[0].strip() for l in mp.process_lines()
                if l.split("#")[0].strip() and not l.lstrip().startswith(("#", "."))]

    eq("maspsx: indexed store of a gp-eligible symbol takes no load-delay nop",
       "nop" in run("sb\\t$2,sym($3)"), False)
    eq("maspsx: direct store stays gp-relative with its nop",
       run("sb\\t$2,sym"), ["lbu\\t$2,0($4)", "nop", "sb\\t$2,%gp_rel(sym)($gp)"])


def test_maspsx_fingerprint() -> None:''')
sub1("engine/test_engine.py", "    test_maspsx_fingerprint()\n", "    test_maspsx_indexed_operand_not_gp()\n    test_maspsx_fingerprint()\n")
print("step 1 applied")
