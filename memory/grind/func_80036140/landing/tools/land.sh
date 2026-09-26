#!/bin/bash
# Apply the whole func_80036140 landing to the REAL tree (run ONLY while holding the landing lock).
# Saves the commit-1 (maspsx gate) versions of the two files commit 2 also edits.
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140
test -z "$(git status --porcelain --untracked-files=no -- Makefile engine include src asm bb2.ld tools .claude $(ls *.txt))" \
  || { echo "tree not clean in landing paths:"; git status --short -- Makefile engine include src asm bb2.ld tools .claude $(ls *.txt); exit 1; }
git rev-parse HEAD > $H/land_rev.txt
python3 $H/gate.py .
python3 $H/register.py .
mkdir -p $H/c1 && cp Makefile $H/c1/Makefile && cp engine/buildconfig.py $H/c1/buildconfig.py
python3 $H/apply_model.py . --split final --merge --ext rec
python3 $H/symfiles.py .
python3 - <<'EOF'
from pathlib import Path
p = Path('.claude/rules/maspsx-gate-lists.md'); s = p.read_text(encoding='utf-8')
old = 'prefill-label (fidelity): main — first and only entry (owner ruling 2026-09-04).\n'
assert s.count(old) == 1
s = s.replace(old, old + 'comm (fidelity): cdrom_SetMix (g_cd_atv), func_80035F78 (D_800A36B8), func_80036140\n'
                   '(g_cd_atv, D_800A36B8) — the first rows, landed with func_80036140 (2026-09-26).\n')
p.write_bytes(s.encode('utf-8'))
EOF
git status --short | grep -v metrics/
