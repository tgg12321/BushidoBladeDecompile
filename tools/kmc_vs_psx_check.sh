#!/bin/bash
# Compare KMC-gcc-2.7.2 vs PSX-gcc-2.7.2 byte output for each function.
#
# Sanity-check for "is our compiler choice (decompals/mips-gcc-2.7.2,
# calibrated to match KMC's N64 output) wrong for a PSX project, or
# does it produce equivalent code to the PSX-targeted variant when
# given PSX flags?"
#
# Measured 2026-05-18 across 8 functions: byte-identical .text on
# all 8. The forks differ only in target defaults (CPU=R3000, MIPS1,
# soft-float, etc.) which our Makefile passes explicitly via
# `-mcpu=3000 -mips1 -mno-abicalls`. The register allocator and
# instruction scheduler are unchanged between forks.
#
# See memory/rules/compiler-patch-low-roi.md for the policy
# implication; this script is the reproducibility check.
#
# Prerequisite: gcc-2.7.2-psx extracted to /tmp/gcc-psx-test/cc1.
# Get it via:
#   gh release download --repo decompals/old-gcc 0.17 \
#     --pattern gcc-2.7.2-psx.tar.gz --clobber -D /tmp
#   mkdir -p /tmp/gcc-psx-test && \
#     tar -xzf /tmp/gcc-2.7.2-psx.tar.gz -C /tmp/gcc-psx-test
#
# Usage: bash tools/kmc_vs_psx_check.sh <func1> [func2 ...]
#   Each func must have permuter/<func>/base.c (use
#   tools/extract_pure_c.py to populate from src/).
set -uo pipefail
cd "$(dirname "$0")/.."

CC1_KMC="tools/gcc-2.7.2/build/cc1"
CC1_PSX="/tmp/gcc-psx-test/cc1"

CPP_FLAGS=(-I include -undef -Wall -lang-c -fno-builtin
    -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips
    -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__
    -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER)
CC_FLAGS=(-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w)
MASPSX_FLAGS=(--expand-div --aspsx-version=2.34
    --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt
    --sdata-exclude=sdata_exclude.txt --expand-lb
    --expand-lb-funcs=expand_lb_funcs.txt
    --multu-funcs=multu_funcs.txt
    --expand-dest-funcs=expand_dest_funcs.txt)
AS_FLAGS=(-I include -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0)

build_with() {
    local cc1="$1" base="$2" out="$3"
    mipsel-linux-gnu-cpp "${CPP_FLAGS[@]}" "$base" 2>/dev/null \
      | "$cc1" "${CC_FLAGS[@]}" 2>/dev/null \
      | python3 tools/prologue_fix.py 2>/dev/null \
      | python3 tools/maspsx/maspsx.py "${MASPSX_FLAGS[@]}" 2>/dev/null \
      | mipsel-linux-gnu-as "${AS_FLAGS[@]}" -o "$out" 2>/dev/null
}

for func in "$@"; do
    base="permuter/$func/base.c"
    if [ ! -f "$base" ]; then
        echo "$func: no base.c, skip"
        continue
    fi
    build_with "$CC1_KMC" "$base" /tmp/kmc.o
    build_with "$CC1_PSX" "$base" /tmp/psx.o
    mipsel-linux-gnu-objcopy -O binary -j .text /tmp/kmc.o /tmp/kmc.bin 2>/dev/null
    mipsel-linux-gnu-objcopy -O binary -j .text /tmp/psx.o /tmp/psx.bin 2>/dev/null
    KMC_SHA=$(sha1sum /tmp/kmc.bin 2>/dev/null | cut -d' ' -f1)
    PSX_SHA=$(sha1sum /tmp/psx.bin 2>/dev/null | cut -d' ' -f1)
    KMC_SIZE=$(stat -c%s /tmp/kmc.bin 2>/dev/null || echo 0)
    PSX_SIZE=$(stat -c%s /tmp/psx.bin 2>/dev/null || echo 0)
    if [ "$KMC_SHA" = "$PSX_SHA" ] && [ -n "$KMC_SHA" ]; then
        echo "$func: IDENTICAL .text (${KMC_SIZE} bytes, sha1=${KMC_SHA:0:12})"
    else
        echo "$func: DIFFER  kmc=${KMC_SHA:0:12}/${KMC_SIZE}b  psx=${PSX_SHA:0:12}/${PSX_SIZE}b"
    fi
done
