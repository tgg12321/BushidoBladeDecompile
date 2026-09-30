"""Full-file CD_ready variants: bios.c helpers moved above CD_sync (Sony's order), SOTN-shaped bodies.

usage: python3 tmp/CD_ready/gen.py <CD_ready body.c> [--sync <CD_sync body.c>] [--datasync <body.c>]
Writes tmp/CD_ready/full_<stem-of-first-arg>.c
"""
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import inlineasm  # noqa: E402

args = sys.argv[1:]
opt = {}
pos = []
i = 0
while i < len(args):
    if args[i] in ("--sync", "--datasync", "--name"):
        opt[args[i]] = args[i + 1]
        i += 2
    else:
        pos.append(args[i])
        i += 1

SRC = Path("src/system.c").read_bytes().decode("utf-8")
HSTART = "/* bios.c's alarm helpers, as in Sony's source"
HEND_MARK = "s32 CD_cw(u8 com, u8 *param, u8 *result, s32 async)\n"
i0 = SRC.index(HSTART)
i1 = SRC.index(HEND_MARK)
helpers = SRC[i0:i1]
t = SRC[:i0] + SRC[i1:]
SYNC = "s32 CD_sync(s32 a0, u8 *a1)\n"
assert t.count(SYNC) == 1
t = t.replace(SYNC, 'extern char D_80016248[]; /* "CD_ready" */\n\n' + helpers + SYNC)
t = inlineasm.substitute_body(t, "CD_ready", Path(pos[0]).read_bytes().decode("utf-8"))
if "--sync" in opt:
    t = inlineasm.substitute_body(t, "CD_sync", Path(opt["--sync"]).read_bytes().decode("utf-8"))
if "--datasync" in opt:
    t = inlineasm.substitute_body(t, "CD_datasync", Path(opt["--datasync"]).read_bytes().decode("utf-8"))
name = opt.get("--name", Path(pos[0]).stem)
Path(f"tmp/CD_ready/full_{name}.c").write_bytes(t.encode("utf-8"))
print("wrote", name)
