"""permfinds.py <workspace> [N] : normalize base.c with the permuter's own printer (ast_util.to_c) and
print, for the N best finds, the source lines that differ from the base -- i.e. what each find changed
(to record what the permuter's finds reuse)."""
import difflib
import os
import re
import sys

ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
sys.path.insert(0, os.path.join(ROOT, "tools", "decomp-permuter"))
from src import ast_util  # noqa: E402
from src.compiler import Compiler  # noqa: F401,E402
from pycparser import c_parser  # noqa: E402

W = sys.argv[1]
N = int(sys.argv[2]) if len(sys.argv) > 2 else 8
src = open(os.path.join(W, "base.c")).read()
src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
ast = ast_util.parse_c(src)
base = ast_util.to_c(ast).splitlines()
outs = []
for d in os.listdir(W):
    m = re.match(r"output-(\d+)-(\d+)$", d)
    if m:
        outs.append((int(m.group(1)), d))
for sc, d in sorted(outs)[:N]:
    f = open(os.path.join(W, d, "source.c")).read().splitlines()
    diff = [l for l in difflib.unified_diff(base, f, lineterm="", n=0) if l[:1] in "+-" and l[:3] not in ("+++", "---")]
    print("=== %s (permuter score %d): %d changed lines" % (d, sc, len(diff)))
    for l in diff[:14]:
        print("   " + l.strip())
