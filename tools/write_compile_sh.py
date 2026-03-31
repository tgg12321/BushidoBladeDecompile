#!/usr/bin/env python3
"""Write the permuter compile.sh script with proper $ variables."""
import os
import sys
import shutil

content = r"""#!/bin/bash
# Compile script for decomp-permuter (BB2)
# Usage: ./compile.sh input.c -o output.o
set -e

INPUT="$1"
shift; shift
OUTPUT="$1"

# Find project root by walking up from script location
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$DIR"
while [ ! -f "$ROOT/Makefile" ] && [ "$ROOT" != "/" ]; do
    ROOT="$(dirname "$ROOT")"
done
if [ ! -f "$ROOT/Makefile" ]; then
    echo "ERROR: Cannot find project root (no Makefile found)" >&2
    exit 1
fi

CC1="$ROOT/tools/gcc-2.7.2/build/cc1"
MASPSX="$ROOT/tools/maspsx/maspsx.py"

CC_FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"

cd "$ROOT"

# Copy input to /tmp to avoid spaces in path (maspsx .file directive bug)
TMP_INPUT="/tmp/bb2_permuter_input.c"
cp "$INPUT" "$TMP_INPUT"

mipsel-linux-gnu-cpp \
  "-I$ROOT/include" -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \
  -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
  -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \
  "$TMP_INPUT" \
  | "$CC1" $CC_FLAGS \
  | python3 "$MASPSX" --expand-div --aspsx-version=2.34 \
      "--sdata-syms=$ROOT/sdata_syms.txt" \
      "--sdata-funcs=$ROOT/sdata_funcs.txt" \
      "--sdata-exclude=$ROOT/sdata_exclude.txt" \
  | mipsel-linux-gnu-as "-I$ROOT/include" -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUTPUT"
"""

# Get project root (directory containing this script's parent)
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)

# Write template
template_path = os.path.join(project_root, "tools", "permuter_compile.sh")
with open(template_path, "w", newline="\n") as f:
    f.write(content)
print(f"Written: {template_path}")

# Copy to any permuter directories specified as arguments
if len(sys.argv) > 1:
    for func_name in sys.argv[1:]:
        dest = os.path.join(project_root, "permuter", func_name, "compile.sh")
        if os.path.isdir(os.path.dirname(dest)):
            shutil.copy(template_path, dest)
            print(f"Copied: {dest}")
        else:
            print(f"Skipped (no dir): {dest}")
else:
    # Default: copy to all existing permuter directories
    permuter_dir = os.path.join(project_root, "permuter")
    if os.path.isdir(permuter_dir):
        for d in os.listdir(permuter_dir):
            dest = os.path.join(permuter_dir, d, "compile.sh")
            if os.path.isdir(os.path.join(permuter_dir, d)):
                shutil.copy(template_path, dest)
                print(f"Copied: {dest}")

print("Done!")
