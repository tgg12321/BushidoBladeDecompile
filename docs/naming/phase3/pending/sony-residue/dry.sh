#!/bin/bash
# sony-residue dry run: stage the manifest where build_census reads it (phase3/<wave>/), census with
# the manifest, naming_wave dry run, then restore the census file and remove the staged copy.
set -u
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-a
source .venv/bin/activate
T="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3sr"
mkdir -p "$T"
D=docs/naming/phase3/pending/sony-residue
S=docs/naming/phase3/zz-sony-residue-dry
mkdir -p $S && cp docs/naming/phase3/sony-residue/func_manifest.csv $S/func_manifest.csv
python3 docs/naming/build_census.py > /dev/null
ADDRS=$(python3 -c "import csv;print(','.join(r['addr'] for r in csv.DictReader(open('$S/func_manifest.csv'))))")
python3 tools/naming_wave.py --from-census --only "$ADDRS" > "$T/dry.txt" 2>&1; echo "rc=$?"
git checkout -- docs/naming/function-names.csv
rm $S/func_manifest.csv && rmdir $S
head -60 "$T/dry.txt"
