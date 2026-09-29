#!/bin/bash
# usage: mkperm.sh <template-tag> <permdir>
#   template tmp/func_800187F4/<tag>.c (@gte_ markers) -> gen.py -> head.h + body -> preprocessed base.c
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T="$1"; D="$2"
mkdir -p "$D"
python3 tmp/func_800187F4/gen.py tmp/func_800187F4/$T.c "$D/body.c"
cat > "$D/head.h" <<'H'
#include "common.h"
extern u8 D_8008D118;
void func_80018094(s32 *arg0, s32 *arg1);
H
cat "$D/head.h" "$D/body.c" | mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C 2>/dev/null | grep -v '^# ' > "$D/base_pp.c"
python3 tmp/f187/b64asm.py "$D/base_pp.c" "$D/base.c"
cat > "$D/compile.sh" <<'C'
#!/bin/bash
# permuter-style: compile.sh in.c -o out.o (preprocessed input; the build's exact cc1..as recipe for code6cac)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"; OUT="$3"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
  | sed "s/\.align\t3/.align\t2/" \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
C
chmod +x "$D/compile.sh"
grep -v '^\.set gp=64' tools/decomp-permuter/prelude.inc > "$D/target.s"
echo '.include "include/gte_macros.inc"' >> "$D/target.s"
cat asm/funcs/func_800187F4.s >> "$D/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 "$D/target.s" -o "$D/target.o"
printf 'func_name = "func_800187F4"\ncompiler_type = "gcc"\n' > "$D/settings.toml"
"$D/compile.sh" "$D/base_pp.c" -o "$D/base.o"
mipsel-linux-gnu-objdump -d --no-show-raw-insn "$D/base.o" | awk '/<func_800187F4>:/{f=1;next} f&&/^$/{exit} f' | wc -l
mipsel-linux-gnu-objdump -d --no-show-raw-insn "$D/target.o" | awk '/<func_800187F4>:/{f=1;next} f&&/^$/{exit} f' | wc -l
