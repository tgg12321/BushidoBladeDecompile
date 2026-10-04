#!/bin/bash
# all.sh : regenerate every Phase 2 census output into memory/grind/phase2-2026-10-03/ (run in WSL).
set -e
cd "$(dirname "$0")/../../../.."
source .venv/bin/activate
T=memory/grind/phase2-2026-10-03/tools
O=memory/grind/phase2-2026-10-03
mkdir -p tmp/p2
python3 $T/census.py
python3 $T/hoist.py --rejects > /dev/null
python3 $T/conflicts.py
python3 $T/refs.py
python3 $T/proto_trial.py
python3 $T/worklist.py $O
cp tmp/p2/phase2_conflicts.tsv tmp/p2/proto_trial.tsv $O/
python3 $T/casts.py $O
python3 $T/protos.py $O
