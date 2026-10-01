#!/usr/bin/env python3
"""aspsx_hilo_syntax.py: which spellings of an explicit high/low address split does Sony ASPSX 2.34 accept,
and, for a small symbol the file DEFINES (.comm / .lcomm / .sdata), does it keep the explicit lui + %lo pair
as written or rewrite the access to gp?  (per-file-gp-model.md explicit-relocation clause)
usage (WSL, repo root): python3 tmp/q56/aspsx_hilo_syntax.py -> tmp/q56/aspsx_hilo_results.txt"""
import subprocess, sys
from pathlib import Path
sys.path.insert(0, "tmp/q56")
import psyqobj
from aspsx_probes import dec  # noqa: E402  (reuses the word decoder; importing runs no probes? see guard)

HERE = Path("tmp/q56/probes/hilo"); HERE.mkdir(parents=True, exist_ok=True)
DEFS = {"comm": ["\t.comm\tsx,4"], "lcomm": ["\t.lcomm\tsx,4"], "sdata": ["\t.sdata", "sx:", "\t.word\t0"],
        "extern": ["\t.extern\tsx,4"]}
SPELL = {
    "pct_hi_lo": ["\tlui\t$4,%hi(sx)", "\tlw\t$4,%lo(sx)($4)"],
    "shift_mask": ["\tlui\t$4,(sx>>16)", "\tlw\t$4,(sx&$ffff)($4)"],
    "shift_mask_adj": ["\tlui\t$4,((sx+$8000)>>16)", "\tlw\t$4,(sx&$ffff)($4)"],
    "sn_hi_lo": ["\tlui\t$4,sx>>16", "\tlw\t$4,sx&$ffff($4)"],
    "la_then_lw": ["\tla\t$4,sx", "\tlw\t$4,0($4)"],
}
out = []
for dname, dl in DEFS.items():
    for sname, sl in SPELL.items():
        s = HERE / f"{dname}_{sname}.s"; o = HERE / f"{dname}_{sname}.obj"
        s.write_text("\n".join(dl + ["\t.text"] + sl) + "\n")
        o.unlink(missing_ok=True)
        r = subprocess.run(["bash", "tmp/research36140/aspsx.sh", str(s), str(o), "-G8"], capture_output=True, text=True)
        err = [l.strip() for l in r.stderr.splitlines() if "Error" in l]
        if err or not o.exists():
            out.append(f"{dname:6} {sname:15}: REJECTED {err[:1]}")
            continue
        t = psyqobj.read_text(o.read_bytes())
        ws = [int.from_bytes(t[i:i + 4], "little") for i in range(0, len(t), 4)]
        out.append(f"{dname:6} {sname:15}: " + " | ".join(dec(w) for w in ws))
open("tmp/q56/aspsx_hilo_results.txt", "w", newline="\n").write("\n".join(out) + "\n")
print("\n".join(out))
