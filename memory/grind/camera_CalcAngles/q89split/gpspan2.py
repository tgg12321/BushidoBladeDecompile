"""gpspan2.py <head.c> <tail.c>: %gp_rel symbols of the ORIGINAL code (asm/funcs) of every function in each
part; prints symbols reached from both parts and per-part sets. Function names: C definitions (ANSI and
K&R headers) and INCLUDE_ASM lines."""
import re, os, glob, sys

label_file = {}
for p in glob.glob("asm/funcs/*.s"):
    for line in open(p, encoding="utf-8", errors="replace"):
        m = re.match(r"\s*glabel\s+(\w+)", line)
        if m:
            label_file.setdefault(m.group(1), p)
for line in open("named_syms.txt", encoding="utf-8", errors="replace"):
    m = re.match(r"\s*(\w+?)_([0-9A-F]{8})\s*=\s*0x([0-9A-Fa-f]{8})", line)
    if m and m.group(1) not in label_file:
        q = "asm/funcs/func_%s.s" % m.group(3).upper()
        if os.path.exists(q):
            label_file[m.group(1)] = q
# legacy names resolved by hand in the camera ledger (g0proof/gpspan.out)
label_file.setdefault("stage_GetId", "asm/funcs/func_80046798.s")
label_file.setdefault("snd_AllocSe", "asm/funcs/func_80046934.s")
label_file.setdefault("snd_SeNullCallback", "asm/funcs/func_80046954.s")

def funcs(path):
    import subprocess
    o = subprocess.run(["wsl", "mipsel-linux-gnu-nm", "-n", path], capture_output=True, text=True).stdout
    return [l.split()[2] for l in o.splitlines() if len(l.split()) == 3 and l.split()[1] in "Tt" and not l.split()[2].startswith(".")]

res = {}
for side, path in (("head", sys.argv[1]), ("tail", sys.argv[2])):
    syms, missing = {}, []
    fl = funcs(path)
    for f in fl:
        p = label_file.get(f)
        if not p:
            missing.append(f); continue
        for s in set(re.findall(r"%gp_rel\(([\w.]+)", open(p, encoding="utf-8", errors="replace").read())):
            syms.setdefault(s, []).append(f)
    res[side] = syms
    print("%s: %d functions (%s .. %s); unresolved: %s" % (side, len(fl), fl[0], fl[-1], missing))
both = sorted(set(res["head"]) & set(res["tail"]))
print("gp symbols reached from BOTH parts:", both or "none")
for side in ("head", "tail"):
    print("%s gp symbols: %s" % (side, sorted(res[side])))
