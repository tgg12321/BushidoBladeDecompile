#!/bin/bash
# perm_setup.sh <dir> <cand.c> : decomp-permuter workspace for func_8002AB08 (minimal TU prelude)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
W=$1; CAND=$2
mkdir -p "$W"
{
  sed -n 2,46p src/code6cac_b_tu2.c | grep -v INCLUDE_RODATA
  sed -n '/^typedef struct {$/,/^#define SPAD/p' src/code6cac_b_tu2.c | sed -n '/LeafPos unk00\[2\]\[3\]/,$p' | head -0
  echo 'typedef struct { LeafPos unk00[2][3]; LeafPos unk48[2][2]; u8 unk78[0xA8 - 0x78]; LeafPos unkA8[2][22]; } ScrPad;'
  echo '#define SPAD ((ScrPad *)0x1F800000)'
  echo 'extern void func_800274BC(s32 *arg0, s16 *arg1);'
  echo 'extern s32 func_80027AD8(s32, u8 *, s32, s32, s32, Tbl8008E194 *, s32, s32 *);'
  echo 'extern void func_8002A458(u8 *obj, s32 *hit, s32 *deep, s32 quiet);'
  echo 'extern void func_80032854(s32 arg0, s32 arg1, u8 *arg2, s16 *arg3);'
  cat "$CAND"
} > "$W/src.c"
mipsel-linux-gnu-cpp ${HDR_I} -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -P "$W/src.c" > "$W/base.c" 2>/dev/null
sed 's/func_8005C8A8/func_8002AB08/' tmp/perm_c8a8/compile.sh | sed 's#sdata-exclude=sdata_exclude.txt#sdata-exclude=sdata_exclude.txt --use-comm-section#' > "$W/compile.sh"; chmod +x "$W/compile.sh"
sed 's/func_8005C8A8/func_8002AB08/' tmp/perm_c8a8/settings.toml > "$W/settings.toml"
sed 's/^.set gp=64$//' tools/decomp-permuter/prelude.inc > "$W/target.s"
cat asm/funcs/func_8002AB08.s >> "$W/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$W/target.o" "$W/target.s"
bash "$W/compile.sh" "$W/base.c" -o "$W/base.o"
mipsel-linux-gnu-objdump -d "$W/base.o" --disassemble=func_8002AB08 | grep -c "^ "
