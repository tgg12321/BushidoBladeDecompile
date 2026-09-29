#!/bin/bash
# usage: mkperm.sh <candidate.c> <permdir>
# Standalone decomp-permuter workspace for func_800198D0: head.h + candidate
# (macros expanded by cpp) -> base.c; compile.sh = the Makefile's code6cac
# recipe (cc1 | prologue_fix | maspsx | align | multu_pad | as); target.o from
# asm/funcs/func_800198D0.s at offset 0.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C="$1"; D="$2"
mkdir -p "$D"
cat > "$D/head.h" <<'H'
#include "common.h"
extern u8 D_800F1B18[];
H
cat "$D/head.h" "$C" | mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx 2>/dev/null \
  | grep -v '^# ' > "$D/base.c"
cat > "$D/compile.sh" <<'CEOF'
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"; OUT="$3"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
  | sed "s/\.align\t3/.align\t2/" \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
CEOF
chmod +x "$D/compile.sh"
grep -v '^\.set gp=64' tools/decomp-permuter/prelude.inc > "$D/target.s"
cat asm/funcs/func_800198D0.s >> "$D/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 "$D/target.s" -o "$D/target.o"
printf 'func_name = "func_800198D0"\ncompiler_type = "gcc"\n' > "$D/settings.toml"
"$D/compile.sh" "$D/base.c" -o "$D/base.o"
echo "base insns:   $(mipsel-linux-gnu-objdump -d --no-show-raw-insn "$D/base.o" | awk '/<func_800198D0>:/{f=1;next} f&&/^$/{exit} f' | grep -c '^ ')"
echo "target insns: $(mipsel-linux-gnu-objdump -d --no-show-raw-insn "$D/target.o" | awk '/<func_800198D0>:/{f=1;next} f&&/^$/{exit} f' | grep -c '^ ')"
