#!/usr/bin/env python3
# score_b2.py BASEFILE [LIST] : engine-sandbox scoring (--disable all, cheat-asm stripped) of candidate
# bodies substituted into a whole-file scratch copy (BASEFILE, e.g. tmp/p2/f01b2/51268.c) instead of
# the tree's src/main/51268.c, which still holds F01b1 (D_800A3478 / 347C s32) while F01b2 is prepared.
# Mirrors engine/sandbox.py sandbox_score(disable="all", strip_cheat_asm=True); the reference object is
# build/src/main/51268.o. LIST (default tmp/p2/f01b2/abl/list.txt): "<name> <func>" rows, bodies in
# <dir>/<name>.c. With no LIST rows it scores BASEFILE's own bodies of the funcs given after "--".
# BASEFILE's game.h / bb2.h (same dir) go ahead of include/. Run in WSL from the repo root, under the
# landing lock.
import shutil, subprocess, sys
from pathlib import Path
sys.path.insert(0, ".")
from engine import cheats, inlineasm, pipeline, score

STEM = "main/51268"
INC = "tmp/p2/f01b2/inc"
Path(INC).mkdir(parents=True, exist_ok=True)
for h in ("game.h", "bb2.h"):
    shutil.copyfile(str(Path(sys.argv[1]).parent / h), INC + "/" + h)
base = Path(sys.argv[1]).read_text(encoding="utf-8")
rows = []
if "--" in sys.argv:
    rows = [(None, f) for f in sys.argv[sys.argv.index("--") + 1:]]
else:
    lst = Path(sys.argv[2] if len(sys.argv) > 2 else "tmp/p2/f01b2/abl/list.txt")
    rows = [tuple(l.split()) for l in lst.read_text(encoding="utf-8").splitlines() if l.strip()]
    d = lst.parent
for name, func in rows:
    text = base if name is None else inlineasm.substitute_body(base, func, (d / (name + ".c")).read_text(encoding="utf-8"))
    wd = Path("tmp/p2/f01b2/sbx") / (name or func)
    ov = cheats.empty_overrides(str(wd / "cfg"))
    src_ovr = str(wd / "src" / "51268.c")
    inlineasm.write_stripped(STEM, src_ovr, text)
    ov["src_override"] = src_ovr
    out_o = str(wd / "51268.o")
    # the scratch headers (tmp/p2/f01b2/game.h / bb2.h) ahead of include/
    cmd = pipeline.c_pipeline_cmd(STEM, out_o, ov)
    assert cmd.count("-Iinclude -undef") == 1
    cmd = cmd.replace("-Iinclude -undef", "-I%s -Iinclude -undef" % INC)
    Path(out_o).parent.mkdir(parents=True, exist_ok=True)
    rr = subprocess.run(cmd, shell=True, executable="/bin/bash", capture_output=True, text=True)
    if rr.returncode != 0:
        print("%-22s %-14s BUILD FAILED %s" % (name or "(base)", func, rr.stderr[-300:]))
        continue
    try:
        r = score.score_func(out_o, "build/src/%s.o" % STEM, func)
        print("%-22s %-14s score %s  target %s  build %s" % (name or "(base)", func, r.get("score"), r.get("target_insns"), r.get("build_insns")))
    except KeyError as e:
        print("%-22s %-14s UNSCORABLE %s" % (name or "(base)", func, e))
