#!/bin/bash
# bb2_smoketest.sh -- 5-minute environment + regression check for the
# targeted permuter and its supporting tooling.
#
# Verifies:
#   1. Toolchain is present (cc1, mipsel-linux-gnu-as, .venv, disc image)
#   2. The main make produces a SHA1-matching binary
#   3. The targeted permuter still finds matches on 3 known-good candidates
#      (regression suite for the BB2 mutation passes)
#
# Exit codes:
#   0 — all checks passed
#   1 — environment problem (toolchain missing or main build broken)
#   2 — regression failure (a known match no longer matches)
#
# Usage:
#   bash tools/bb2_smoketest.sh              # default: 60s/candidate
#   bash tools/bb2_smoketest.sh --quick      # 30s/candidate (skips if RNG slow)
#   bash tools/bb2_smoketest.sh --full       # 300s/candidate (canonical run)

set -u

PER_FUNC_TIME=60
case "${1:-}" in
    --quick)  PER_FUNC_TIME=30 ;;
    --full)   PER_FUNC_TIME=300 ;;
    "")       ;;
    *)        echo "Unknown arg: $1" >&2 ; exit 1 ;;
esac

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PASS=0
FAIL=0

say()    { printf '[smoketest] %s\n' "$*" ; }
section() { printf '\n=== %s ===\n' "$*" ; }

# 1. Toolchain check
section "1. Toolchain"
for path in tools/gcc-2.7.2/build/cc1 tools/maspsx/maspsx.py .venv/bin/python3 disc/SLUS_006.63; do
    if [ -e "$path" ]; then
        say "OK  $path"
        PASS=$((PASS + 1))
    else
        say "FAIL  missing: $path"
        FAIL=$((FAIL + 1))
    fi
done
if ! command -v mipsel-linux-gnu-as >/dev/null 2>&1; then
    say "FAIL  mipsel-linux-gnu-as not on PATH"
    FAIL=$((FAIL + 1))
else
    say "OK  mipsel-linux-gnu-as"
    PASS=$((PASS + 1))
fi
if [ "$FAIL" -gt 0 ]; then
    say "STOP: environment broken; re-run bootstrap"
    exit 1
fi

# 2. Main build SHA1
section "2. Main build"
source .venv/bin/activate
if make 2>&1 | tail -3 | grep -q "bb2 matches!"; then
    say "OK  main build SHA1-matches"
    PASS=$((PASS + 1))
else
    say "FAIL  main build does not match"
    say "  run `make 2>&1 | tail -20` to investigate"
    exit 1
fi

# 3. Permuter regression
section "3. Permuter regression (3 known matches)"
# These were the canonical matches that established the BB2 passes work.
# If a future pass change breaks one, this catches it.
REGRESSION_FUNCS=(InitFadePanel func_80077894 mario_getMarioVoiceData_8005BE84)

for func in "${REGRESSION_FUNCS[@]}"; do
    permdir="permuter/$func"
    if [ ! -d "$permdir" ]; then
        say "SKIP  $func: no permuter dir (run bb2_setup_backlog.sh first)"
        continue
    fi
    if [ ! -f "$permdir/target.o" ]; then
        say "SKIP  $func: missing target.o"
        continue
    fi
    rm -rf "$permdir"/output-*
    log="/tmp/bb2_smoketest_${func}.log"
    BB2_PERMUTER_HEAVY=1 timeout "$PER_FUNC_TIME" python3 tools/bb2_permuter.py \
        "$permdir" --stop-on-zero --best-only -j 6 > "$log" 2>&1 || true

    if ls -d "$permdir"/output-0-* >/dev/null 2>&1; then
        iters=$(tr '\r' '\n' < "$log" | grep -oE 'iteration [0-9]+, .*score = 0' | head -1 | grep -oE '[0-9]+' | head -1)
        say "OK  $func: matched at iter $iters"
        PASS=$((PASS + 1))
    else
        best=$(tr '\r' '\n' < "$log" | grep -oE 'score = [0-9]+' | awk '{print $NF}' | sort -n | head -1)
        say "FAIL  $func: no match in ${PER_FUNC_TIME}s (best=$best)"
        say "  log: $log"
        FAIL=$((FAIL + 1))
    fi
done

# Summary
section "Summary"
say "Passed: $PASS"
say "Failed: $FAIL"
if [ "$FAIL" -eq 0 ]; then
    say "All checks passed. Environment + tooling ready for use."
    exit 0
fi
if [ "$FAIL" -gt 0 ] && [ "$PASS" -gt 0 ]; then
    say "Partial pass. Environment OK but at least one regression failure."
    say "Possible causes: RNG variance (--full uses 300s budget per func),"
    say "BB2 pass change regressed a match, or build subtly broken."
    exit 2
fi
exit 2
