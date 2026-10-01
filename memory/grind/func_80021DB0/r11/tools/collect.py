#!/usr/bin/env python3
"""collect.py FUNCDIR OUT variant... : bank (D)(1) excerpts for each dumped variant d_<variant>/:
command lines, instcheck, the user-variable pseudo table, the BB2_ALLOC_DEBUG order, every
FINDREGDBG record captured, and the .lreg 'Register N' summary lines of the user variables."""
import re
import subprocess
import sys
from pathlib import Path

fd, out = Path(sys.argv[1]), Path(sys.argv[2])
tool = fd / "r11tools" / "r11table.py"
buf = []
for v in sys.argv[3:]:
    d = fd / f"d_{v}"
    buf.append(f"################ {v}  (body: {v}.c)")
    buf.append((d / "cmd.txt").read_text().rstrip())
    buf.append((d / "instcheck.txt").read_text().rstrip())
    tab = subprocess.run([sys.executable, str(tool), str(d)], capture_output=True, text=True).stdout
    buf.append("-- user-variable pseudos (r11table.py)")
    buf.append(tab.rstrip())
    pseudos = {int(m.group(1)) for m in re.finditer(r"^\S+\s+\S+\s+(\d+)", tab, re.M)}
    lreg = (d / "tu.i.lreg.fn").read_text()
    buf.append("-- .lreg summaries (user variables)")
    for m in re.finditer(r"^Register (\d+) .*$", lreg, re.M):
        if int(m.group(1)) in pseudos:
            buf.append(m.group(0))
    buf.append("-- BB2_ALLOC_DEBUG (global.c allocation order)")
    buf.append((d / "alloc.txt").read_text().rstrip())
    for f in sorted(d.glob("findreg_*.txt")):
        buf.append(f"-- {f.name}")
        buf.append(f.read_text().rstrip())
    buf.append("")
out.write_bytes(("\n".join(buf) + "\n").encode())
print("wrote", out)
