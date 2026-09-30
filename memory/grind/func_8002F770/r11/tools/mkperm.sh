#!/bin/bash
# Standalone decomp-permuter workspace for a code6cac_b_tu2.c function body.
# usage: bash tmp/gte/mkperm.sh <func> <body.c> <permdir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
F="$1"; B="$2"; D="$3"
rm -rf "$D"; mkdir -p "$D"
cat > "$D/head.h" <<'H'
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "gte.h"
#include "code6cac.h"
#include "bb2_const.h"
extern s32 ratan2(s32, s32);
extern void RotMatrixX(s32, s32 *);
extern void RotMatrixY(s32, s32 *);
extern void RotMatrixZ(s32, s32 *);
extern u8 g_sqrt_table_u8;
H
cat "$D/head.h" "$B" | mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C 2>/dev/null | grep -v '^# ' > "$D/base_pp.c"
python3 memory/grind/func_800187F4/r11/tools/b64asm.py "$D/base_pp.c" "$D/base.c"
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
echo "base insns: $(mipsel-linux-gnu-objdump -d --no-show-raw-insn "$D/base.o" | awk "/<$F>:/{f=1;next} f&&/^\$/{exit} f" | wc -l)"
echo "target insns: $(mipsel-linux-gnu-objdump -d --no-show-raw-insn "$D/target.o" | awk "/<$F>:/{f=1;next} f&&/^\$/{exit} f" | wc -l)"
python3 tools/decomp-permuter/permuter.py --help >/dev/null 2>&1 && echo permuter-ok
