#!/bin/bash
# Apply naming wave74 (functions) in the lane-naming worktree (naming-bar steps 3-4). Stops at the first failure.
set -eu
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/lane-naming
source .venv/bin/activate
T="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w74"
START=$(git rev-parse HEAD); echo "start $START"
W=docs/naming/phase3/wave74
python3 docs/naming/build_census.py > /dev/null
git add $W docs/naming/function-names.csv
git commit -q -m "wip: wave74 manifest + census (temporary, squashed into the wave commit)"
ADDRS=$(python3 -c "import csv;print(','.join(r['addr'] for r in csv.DictReader(open('$W/func_manifest.csv'))))")
python3 tools/naming_wave.py --from-census --only "$ADDRS" --apply --manifest "$T/fn.json" > "$T/naming_wave.out" 2>&1 || { tail -20 "$T/naming_wave.out"; git reset -q --soft "$START"; exit 1; }
grep -E "FATAL|APPLIED|REJECT" "$T/naming_wave.out" || true
git reset -q --soft "$START"
python3 tools/naming_keycheck.py --from-manifests "$T/fn.json" --sub-comments > "$T/subcomments.out" 2>&1; tail -3 "$T/subcomments.out"
python3 tools/format.py $(git diff --cached --name-only -- 'src/*.c' 'include/*.h'; git diff --name-only -- 'src/*.c' 'include/*.h') | tail -1
echo "applied; next: verify-oracle --rebuild, gates, keycheck"
