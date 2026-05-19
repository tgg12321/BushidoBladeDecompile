#!/bin/bash
# Batch-run the cc1psx vs decompals diagnostic on a list of tier-3
# functions. For each: ensure permuter dir is set up, extract a pure-C
# base.c, run the diagnostic, collect the verdict.
#
# Usage: bash tools/diagnose_batch.sh <func1> <func2> ...
#
# Outputs CSV to stdout:
#   func,src_file,target_insns,decompals_score,cc1psx_score,category
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

find_src() {
    local func="$1"
    grep -lE "^\s*([a-zA-Z_][a-zA-Z0-9_*]*\s+)+\**\s*${func}\s*\(" src/*.c 2>/dev/null | head -1
}

echo "func,src_file,target_insns,decompals_score,cc1psx_score,category"
for func in "$@"; do
    src="$(find_src "$func")"
    if [ -z "$src" ]; then
        echo "${func},NOT_FOUND,,,,FUNC_NOT_FOUND"
        continue
    fi
    if [ ! -f "permuter/${func}/compile.sh" ]; then
        bash tools/dc.sh setup "$func" "$src" >/dev/null 2>&1 || {
            echo "${func},${src},,,,SETUP_FAILED"
            continue
        }
    fi
    python3 tools/extract_pure_c.py "$func" --force >/dev/null 2>&1 || {
        echo "${func},${src},,,,EXTRACT_FAILED"
        continue
    }
    json="$(python3 tools/cc1psx_diagnostic.py "$func" --json 2>/dev/null)"
    if [ -z "$json" ]; then
        echo "${func},${src},,,,DIAG_FAILED"
        continue
    fi
    line="$(python3 -c "
import json, sys
data = json.loads(sys.stdin.read())[0]
print(','.join([
    data.get('func', ''),
    '${src}',
    str(data.get('target_insns', '')),
    data.get('decompals_score', '').replace(',', ';'),
    data.get('cc1psx_score', '').replace(',', ';'),
    data.get('category', ''),
]))
" <<<"$json")"
    echo "$line"
done
