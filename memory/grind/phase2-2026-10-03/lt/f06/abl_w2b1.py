#!/usr/bin/env python3
# FAKE ablations for w2b1 (run in WSL from the repo root): each variant is the generator's output with
# one construct removed (or its plain alternative), written to tmp/w2/abl/<name>/; the touched
# function bodies go to tmp/w2/abl/<name>.<func>.c and tmp/w2/abl/list.txt lists "<name> <func> <file>"
# for run_abl_w2b1.ps1 (engine sandbox <func> --disable all --candidate <file>).
import os, re, subprocess, sys

D = ["gaks_raw", "gvex_raw", "gks_raw", "init_ptr", "rev_sotn", "de_c", "bcr_vol", "kon_plain", "snc_one"]


def without(*a):
    return [o for o in D if o not in a]


ABL = {
    # new constructs (each kept because its plain form differs)
    "init": (without("init_ptr"), ["_spu_init"]),
    "de": (without("de_c") + ["de_b"], ["SetDrawEnv", "SetDrawEnv2"]),
    "gks_sep1": (without("gks_raw") + ["gks_sep1"], ["SpuGetKeyStatus"]),
    "gks_sepall": (without("gks_raw") + ["gks_sepall"], ["SpuGetKeyStatus"]),
    # existing FAKEs in moved bodies, re-ablated
    "dealias": (D + ["a_dealias"], ["SetDrawEnv", "SetDrawEnv2"]),
    "pdvol": (D + ["a_pdvol"], ["PutDispEnv"]),
    "pdarm": (D + ["a_pdarm"], ["PutDispEnv"]),
    "dxwrap": (D + ["a_dxwrap"], ["get_dx"]),
    "svaplain": (D + ["a_svaplain"], ["func_8008B488"]),
    # SpuGetKeyStatus: typed alternatives to the debt line
    "gks_h": (without("gks_raw") + ["gks_h"], ["SpuGetKeyStatus"]),
    "gks_a": (without("gks_raw") + ["gks_a"], ["SpuGetKeyStatus"]),
}
FILE = {"_spu_init": "src/main/psxsdk/libspu/spu.c", "SpuGetVoiceEnvelope": "src/main/psxsdk/libspu/s_gvex.c",
        "SpuGetAllKeysStatus": "src/main/psxsdk/libspu/sr_gaks.c", "SpuRGetAllKeysStatus": "src/main/psxsdk/libspu/sr_gaks.c", "SetGraphReverse": "src/main/psxsdk/libgpu/sys.c",
        "SetDrawEnv": "src/main/psxsdk/libgpu/sys.c", "SetDrawEnv2": "src/main/psxsdk/libgpu/sys.c",
        "PutDispEnv": "src/main/psxsdk/libgpu/sys.c", "get_dx": "src/main/psxsdk/libgpu/sys.c",
        "func_8008B488": "src/main/psxsdk/libspu/7BC88.c", "SpuGetKeyStatus": "src/main/psxsdk/libspu/s_gks.c"}
GEN = "memory/grind/phase2-2026-10-03/lt/f06/w2b1.py"


def body(s, f):
    m = re.search(r"\n[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{" % f, s)
    i = m.start() + 1
    return s[i:s.index("\n}\n", i) + 3]


os.makedirs("tmp/w2/abl", exist_ok=True)
lines = []
for name, (opts, fs) in ABL.items():
    out = "tmp/w2/abl/" + name
    subprocess.run(["python3", GEN, "opt=" + ",".join(opts), "out=" + out], check=True)
    for f in fs:
        p = "tmp/w2/abl/%s.%s.c" % (name, f)
        open(p, "w", encoding="utf-8", newline="\n").write(body(open(out + "/" + FILE[f], encoding="utf-8").read(), f))
        lines.append("%s %s %s" % (name, f, p))
open("tmp/w2/abl/list.txt", "w", newline="\n").write("\n".join(lines) + "\n")
print("\n".join(lines))
