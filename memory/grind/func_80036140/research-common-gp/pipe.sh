#!/bin/bash
# original toolchain: cc1psx (-G8) -> ASPSX 2.34 (-G8); prints decoded .text
# usage: pipe.sh <file.c> [cc1 -G flag]
set -e
cd "$(dirname "$0")/../.."
C="$1"; G="${2:--G8}"
S="${C%.c}.psx.s"
bash tools/cc1psx_wrapper.sh -O2 $G -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < "$C" > "$S"
python3 tmp/research36140/asm_obj.py "$S" -G8 | grep -v "Psy-Q\|Assembly completed\|error(s)"
