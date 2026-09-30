"""Ruling 11 (E)/(F): rename det -> work, sum -> temp (honest generic scratch names) and annotate.
usage: python tmp/gte/rename.py <in.c> <out.c> <func>"""
import re
import sys
from pathlib import Path

src, out, func = sys.argv[1:4]
t = Path(src).read_bytes().decode()
t = re.sub(r"\bdet\b", "work", t)
t = re.sub(r"\bsum\b", "temp", t)
ledger = f"memory/grind/{func}/r11/proof.md"
dw = ("    /* work holds two values (Ruling 11, owner 2026-09-26; proof: %s): the\n"
      "     * 3x3 determinant (the divisor of the six cofactors) and then the square root of\n"
      "     * c0*c0 + c1*c1 (the ratan2 length). */\n    s32 work;\n" % ledger)
dt = ("    /* temp holds two values (Ruling 11; proof: %s): c0*c0 + c1*c1 (the\n"
      "     * squared length fed to the table lookup and the leading-zero count) and then the\n"
      "     * square-root table byte. */\n    s32 temp;\n" % ledger)
assert t.count("    s32 work;\n") == 1 and t.count("    s32 temp;\n") == 1
t = t.replace("    s32 work;\n", dw).replace("    s32 temp;\n", dt)
Path(out).write_bytes(t.encode())
print("wrote", out)
