#!/bin/bash
# Standalone decomp-permuter workspace for a text1b_tu2.c function body (func_80074E08 copy of
# memory/grind/func_8002CD58/r11/tools/mkperm.sh; the header is the TU's own declarations, lines 1-122).
# usage: bash memory/grind/func_80074E08/r9/tools/mkperm.sh <func> <body.c> <permdir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
F="$1"; B="$2"; D="$3"
rm -rf "$D"; mkdir -p "$D"
head -122 src/text1b_tu2.c > "$D/head.h"
echo "extern s32 g_gpu_ot_ptr;" >> "$D/head.h"
cat "$D/head.h" "$B" | mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C 2>/dev/null | grep -v '^# ' > "$D/base_pp.c"
cp "$D/base_pp.c" "$D/base.c"
cat > "$D/compile.sh" <<'C'
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"; OUT="$3"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
C
chmod +x "$D/compile.sh"
grep -v '^\.set gp=64' tools/decomp-permuter/prelude.inc > "$D/target.s"
echo '.include "include/gte_macros.inc"' >> "$D/target.s"
cat "asm/funcs/$F.s" >> "$D/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 "$D/target.s" -o "$D/target.o"
printf 'func_name = "%s"\ncompiler_type = "gcc"\n' "$F" > "$D/settings.toml"
"$D/compile.sh" "$D/base_pp.c" -o "$D/base.o"
echo "base insns: $(mipsel-linux-gnu-objdump -d -z --no-show-raw-insn "$D/base.o" | awk "/<$F>:/{f=1;next} f&&/^\$/{exit} f" | wc -l)"
echo "target insns: $(mipsel-linux-gnu-objdump -d -z --no-show-raw-insn "$D/target.o" | awk "/<$F>:/{f=1;next} f&&/^\$/{exit} f" | wc -l)"
