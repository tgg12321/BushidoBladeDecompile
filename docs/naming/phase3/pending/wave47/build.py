"""wave47 manifests: func_manifest.csv (rows.py FUNCS) and data_manifest.csv (rows.py DATA).
HOMES / USES enumerated live (src, include, asm/funcs, registries) as wave77/build.py; data rows
also list their asm sites. Writes <WT>/docs/naming/phase3/wave47/ (LF)."""
import csv
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rows import FUNCS, DATA, VERDICT, VERIFIER_NOTE  # noqa: E402

VERDICT = os.environ.get("W47_DRY_VERDICT", VERDICT)  # dry run only; the committed manifest keeps VERDICT empty
VERIFIER_NOTE = os.environ.get("W47_DRY_NOTE", VERIFIER_NOTE)
WT =os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")
OUT = os.path.join(WT, "docs", "naming", "phase3", "wave47")
os.chdir(WT)
BASE = os.popen("git rev-parse --short=9 HEAD").read().strip()
FRESH_FILES = (glob.glob("src/**/*.[ch]", recursive=True) + glob.glob("include/**/*.h", recursive=True) +
               glob.glob("tools/**/*.py", recursive=True) + glob.glob("engine/**/*.py", recursive=True) +
               glob.glob("tools/**/*.txt", recursive=True) + glob.glob("*.txt") + glob.glob("*.ld") +
               glob.glob("asm/**/*.s", recursive=True))
TREE = {f: open(f, encoding="utf-8", errors="replace").read() for f in FRESH_FILES}
CTREE = {f: t for f, t in TREE.items() if f.startswith(("src", "include"))}


def fresh(new):
    for f, t in TREE.items():
        if re.search(r"\b%s\b" % new, t):
            raise SystemExit(f"{new} not fresh ({f})")


def homes(addr, names):
    out = []
    for f in sorted(glob.glob("src/main/*.c")) + sorted(glob.glob("include/*.h")):
        for k, l in enumerate(open(f, encoding="utf-8"), 1):
            if any(re.search(r"\b%s\b" % n, l) for n in names):
                kind = "def" if re.match(r"^[A-Za-z_]", l) and "(" in l and (l.rstrip().endswith("{") or l.rstrip().endswith("}")) else "ref"
                out.append(f"{f.replace(os.sep, '/')}:{k} {kind}")
    for f in sorted(glob.glob("asm/funcs/*.s")):
        for k, l in enumerate(open(f, encoding="utf-8", errors="replace"), 1):
            if any(re.search(r"\b(jal|j|glabel)\s+%s\b" % n, l) for n in names):
                out.append(f"{f.replace(os.sep, '/')}:{k} " + ("glabel" if "glabel" in l else "jal"))
    for f in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt"):
        for k, l in enumerate(open(f, encoding="utf-8", errors="replace"), 1):
            if re.search(addr[2:], l, re.I):
                out.append(f"{f}:{k} registry")
    return out


def main():
    census = {r["address"].lower(): r for r in csv.DictReader(open("docs/naming/function-names.csv", encoding="utf-8"))}
    os.makedirs(OUT, exist_ok=True)
    cols = ["addr", "glabel", "current_name", "proposed_name", "action", "evidence_class", "evidence", "verdict", "verifier_note"]
    with open(os.path.join(OUT, "func_manifest.csv"), "w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(cols)
        for addr, new, action, ev in FUNCS:
            c = census[addr.lower()]
            reset = action == "RESET"
            if reset:
                assert new == "func_" + addr[2:].upper(), (addr, new)
            else:
                fresh(new)
            h = homes(addr, {"func_" + addr[2:].upper(), c["current_name"]})
            tail = "" if reset else f" FRESH: {new} occurs nowhere in src/, include/, tools/, engine/ or the linker registries."
            ev2 = (f"HOMES / USES ({len(h)}): " + "; ".join(h) + ". " + ev + tail +
                   f" Census: {c['tier']} ({c['origin']}). Cites at {BASE}.")
            w.writerow([addr, c["glabel"], c["current_name"], new, action,
                        "reset-contradicted" if reset else "sotn-review", ev2,
                        VERDICT, VERIFIER_NOTE])
    with open(os.path.join(OUT, "data_manifest.csv"), "w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["addr", "current_names", "proposed_name", "kind", "evidence", "verdict", "verifier_note"])
        for addr, new, ev in DATA:
            fresh(new)
            old = "D_" + addr[2:].upper()
            c = [f"{f.replace(os.sep, '/')}:{k}" for f, t in sorted(CTREE.items())
                 for k, l in enumerate(t.split("\n"), 1) if re.search(r"\b%s\b" % old, l)]
            a = [f"{f.replace(os.sep, '/')}:{k}" for f in sorted(glob.glob("asm/funcs/*.s") + glob.glob("asm/data/*.s"))
                 for k, l in enumerate(open(f, encoding="utf-8", errors="replace"), 1) if re.search(r"\b%s\b" % old, l)]
            reg = [f"{f}:{k}" for f in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt")
                   for k, l in enumerate(open(f, encoding="utf-8", errors="replace"), 1)
                   if re.search(r"^%s\b" % old, l)]
            w.writerow([addr, old, new, "rename",
                        f"C SITES ({len(c)}): {'; '.join(c)}. ASM SITES ({len(a)}): {'; '.join(a)}. "
                        f"REGISTRY DEFS: {'; '.join(reg)}. " + ev +
                        f" FRESH: {new} occurs nowhere in src/, include/, tools/, engine/ or the linker registries. Cites at {BASE}.",
                        VERDICT, VERIFIER_NOTE])
    print(len(FUNCS), "func rows,", len(DATA), "data rows ->", OUT)


if __name__ == "__main__":
    main()
