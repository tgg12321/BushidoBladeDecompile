#!/usr/bin/env bash
# Build a hand-made decomp-permuter workspace for func_800290B8.
# usage: bash tmp/f290b8/mkperm.sh <dir> <candidate.c>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D="$1"; CAND="$2"
rm -rf "$D"; mkdir -p "$D"
{
  echo '#include "common.h"'
  echo '#include "gpu.h"'
  echo '#include "sound.h"'
  echo '#include "game.h"'
  echo '#include "system.h"'
  echo '#include "gte.h"'; echo '#include "code6cac.h"'
  echo '#include "bb2_const.h"'; echo 'extern s16 *func_8004678C(void);'
  cat "$CAND"
} > "$D/src.c"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$D/src.c" | grep -v '^#' > "$D/base.c"
cat > "$D/compile.sh" <<'EOF'
#!/usr/bin/env bash
INPUT="$(realpath "$1")"
OUTPUT="$(realpath "$3")"
cd '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < "$INPUT" | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt | sed "s/\.align\t3/.align\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUTPUT"
EOF
chmod +x "$D/compile.sh"
{
  printf ".set noat
.set noreorder
"; cat include/macro.inc
  printf '\n.section .text\n'
  cat asm/funcs/func_800290B8.s
} > "$D/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$D/target.o" "$D/target.s"
cp tmp/perm_6d8b/settings.toml "$D/settings.toml"
sed -i 's/func_8006D808/func_800290B8/' "$D/settings.toml"
"$D/compile.sh" "$D/base.c" -o "$D/base.o"
echo built "$D"
