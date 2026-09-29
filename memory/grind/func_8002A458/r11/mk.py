#!/usr/bin/env python3
"""Splice a func_8002A458 body variant into code6cac.c, dump all RTL passes with the
instrumented cc1, and print the function's asm.  Usage (WSL, repo root):
  python3 tmp/f1be20s2/mk.py <variant.c> <tag> [ENV=VAL ...]
"""
import os, re, subprocess, sys, shlex
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from engine import buildconfig as cfg

variant, tag = sys.argv[1], sys.argv[2]
env = dict(os.environ)
for kv in sys.argv[3:]:
    k, v = kv.split("=", 1)
    env[k] = v
out = ROOT / "tmp" / "rtl" / tag
out.mkdir(parents=True, exist_ok=True)
for p in out.iterdir():
    p.unlink()
src = (ROOT / "src" / "code6cac_b.c").read_text()
body = (ROOT / variant).read_text()
line = 'INCLUDE_ASM("asm/funcs", func_8002A458);'
assert line in src
src = src.replace(line, body)
(out / "code6cac_b.c").write_text(src)
cmd = "%s %s %s code6cac_b.c" % (cfg.CPP, cfg.CPP_FLAGS.replace("-Iinclude", "-I" + shlex.quote(str(ROOT / "include"))), cfg.CPP_DEFS)
r = subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True)
(out / "in.i").write_text(r.stdout)
cc1 = str(ROOT / env.get("CC1", "tools/gcc-2.7.2/build/cc1"))
cmd = "%s %s -da in.i -o in.s" % (shlex.quote(cc1), cfg.CC_FLAGS)
r = subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True, env=env)
(out / "stderr.txt").write_text(r.stderr); (out / "cmd.txt").write_text(cmd + "\n")
s = (out / "in.s").read_text().splitlines()
start = next(i for i, l in enumerate(s) if l.startswith("func_8002A458:"))
end = next(i for i in range(start, len(s)) if s[i].strip().startswith(".end\tfunc_8002A458"))
(out / "func.s").write_text("\n".join(s[start:end + 1]) + "\n")
for p in ("rtl", "jump", "cse", "loop", "cse2", "flow", "combine", "sched", "lreg", "greg", "sched2"):
    t = (out / ("in.i." + p)).read_text()
    i = t.find(";; Function func_8002A458")
    j = t.find(";; Function", i + 10)
    (out / ("f." + p)).write_text(t[i:j] if j > 0 else t[i:])
print("wrote", out)
