"""Build the Ruling 9 candidate: the rejected shared-q body with `q` renamed `cells`.

Keeps the per-site (q0..q3) receipt as rejected/per-site-locals-25.c.
"""
import re, shutil
from pathlib import Path

g = Path("memory/grind/func_800759D0")
src = (g / "rejected/function-scope-q-multiwrite-0.c").read_text(encoding="utf-8")
# drop the leading REJECTED banner comment
assert src.startswith("/* REJECTED")
src = src[src.index("*/") + 2:].lstrip("\n")
body = re.sub(r"\bq\b", "cells", src)
body = body.replace(
    "    s32 cells;\n",
    "    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts\n"
    "       just past the sheet's 12-byte headers: one header on the frame sheet\n"
    "       (+0xC), three on the name-plate sheets (+0x24). */\n"
    "    s32 cells;\n",
)
per_site = g / "rejected/per-site-locals-25.c"
if not per_site.exists():
    shutil.copyfile(g / "candidate.c", per_site)
with open(g / "candidate.c", "w", encoding="utf-8", newline="\n") as f:
    f.write(body)
print(body.count("cells"))
