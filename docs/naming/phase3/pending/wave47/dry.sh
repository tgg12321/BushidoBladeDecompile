#!/bin/bash
# wave47 dry run: census with the manifest, naming_wave + data_wave dry runs, then restore the census file.
set -u
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-b
source .venv/bin/activate
T="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w47"
W=docs/naming/phase3/wave47
python3 docs/naming/build_census.py > /dev/null
ADDRS=$(python3 -c "import csv;print(','.join(r['addr'] for r in csv.DictReader(open('$W/func_manifest.csv'))))")
DADDRS=$(python3 -c "import csv;print(','.join(r['addr'] for r in csv.DictReader(open('$W/data_manifest.csv'))))")
{
  echo "== naming_wave --from-census --only $ADDRS"
  python3 tools/naming_wave.py --from-census --only "$ADDRS"; echo "rc=$?"
  echo "== data_wave --manifest-csv $W/data_manifest.csv --only $DADDRS"
  python3 tools/data_wave.py --manifest-csv $W/data_manifest.csv --only "$DADDRS"; echo "rc=$?"
} > "$T/dry.txt" 2>&1
git checkout -- docs/naming/function-names.csv
head -60 "$T/dry.txt"
