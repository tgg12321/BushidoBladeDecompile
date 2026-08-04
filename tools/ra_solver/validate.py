#!/usr/bin/env python3
"""ra_solver.validate — batch ground-truth validation of the simulator."""
import subprocess, sys
from pathlib import Path

HERE = Path(__file__).parent
CASES = [
    ("hirahira_w_ctrl", "text1a"),
    ("saTan4FireDisp", "text1a"),
    ("tslGlobalMemFree_800861BC", "main"),
    ("title_mv_exec2", "main"),
    ("func_8007C7A0", "display"),
    ("func_8007C86C", "display"),
    ("func_8007CE0C", "display"),
    # completed controls
    ("func_8003D9A0", "code6cac_c2"),
    ("func_8003DBE4", "code6cac_c2"),
    ("tslLineG5Init", "code6cac_c2"),
]

ok = bad = 0
for func, stem in CASES:
    r1 = subprocess.run([sys.executable, HERE / "extract.py", func, stem],
                        capture_output=True, text=True)
    if r1.returncode != 0:
        print(f"EXTRACT-FAIL {func}: {r1.stdout.strip()[-120:]} {r1.stderr.strip()[-120:]}")
        bad += 1
        continue
    model = Path("tmp/ra_solver_work") / f"{func}.model.json"
    r2 = subprocess.run([sys.executable, HERE / "simulate.py", model],
                        capture_output=True, text=True)
    tail = [l for l in r2.stdout.splitlines() if "dispositions:" in l or "sort order" in l]
    status = "PASS" if r2.returncode == 0 else "FAIL"
    if r2.returncode == 0:
        ok += 1
    else:
        bad += 1
    print(f"{status} {func:30s} {' | '.join(tail)}")
    if r2.returncode != 0:
        for l in r2.stdout.splitlines():
            if "XX" in l or "MISMATCH" in l or l.startswith("  sim") or l.startswith("  dump"):
                print("   ", l)
print(f"\n{ok}/{ok+bad} functions validated exactly")
sys.exit(0 if bad == 0 else 1)
