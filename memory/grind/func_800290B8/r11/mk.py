#!/usr/bin/env python3
"""Splice a func_800290B8 body variant into code6cac_b.c, dump all RTL passes, print nothing.
Usage (WSL, repo root): python3 tmp/f290b8/mk.py <variant.c> <tag> [ENV=VAL ...]"""
import os, subprocess, sys, shlex
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from engine import buildconfig as cfg
variant, tag = sys.argv[1], sys.argv[2]
env = dict(os.environ)
for kv in sys.argv[3:]:
    k, v = kv.split("=", 1); env[k] = v
out = ROOT / "tmp" / "rtl" / tag
out.mkdir(parents=True, exist_ok=True)
for p in out.iterdir(): p.unlink()
src = (ROOT / "src" / "code6cac_b.c").read_text()
line = 'INCLUDE_ASM("asm/funcs", func_800290B8);'
assert src.count(line) == 1
src = src.replace(line, (ROOT / variant).read_text())
(out / "code6cac_b.c").write_text(src)
cmd = "%s %s %s code6cac_b.c" % (cfg.CPP, cfg.CPP_FLAGS.replace("-Iinclude", "-I" + shlex.quote(str(ROOT / "include"))), cfg.CPP_DEFS)
r = subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True)
(out / "in.i").write_text(r.stdout)
cc1 = str(ROOT / env.get("CC1", "tools/gcc-2.7.2/build/cc1"))
cmd = "%s %s -da in.i -o in.s" % (shlex.quote(cc1), cfg.CC_FLAGS)
r = subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True, env=env)
(out / "stderr.txt").write_text(r.stderr); (out / "cmd.txt").write_text(cmd + "\n")
s = (out / "in.s").read_text().splitlines()
st = next(i for i, l in enumerate(s) if l.startswith("func_800290B8:"))
en = next(i for i in range(st, len(s)) if s[i].strip().startswith(".end\tfunc_800290B8"))
(out / "func.s").write_text("\n".join(s[st:en + 1]) + "\n")
for p in ("rtl", "jump", "cse", "loop", "cse2", "flow", "combine", "sched", "lreg", "greg", "sched2"):
    t = (out / ("in.i." + p)).read_text()
    i = t.find(";; Function func_800290B8")
    j = t.find(";; Function", i + 10)
    (out / ("f." + p)).write_text(t[i:j] if j > 0 else t[i:])
print("wrote", out)
