#!/usr/bin/env python3
# Q116 (fspu1): _SpuSetAnyVoice (libspu s_sav.c) reads and writes the SPU request shadow as psyz@4e4b3e8d's
# s_sav.c does, `_spu_RQ[reg - 0xC4]` (BB2's `extern volatile u16 _spu_RQ[10];`, libspu_internal.h). The
# D_800F7298 extern (a phantom base, 0x800F7298 + 0xC4 * 2 = _spu_RQ) and its SpuUnion view go, as do the
# no-op (SpuUnion *) casts on _spu_RXX.
# usage: fspu1.py   writes tmp/p2/fspu/out/ (scratch only); BASE_REV env (default a6939b16f, ":" = index)
import os, subprocess
NL = chr(10)
OUT = "tmp/p2/fspu/out/"
BASE_REV = os.environ.get("BASE_REV", "a6939b16f")

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, (a[:70], t.count(a))
        t = t.replace(a, b)
    return t

def s_sav(t):
    t = rep(t, [("""/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern SpuUnion D_800F7298;

""", "")])
    for a in ("addr1", "addr2"):
        assert t.count("D_800F7298.raw[%s]" % a) == 4, a
        t = t.replace("D_800F7298.raw[%s]" % a, "_spu_RQ[%s - 0xC4]" % a)
    assert t.count("((SpuUnion *)_spu_RXX)->raw[") == 8
    t = t.replace("((SpuUnion *)_spu_RXX)->raw[", "_spu_RXX->raw[")
    assert "D_800F7298" not in t and "SpuUnion" not in t
    return t

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {"s_sav.c": s_sav(show("src/main/psxsdk/libspu/s_sav.c"))}
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    for h in ("game.h", "bb2.h"):
        open(OUT + h, "w", encoding="utf-8", newline=NL).write(show("include/" + h))
    return out

if __name__ == "__main__":
    write()
    print("wrote fspu1")
