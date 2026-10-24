#!/bin/bash
# wave78 dry run: census with the manifest, naming_wave dry run, then restore the census file.
set -u
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-a
source .venv/bin/activate
T="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w78"
W=docs/naming/phase3/wave78
python3 docs/naming/build_census.py > /dev/null
ADDRS=$(python3 -c "import csv;print(','.join(r['addr'] for r in csv.DictReader(open('$W/func_manifest.csv'))))")
python3 tools/naming_wave.py --from-census --only "$ADDRS" > "$T/dry.txt" 2>&1; echo "rc=$?"
git checkout -- docs/naming/function-names.csv
head -40 "$T/dry.txt"
