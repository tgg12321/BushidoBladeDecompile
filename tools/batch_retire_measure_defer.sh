#!/bin/bash
# Batch retire+measure+defer for a list of asmfix functions.
# For each function: write active marker, retire bridge, build, measure
# diff via verify-c, restore bridge, append to defer file with measurement.
#
# Usage: bash tools/batch_retire_measure_defer.sh func1 func2 func3 ...

set -e
cd "$(dirname "$0")/.."

DEFER_FILE="tools/asmfix_defer.txt"

for FUNC in "$@"; do
    echo "=== $FUNC ==="
    echo "$FUNC" > .bb2_active_func
    if ! bash tools/dc.sh retire "$FUNC" > /tmp/retire.log 2>&1; then
        echo "  retire failed; skipping"
        : > .bb2_active_func
        continue
    fi
    SRC_FILE=$(grep -l "$FUNC" src/*.c 2>/dev/null | head -1)
    if [ -n "$SRC_FILE" ]; then
        OBJ_FILE="build/$(basename $SRC_FILE .c).o"
        rm -f "build/src/$(basename $SRC_FILE)" "$OBJ_FILE" 2>/dev/null || true
    fi
    bash tools/dc.sh build > /tmp/build.log 2>&1 || true
    DIFF_LINE=$(bash tools/dc.sh verify-c "$FUNC" 2>&1 | grep "differ\|MATCH" | head -1)
    SIDE_BY_SIDE=$(bash tools/dc.sh side-by-side "$FUNC" 2>&1 | grep "Summary:" | head -1)
    echo "  $DIFF_LINE"
    echo "  $SIDE_BY_SIDE"

    # Restore bridge
    python3 -c "
import re
from pathlib import Path
p = Path('asmfix.txt')
lines = p.read_text(encoding='utf-8').splitlines()
out = []
restored = 0
for line in lines:
    if line.startswith('# RETIRE: ') and '$FUNC' in line:
        out.append(line[len('# RETIRE: '):])
        restored += 1
    else:
        out.append(line)
p.write_text('\n'.join(out) + '\n', encoding='utf-8')
"

    # Restore label drift if any
    bash tools/dc.sh build > /tmp/build2.log 2>&1
    if grep -q "undefined reference to .L" /tmp/build2.log; then
        # Need to find what label drifted
        LABEL=$(grep -oE "\.L[0-9]+" /tmp/build2.log | head -1)
        if [ -n "$LABEL" ]; then
            # Likely func_80074B18 drifted; find and revert
            # to .L1077 (the historic value)
            sed -i "s|bne\\\\t\$2,\$0,\.L[0-9]\+|bne\\\\t\$2,\$0,.L1077|" regfix.txt 2>/dev/null || true
        fi
    fi

    : > .bb2_active_func

    # Append to defer file with measurement
    MEASUREMENT=$(echo "$SIDE_BY_SIDE" | sed 's/Summary: //; s/,/;/g')
    echo "${FUNC}         # Retire-measure: $MEASUREMENT. Defer 2026-05-18." >> "$DEFER_FILE"
done

# Final regen
python3 tools/gen_work_queue.py 2>&1 | tail -2
bash tools/dc.sh build 2>&1 | grep -E "matches|MISMATCH" | head -2
