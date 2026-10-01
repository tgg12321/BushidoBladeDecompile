#!/usr/bin/env python3
"""Make land.py / land2.py / land3.py honour L1_ROOT (scratch tree) while reading probe bodies from the repo."""
from pathlib import Path

repo = Path(__file__).resolve().parents[2]
ENV = ('import os\nSRCROOT = Path(__file__).resolve().parents[2]\n'
       'root = Path(os.environ.get("L1_ROOT") or SRCROOT)\n')
for name in ("land.py", "land2.py", "land3.py"):
    p = repo / "tmp/func_80021424" / name
    s = p.read_bytes().decode()
    if "L1_ROOT" in s and name != "land3.py":
        continue
    s = s.replace("root = Path(__file__).resolve().parents[2]\n", ENV, 1)
    s = s.replace('d = root / "tmp/func_80021424"', 'd = SRCROOT / "tmp/func_80021424"')
    s = s.replace('bodies = (d := root / "tmp/func_80021424")', 'bodies = (d := SRCROOT / "tmp/func_80021424")')
    s = s.replace('(root / "tmp/func_80021424/func_80021424.r3.c")', '(SRCROOT / "tmp/func_80021424/func_80021424.r3.c")')
    s = s.replace('patch = subprocess.run(', 'if os.environ.get("L1_ROOT"):\n    sys.exit(0)\npatch = subprocess.run(')
    p.write_bytes(s.encode())
    print("updated", name)
