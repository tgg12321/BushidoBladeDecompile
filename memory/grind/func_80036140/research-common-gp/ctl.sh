#!/bin/bash
# positive control: our cc1 -G8 | prologue_fix | maspsx (stock vs gated) on the extern-CdlATV probe
cd "$(dirname "$0")/../.."
source .venv/bin/activate
F="--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt"
PRE="mipsel-linux-gnu-cpp -Iinclude -undef -lang-c tmp/research36140/c/ctl.c | tools/gcc-2.7.2/build/cc1 -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | python3 tools/prologue_fix.py"
echo "== stock"; bash -c "$PRE | python3 tools/maspsx/maspsx.py $F" | grep -E "^\s*(sb|lw|sw|lb|lbu|lwl|lwr|swl|swr|lui|addiu)\s" | grep -v "\$sp" 
echo "== gated"; bash -c "$PRE | python3 tmp/research36140/maspsx_comm/maspsx.py $F --comm-syms=tmp/research36140/maspsx_comm_syms.txt" | grep -E "^\s*(sb|lw|sw|lb|lbu|lwl|lwr|swl|swr|lui|addiu)\s" | grep -v "\$sp"
