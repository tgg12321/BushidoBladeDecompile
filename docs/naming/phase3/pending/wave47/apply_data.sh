#!/bin/bash
# Apply naming wave47 (data) in the ln-naming-b worktree (naming-bar steps 3-4). Stops at the first failure.
set -eu
cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-b
source .venv/bin/activate
T="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w47"
W=docs/naming/phase3/wave47
python3 tools/data_wave.py --manifest-csv $W/data_manifest.csv --apply --manifest "$T/dw.json" > "$T/data_wave.out" 2>&1 || { tail -20 "$T/data_wave.out"; exit 1; }
grep -E "FATAL|APPLIED|DATA WAVE" "$T/data_wave.out" || true
python3 tools/naming_keycheck.py --from-manifests "$T/dw.json" --sub-comments > "$T/subcomments.out" 2>&1; tail -3 "$T/subcomments.out"
python3 tools/format.py $(git diff --name-only -- 'src/*.c' 'include/*.h') | tail -1
echo "applied; next: verify-oracle --rebuild, gates, keycheck"
