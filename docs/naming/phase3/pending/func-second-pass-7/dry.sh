#!/bin/bash
# wave98 dry run. The manifest's verdict is PENDING (the reviewer sets CONFIRM later) and the census only
# reads CONFIRMed rows, so stage a CONFIRM copy under phase3/zz-wave98-dry/, run the census with it, the
# naming_wave dry run, then restore the census file and remove the staged copy. Needs build.py run first.
set -u
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-a
source .venv/bin/activate
T="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w98"
W=docs/naming/phase3/wave98
S=docs/naming/phase3/zz-wave98-dry
mkdir -p "$T" $S
python3 - "$W/func_manifest.csv" "$S/func_manifest.csv" <<'PY'
import csv, sys
rows = list(csv.reader(open(sys.argv[1], encoding="utf-8", newline="")))
v = rows[0].index("verdict")
for r in rows[1:]:
    r[v] = "CONFIRM"
with open(sys.argv[2], "w", encoding="utf-8", newline="") as fh:
    csv.writer(fh, lineterminator="\n").writerows(rows)
PY
python3 docs/naming/build_census.py > /dev/null
ADDRS=$(python3 -c "import csv;print(','.join(r['addr'] for r in csv.DictReader(open('$W/func_manifest.csv'))))")
python3 tools/naming_wave.py --from-census --only "$ADDRS" > "$T/dry.txt" 2>&1; echo "rc=$?"
git checkout -- docs/naming/function-names.csv
rm $S/func_manifest.csv && rmdir $S
cat "$T/dry.txt"
