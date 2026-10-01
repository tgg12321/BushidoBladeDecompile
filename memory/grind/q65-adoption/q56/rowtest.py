#!/usr/bin/env python3
"""rowtest.py — Q56 step 1: per-row liveness of sdata_exclude.txt.

Runs inside WSL against the scratch copy /tmp/q56/tree (baseline built by setup.sh,
reference objects in /tmp/q56/refobj, reference bin /tmp/q56/ref.bin).

For each variant (a row removed, a function's rows removed, one symbol removed from a row)
it builds ONLY the owning TU with the Makefile's own recipe (make with BUILD_DIR and the two
MASPSX flag variables overridden so --sdata-exclude points at the variant file) and compares
the object with the reference object. Differing objects are then relinked (copy of the
reference build dir with the object swapped, same ld/objcopy commands as the Makefile) and
the .bin compared with the reference .bin.

Output: /tmp/q56/results.json
"""
import json, os, re, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

T = "/tmp/q56/tree"
Q = "/tmp/q56"
REF = Q + "/refobj"

def sh(cmd, **kw):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True, **kw)

mk = open(T + "/Makefile").read()
MF = re.search(r"^MASPSX_FLAGS :=(.*)$", mk, re.M).group(1).strip()
MFG = re.search(r"^MASPSX_FLAGS_GP :=(.*)$", mk, re.M).group(1).strip()
assert "--sdata-exclude=sdata_exclude.txt" in MF and "--sdata-exclude=sdata_exclude.txt" in MFG

lines = open(T + "/sdata_exclude.txt").read().split("\n")
rows = []  # (lineno 1-based, text, func, [syms])
for i, l in enumerate(lines, 1):
    s = l.strip()
    if not s or s.startswith("#") or ":" not in s:
        continue
    f, syms = s.split(":", 1)
    rows.append((i, l, f.strip(), [x.strip() for x in syms.split(",") if x.strip()]))

# func -> TU (defined text symbol in a C object)
func_tu = {}
for o in sorted(os.listdir(REF)):
    out = sh(f"mipsel-linux-gnu-nm --defined-only {REF}/{o}").stdout
    for ln in out.splitlines():
        p = ln.split()
        if len(p) == 3 and p[1] in "Tt":
            func_tu.setdefault(p[2], []).append(o[:-2])

sdata_funcs = set(x.strip() for x in open(T + "/sdata_funcs.txt") if x.strip() and not x.startswith("#"))
sdata_syms = set(x.strip() for x in open(T + "/sdata_syms.txt") if x.strip() and not x.startswith("#"))

def is_include_asm(func, tu):
    src = open(f"{T}/src/{tu}.c", errors="replace").read()
    return re.search(r"INCLUDE_ASM\(\s*\"[^\"]*\"\s*,\s*%s\s*\)" % re.escape(func), src) is not None

def build_variant(tag, tu, excl_text):
    d = f"{Q}/b/{tag}"
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d + "/src")
    ex = f"{d}/sdata_exclude.txt"
    open(ex, "w", newline="\n").write(excl_text)
    mf = MF.replace("--sdata-exclude=sdata_exclude.txt", f"--sdata-exclude={ex}")
    mfg = MFG.replace("--sdata-exclude=sdata_exclude.txt", f"--sdata-exclude={ex}")
    r = subprocess.run(["make", "-C", T, f"BUILD_DIR={d}", f"MASPSX_FLAGS={mf}", f"MASPSX_FLAGS_GP={mfg}",
                        f"{d}/src/{tu}.o"], capture_output=True, text=True)
    obj = f"{d}/src/{tu}.o"
    if r.returncode != 0 or not os.path.exists(obj):
        return {"built": False, "err": (r.stdout + r.stderr)[-1500:]}
    same = open(obj, "rb").read() == open(f"{REF}/{tu}.o", "rb").read()
    return {"built": True, "obj_identical": same, "obj": obj}

def relink(tag, tu, obj):
    L = f"{Q}/link/{tag}"
    shutil.rmtree(L, ignore_errors=True)
    os.makedirs(L)
    shutil.copytree(T + "/build", L + "/build", symlinks=True)
    shutil.copy(obj, f"{L}/build/src/{tu}.o")
    for f in ("bb2.elf", "bb2.bin", "bb2.exe", "bb2.map"):
        if os.path.exists(f"{L}/build/{f}"):
            os.remove(f"{L}/build/{f}")
    cmd = (f"cd {L} && mipsel-linux-gnu-ld -nostdlib --no-check-sections -Map build/bb2.map -T {T}/bb2.ld "
           f"-T {T}/undefined_funcs_auto.txt -T {T}/undefined_syms_auto.txt -T {T}/named_syms.txt -o build/bb2.elf "
           f"&& mipsel-linux-gnu-objcopy -O binary -j .main build/bb2.elf build/bb2.bin")
    r = sh(cmd)
    if r.returncode != 0:
        return {"linked": False, "err": (r.stdout + r.stderr)[-1500:]}
    same = open(f"{L}/build/bb2.bin", "rb").read() == open(Q + "/ref.bin", "rb").read()
    return {"linked": True, "bin_identical": same}

def funcdiff(tu, obj, func):
    """objdump -dr of func in ref vs variant; returns unified diff text."""
    def dis(o):
        out = sh(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases {o}").stdout
        m = re.search(r"^[0-9a-f]+ <%s>:\n(.*?)(?=^[0-9a-f]+ <|\Z)" % re.escape(func), out, re.M | re.S)
        return m.group(1) if m else ""
    a, b = dis(f"{REF}/{tu}.o"), dis(obj)
    import difflib
    al = [re.sub(r"^\s*[0-9a-f]+:\s*", "", x) for x in a.splitlines()]
    bl = [re.sub(r"^\s*[0-9a-f]+:\s*", "", x) for x in b.splitlines()]
    return {"ref_lines": len(al), "var_lines": len(bl),
            "diff": "\n".join(difflib.unified_diff(al, bl, "ref", "variant", n=1, lineterm=""))}

def excl_without(skip_linenos=(), sym_drop=None):
    out = []
    for i, l in enumerate(lines, 1):
        if i in skip_linenos:
            continue
        if sym_drop and i == sym_drop[0]:
            f, syms = l.split(":", 1)
            keep = [s.strip() for s in syms.split(",") if s.strip() and s.strip() != sym_drop[1]]
            if not keep:
                continue
            l = f + ": " + ", ".join(keep)
        out.append(l)
    return "\n".join(out)

def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else "all"
    tasks = []
    info = {}
    for (n, text, func, syms) in rows:
        tus = func_tu.get(func, [])
        info[n] = {"line": n, "text": text, "func": func, "syms": syms, "tus": tus,
                   "in_sdata_funcs": func in sdata_funcs,
                   "syms_in_sdata_syms": {s: (s in sdata_syms) for s in syms},
                   "include_asm": (is_include_asm(func, tus[0]) if len(tus) == 1 else None)}
        if len(tus) == 1:
            tasks.append((f"r{n}", tus[0], excl_without({n}), ("row", n, func, None)))
    if mode == "control":
        tus = sorted(set(t[1] for t in tasks))
        ctl = [(f"ctl_{tu}", tu, "\n".join(lines), ("ctl", 0, None, None)) for tu in tus]
        with ThreadPoolExecutor(12) as ex:
            res = list(ex.map(lambda t: (t[0], build_variant(t[0], t[1], t[2])), ctl))
        for tag, r in res:
            print(tag, r.get("obj_identical"), r.get("err", "")[:200])
        return
    # functions with several rows: remove all of them
    byf = {}
    for (n, text, func, syms) in rows:
        byf.setdefault(func, []).append(n)
    for func, ns in byf.items():
        if len(ns) > 1 and len(func_tu.get(func, [])) == 1:
            tasks.append((f"f_{func}", func_tu[func][0], excl_without(set(ns)), ("func", ns, func, None)))
    with ThreadPoolExecutor(12) as ex:
        res = list(ex.map(lambda t: (t, build_variant(t[0], t[1], t[2])), tasks))
    results = {"rows": info, "row_tests": {}, "func_tests": {}, "sym_tests": {}}
    live_rows = []
    for (tag, tu, _, meta), r in res:
        kind, n, func, _ = meta
        rec = {"tu": tu, **{k: v for k, v in r.items() if k != "obj"}}
        if r.get("built") and not r["obj_identical"]:
            rec.update(relink(tag, tu, r["obj"]))
            rec["funcdiff"] = funcdiff(tu, r["obj"], func)
        if kind == "row":
            results["row_tests"][n] = rec
            if r.get("built") and not r["obj_identical"]:
                live_rows.append(n)
        else:
            results["func_tests"][func] = rec
    # per-symbol tests on live rows with >1 symbol
    stasks = []
    for n in live_rows:
        _, text, func, syms = next(x for x in rows if x[0] == n)
        if len(syms) > 1:
            for s in syms:
                stasks.append((f"s{n}_{s}", func_tu[func][0], excl_without(sym_drop=(n, s)), (n, s, func)))
    with ThreadPoolExecutor(12) as ex:
        sres = list(ex.map(lambda t: (t, build_variant(t[0], t[1], t[2])), stasks))
    for (tag, tu, _, (n, s, func)), r in sres:
        rec = {"tu": tu, **{k: v for k, v in r.items() if k != "obj"}}
        if r.get("built") and not r["obj_identical"]:
            rec["funcdiff"] = funcdiff(tu, r["obj"], func)
        results["sym_tests"].setdefault(str(n), {})[s] = rec
    json.dump(results, open(Q + "/results.json", "w"), indent=1)
    print("rows", len(rows), "live", len(live_rows), "->", live_rows)

main()
