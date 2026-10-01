#!/usr/bin/env python3
"""Step 12 (Q65, per-file-gp-model.md "What the adoption changes" 1, second global fix): maspsx models an
uninitialized `static` (our cc1: `.local S` + `.comm S,size,align`; cc1psx: `.lcomm S,size`) as `.lcomm`:
the file's own storage (.sbss when size <= the -G limit, else .bss), never COMMON, so Sony ASPSX 2.34 gives
it gp at every offset (probe lcomm4); it stays a LOCAL symbol. Replaces the fail-closed ValueError of the Q62
parser fix. Inert today (no C static exists; maspsx has no -G yet): byte-neutral.
usage: s12_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
P = "tools/maspsx/maspsx/__init__.py"
sub1(P, """                if line.startswith(".comm") and symbol in self.local_symbols:
                    # `.local sym` + `.comm` = an uninitialized static; cc1psx wrote
                    # `.lcomm`, which ASPSX treats differently (gp at every offset).
                    # Not modelled: fail closed, as before the three-field parse.
                    raise ValueError(
                        f".local+.comm {symbol} (uninitialized static) is not modelled; "
                        "ASPSX saw .lcomm -- needs its own ruling"
                    )
                size = int(size_str)""",
"""                if line.startswith(".comm") and symbol in self.local_symbols:
                    # `.local sym` + `.comm` = an uninitialized static; cc1psx wrote
                    # `.lcomm sym,size`. Sony ASPSX 2.34 keeps it in the file's own
                    # .sbss when small (gp at EVERY offset) and PSYLINK allocates
                    # each file's statics in link order (owner ruling Q65). Model it
                    # exactly as `.lcomm`: never COMMON, a local symbol.
                    self.static_symbols.add(symbol)
                    size = int(size_str)
                    if size <= self.sdata_limit:
                        self.sbss_entries[symbol] = size
                    else:
                        self.bss_entries[symbol] = size
                    continue
                size = int(size_str)""")
sub1(P, """        # symbols named by `.local` (our cc1's svr4 form of an uninitialized static)
        self.local_symbols: set[str] = set()""",
"""        # symbols named by `.local` (our cc1's svr4 form of an uninitialized static)
        self.local_symbols: set[str] = set()
        # uninitialized statics (`.local` + `.comm`): emitted as local storage, never COMMON
        self.static_symbols: set[str] = set()""")
sub1(P, """                if section == "sbss":
                    if size >= 8:
                        res.append("\\t.align 3")
                    elif size >= 4:
                        res.append("\\t.align 2")
                    elif size >= 2:
                        res.append("\\t.align 1")

                # only mark bss symbols as global
                if section == "bss":""",
"""                if section == "sbss" or symbol in self.static_symbols:
                    if size >= 8:
                        res.append("\\t.align 3")
                    elif size >= 4:
                        res.append("\\t.align 2")
                    elif size >= 2:
                        res.append("\\t.align 1")

                # only mark bss symbols as global (a static stays local)
                if section == "bss" and symbol not in self.static_symbols:""")

# tests: the fail-closed test becomes the model test; a unit test and an engine test for the model
T = "tools/maspsx/tests/test_comm_global.py"
sub1(T, '''uninitialized static (`.local sym` + `.comm`) is NOT modelled and must fail closed."""''',
     '''uninitialized static (`.local sym` + `.comm`) is modelled as cc1psx's `.lcomm` (owner ruling Q65): file-own
storage, never COMMON, a local symbol, gp at every offset when small (tests/test_static_lcomm.py)."""''')
sub1(T, '''    def test_local_comm_static_fails_closed(self):
        with self.assertRaises(ValueError):
            run([".local\\tg_cd_atv", ".comm\\tg_cd_atv,4,4"] + BODY, use_comm_section=True)
''', '''    def test_local_comm_static_is_not_common(self):
        res = run([".local\\tg_cd_atv", ".comm\\tg_cd_atv,4,4"] + BODY, use_comm_section=True)
        self.assertNotIn(".comm g_cd_atv,4", res)
        self.assertIn("g_cd_atv:", res)
        self.assertIn("sb\\t$5,%gp_rel(g_cd_atv+1)($gp)", res)  # a static takes gp at an offset
''')
wr("tools/maspsx/tests/test_static_lcomm.py", '''"""An uninitialized static (`.local S` + `.comm S,size,align`; cc1psx `.lcomm`) is the file's own storage:
never COMMON, a local symbol, gp at EVERY offset when small; a tentative definition (`.comm`) only at its
base (owner ruling Q65; Sony ASPSX 2.34 probes lcomm4 / comm4)."""
import unittest

from maspsx import MaspsxProcessor

from .util import strip_comments


def run(lines, g=8):
    return [l for l in strip_comments(MaspsxProcessor(lines, sdata_limit=g).process_lines()) if l.strip()]


class TestStaticLcomm(unittest.TestCase):
    def test_static_is_gp_at_every_offset_and_local(self):
        res = run([".text", ".ent\\tf", "lb\\t$4,st", "lb\\t$5,st+1", ".end\\tf", ".local\\tst", ".comm\\tst,4,4"])
        self.assertIn("lb\\t$4,%gp_rel(st)($gp)", res)
        self.assertIn("lb\\t$5,%gp_rel(st+1)($gp)", res)
        self.assertIn("st:", res)
        self.assertNotIn(".globl st", " ".join(res))

    def test_tentative_is_gp_at_base_only(self):
        res = run([".text", ".ent\\tf", "lb\\t$4,cm", "lb\\t$5,cm+1", ".end\\tf", ".comm\\tcm,4,4"])
        self.assertIn("lb\\t$4,%gp_rel(cm)($gp)", res)
        self.assertIn("lb\\t$5,cm+1", res)

    def test_static_without_g_is_local_bss_not_gp(self):
        res = run([".text", ".ent\\tf", "lb\\t$4,st", ".end\\tf", ".local\\tst", ".comm\\tst,4,4"], g=0)
        self.assertNotIn("lb\\t$4,%gp_rel(st)($gp)", res)
        self.assertNotIn(".globl st", " ".join(res))


if __name__ == "__main__":
    unittest.main()
''')
sub1("engine/test_engine.py", """def test_maspsx_fingerprint() -> None:""", '''def test_maspsx_static_lcomm() -> None:
    """Owner ruling Q65: an uninitialized static (`.local`+`.comm`, cc1psx `.lcomm`) is file-own small data
    under -G8, gp at every offset, never COMMON; the Q62-era fail-closed error is gone."""
    import sys
    sys.path.insert(0, str(Path("tools/maspsx").resolve()))
    try:
        from maspsx import MaspsxProcessor
    finally:
        sys.path.pop(0)
    lines = [".text", ".ent\\tf", "lb\\t$4,st", "lb\\t$5,st+1", ".end\\tf", ".local\\tst", ".comm\\tst,4,4"]
    res = [l.split("#")[0].strip() for l in MaspsxProcessor(lines, sdata_limit=8).process_lines()]
    eq("maspsx -G8: a static is gp at base and offset",
       [l for l in res if l.startswith("lb")], ["lb\\t$4,%gp_rel(st)($gp)", "lb\\t$5,%gp_rel(st+1)($gp)"])


def test_maspsx_fingerprint() -> None:''')
sub1("engine/test_engine.py", "    test_maspsx_fingerprint()\n", "    test_maspsx_static_lcomm()\n    test_maspsx_fingerprint()\n")
print("step 12 applied")
