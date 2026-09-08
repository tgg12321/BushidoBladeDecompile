#!/bin/bash
# ra_solver / Campaign 7 — tree-wide BB2_RELOAD_DEBUG harvest.
#
# Runs cpp + the instrumented cc1 (tools/gcc-2.7.2/cc1) over every src/*.c with
# BB2_RELOAD_DEBUG=1 and banks the stderr stream per TU under
# tmp/reload_work/<stem>.reload.log.  Print-only: the -o output goes to
# /dev/null and the env var is output-inert (tmp/parity_check_multi.sh).
#
# Usage: bash tools/ra_solver/reload_harvest.sh [stem ...]     (default: all)
set -uo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"

CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin \
-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ \
-D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
BASE="-O2 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
GP_FILES="text1a"          # keep in sync with the Makefile's GP_FILES
CC1=tools/gcc-2.7.2/cc1
OUT=tmp/reload_work
mkdir -p "$OUT"

stems="$*"
if [ -z "$stems" ]; then
  stems=$(ls src/*.c | sed 's|src/||; s|\.c$||')
fi

for stem in $stems; do
  g="-G0"
  for gp in $GP_FILES; do [ "$stem" = "$gp" ] && g="-G8"; done
  $CPP "src/$stem.c" > "$OUT/$stem.i" 2>/dev/null
  BB2_RELOAD_DEBUG=1 "$CC1" $BASE $g "$OUT/$stem.i" -o /dev/null \
      2> "$OUT/$stem.reload.log"
  n=$(grep -c '^RETRYDBG func=' "$OUT/$stem.reload.log" 2>/dev/null || echo 0)
  s=$(grep -c '^RELOADDBG spill_hard_reg' "$OUT/$stem.reload.log" 2>/dev/null || echo 0)
  printf '%-32s retries=%-5s spill_hard_reg=%s\n' "$stem" "$n" "$s"
done
