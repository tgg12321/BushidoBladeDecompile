#!/bin/bash
# build a permuter workspace tmp/perm_759/<name> from an override TU
# usage: mkperm.sh <name> <override.c>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
W=tmp/perm_759/$1
rm -rf "$W"; mkdir -p "$W"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -DPERMUTER -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -P "$2" > "$W/full.c" 2>/dev/null
python3 tools/decomp-permuter/strip_other_fns.py "$W/full.c" func_800759D0 > /dev/null 2>&1 || true
cp "$W/full.c" "$W/base.c"
cat > "$W/compile.sh" <<'EOF'
#!/usr/bin/env bash
INPUT="$(realpath "$1")"
OUTPUT="$(realpath "$3")"
cd '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < "$INPUT" | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section --expand-lb | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUTPUT"
EOF
chmod +x "$W/compile.sh"
printf '.include "include/macro.inc"\n.set noat\n.set noreorder\n.section .text\n.include "asm/funcs/func_800759D0.s"\n' > "$W/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$W/target.o" "$W/target.s"
printf 'func_name = "func_800759D0"\ncompiler_type = "gcc"\n' > "$W/settings.toml"
"$W/compile.sh" "$W/base.c" -o "$W/base.o" && echo BASE_OK
