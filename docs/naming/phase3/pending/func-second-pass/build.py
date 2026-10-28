"""wave92 manifests: func_manifest.csv (rows.py FUNCS; func-second-pass draft) and
data_manifest.csv (none). HOMES / USES enumerated live as
tmp/p3w46/build.py. Writes <WT>/docs/naming/phase3/wave92/."""
import csv
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rows import FUNCS, DATA, VERDICT, VERIFIER_NOTE, NOTES  # noqa: E402

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")
OUT = os.path.join(WT, "docs", "naming", "phase3", "wave92")
HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(WT)
BASE = os.popen("git rev-parse --short=9 HEAD").read().strip()
TREE = {f: open(f, encoding="utf-8", errors="replace").read() for f in
        glob.glob("src/**/*.[ch]", recursive=True) + glob.glob("include/**/*.h", recursive=True)}


def fresh(new):
    if any(re.search(r"\b%s\b" % new, t) for t in TREE.values()):
        raise SystemExit(f"{new} not fresh")


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
            tail = "" if reset else f" FRESH: {new} occurs nowhere in src/ or include/."
            ev2 = (f"HOMES / USES ({len(h)}): " + "; ".join(h) + ". " + ev + tail +
                   f" Census: {c['tier']} ({c['origin']}). Cites at {BASE}.")
            note = NOTES.get(addr, VERIFIER_NOTE)
            w.writerow([addr, c["glabel"], c["current_name"], new, action,
                        "reset-contradicted" if reset else "sotn-review", ev2,
                        VERDICT, note])
    with open(os.path.join(OUT, "data_manifest.csv"), "w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["addr", "current_names", "proposed_name", "kind", "evidence", "verdict", "verifier_note"])
        for addr, new, ev in DATA:
            fresh(new)
            old = "D_" + addr[2:].upper()
            c = [f"{f.replace(os.sep, '/')}:{k}" for f, t in TREE.items()
                 for k, l in enumerate(t.split("\n"), 1) if re.search(r"\b%s\b" % old, l)]
            w.writerow([addr, old, new, "rename",
                        f"C SITES ({len(c)}): {'; '.join(c)}. " + ev +
                        f" FRESH: {new} occurs nowhere in src/ or include/. Cites at {BASE}.",
                        "CONFIRM", "naming-reviewer pre-apply PASS (round 1)"])
    print(len(FUNCS), "func rows,", len(DATA), "data rows ->", OUT)


if __name__ == "__main__":
    main()
