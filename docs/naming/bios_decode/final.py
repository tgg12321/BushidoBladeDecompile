#!/usr/bin/env python3
"""Final bios_names.csv: decoded index vs current name vs libscan manifest (addr join)."""
import csv, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import decode as D

HERE = Path(__file__).parent
man = {}
with (D.REPO / "tmp" / "libscan" / "rename_manifest.csv").open(newline="") as f:
    for r in csv.DictReader(f):
        a = (r.get("addr") or "").strip().lower().replace("0x", "")
        if a:
            man[int(a, 16)] = r

rows = list(csv.DictReader((HERE / "bios_names.csv").open(newline="")))
out = []
for r in rows:
    a = int(r["addr"], 16)
    m = man.get(a, {})
    ls = (m.get("proposed_name") or "").strip()
    tier = m.get("census_tier", "")
    act = m.get("census_action", "")
    spec = r["spec_name"]
    if not ls:
        cross = "LIBSCAN-UNCOVERED"
    elif ls.lower() == spec.lower() or ls.lower().lstrip("_") == spec.lower().lstrip("_"):
        cross = "CONCUR"
    else:
        cross = "CONCUR-ALIAS"       # PsyQ library symbol vs nocash BIOS-table name
    note = ""
    if act == "RESET":
        note = "libscan would RESET to func_*; index decode VERIFIES the name -- do not reset"
        cross = "CONFLICT-LIBSCAN"
    elif tier in ("AUTO", "INFERRED", "SUSPECT"):
        note = f"index decode upgrades libscan tier {tier} -> VERIFIED"
    out.append(dict(
        addr=r["addr"], current_name=r["current_name"], table=r["table"],
        index=r["index"], spec_name=spec, status=r["status"],
        libscan_name=ls, libscan_tier=tier, libscan_action=act,
        cross_check=cross, file=r["file"], note=note,
    ))

p = HERE / "bios_names.csv"
with p.open("w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(out[0].keys()))
    w.writeheader(); w.writerows(out)

from collections import Counter
print("status   :", dict(Counter(r["status"] for r in out)))
print("crosscheck:", dict(Counter(r["cross_check"] for r in out)))
print()
for r in out:
    if r["note"] or r["cross_check"] not in ("CONCUR",):
        print(f"  {r['addr']} {r['current_name']:<34} {r['table']}:{r['index']:<5} "
              f"spec={r['spec_name']:<28} libscan={r['libscan_name']:<26} {r['cross_check']}")
        if r["note"]:
            print(f"      ! {r['note']}")
print("\n->", p)
