#!/usr/bin/env python3
"""Splice a func_8008B488 body variant into a scratch copy of src/main.c (with the
landing chassis: the two hand-transcribed jtbl consts deleted), dump every RTL
pass for func_8008B488 into tmp/rtl/<tag>/.
Usage (WSL, repo root): python3 tmp/f8b488s4/mk.py <variant.c> <tag> [ENV=VAL ...]
  CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo> for the instrumented cc1."""
import os, re, subprocess, sys, shlex
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from engine import buildconfig as cfg
FN = "func_8008B488"
variant, tag = sys.argv[1], sys.argv[2]
env = dict(os.environ)
for kv in sys.argv[3:]:
    k, v = kv.split("=", 1); env[k] = v
out = ROOT / "tmp" / "rtl" / tag
out.mkdir(parents=True, exist_ok=True)
for p in out.iterdir(): p.unlink()
# the committed main.c (INCLUDE_ASM form), never the working tree: a landing may be spliced there
src = subprocess.run(["git", "show", "HEAD:src/main.c"], cwd=ROOT, capture_output=True, text=True, check=True).stdout
line = 'INCLUDE_ASM("asm/funcs", %s);' % FN
assert src.count(line) == 1
src = src.replace(line, (ROOT / variant).read_text())
src, n = re.subn(r"const u32 jtbl_800164[68]0\[\d\] = \{.*?\};\n", "", src, flags=re.S)
assert n == 2, n
(out / "main.c").write_text(src)
cmd = "%s %s %s main.c" % (cfg.CPP, cfg.CPP_FLAGS.replace("-Iinclude", "-I" + shlex.quote(str(ROOT / "include"))), cfg.CPP_DEFS)
r = subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True)
(out / "in.i").write_text(r.stdout)
cc1 = str(ROOT / env.get("CC1", "tools/gcc-2.7.2/build/cc1"))
cmd = "%s %s -da in.i -o in.s" % (shlex.quote(cc1), cfg.CC_FLAGS)
r = subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True, env=env)
(out / "stderr.txt").write_text(r.stderr); (out / "cmd.txt").write_text(cmd + "\n")
s = (out / "in.s").read_text().splitlines()
st = next(i for i, l in enumerate(s) if l.startswith(FN + ":"))
en = next(i for i in range(st, len(s)) if s[i].strip().startswith(".end\t" + FN))
(out / "func.s").write_text("\n".join(s[st:en + 1]) + "\n")
for p in ("rtl", "jump", "cse", "loop", "cse2", "flow", "combine", "sched", "lreg", "greg", "sched2", "jump2", "dbr"):
    f = out / ("in.i." + p)
    if not f.exists():
        continue
    t = f.read_text()
    i = t.find(";; Function " + FN)
    j = t.find(";; Function", i + 10)
    (out / ("f." + p)).write_text(t[i:j] if j > 0 else t[i:])
for p in out.glob("in.i.*"):
    p.unlink()
print("wrote", out)
