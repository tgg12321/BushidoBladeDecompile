#!/bin/bash
# mkperm.sh <body.c> <workspace-dir> : decomp-permuter workspace for func_800571C0.
# base.c = the minimal declarations the body needs + <body.c>; compile.sh = the build's per-file recipe
# for code6cac_tu2 (-G0 CC_FLAGS, MASPSX_FLAGS from engine/buildconfig.py); target.o from asm/funcs.
set -e
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT"
source .venv/bin/activate
BODY="$1"; W="$2"
mkdir -p "$W"
{
cat <<'EOF'
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef struct { s32 x, y, z, w; } Vec4_571C0;
extern s16 Judge[];
extern s32 D_800A387C;
extern s32 rand(void);
extern s32 func_80053614(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 arg4);
EOF
cat "$BODY"
} > "$W/base.c"
printf 'func_name = "func_800571C0"\ncompiler_type = "gcc"\n' > "$W/settings.toml"
CPP=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CPP_FLAGS, b.CPP_DEFS)")
CC=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CC_FLAGS)")
MS=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.MASPSX_FLAGS)")
cat > "$W/compile.sh" <<EOF
#!/usr/bin/env bash
INPUT="\$(realpath "\$1")"
OUTPUT="\$(realpath "\$3")"
TMPC=\$(mktemp /tmp/71c0XXXXXX.c); cp "\$INPUT" "\$TMPC"; trap "rm -f \$TMPC" EXIT; INPUT="\$TMPC"
cd '$ROOT'
mipsel-linux-gnu-cpp $CPP "\$INPUT" | tools/gcc-2.7.2/build/cc1 $CC | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py $MS | sed "s/\\.align\\t3/.align\\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "\$OUTPUT"
EOF
chmod +x "$W/compile.sh"
sed 's/^.set gp=64$//' tools/decomp-permuter/prelude.inc > "$W/prelude.inc"
cat "$W/prelude.inc" asm/funcs/func_800571C0.s > "$W/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$W/target.o" "$W/target.s"
bash "$W/compile.sh" "$W/base.c" -o "$W/base.o"
f() { mipsel-linux-gnu-objdump -d --no-show-raw-insn "$1" | awk '/<func_800571C0>:/{p=1;next} /^$/{if(p)exit} p' | sed 's/^ *[0-9a-f]*:\t//; s/<[^>]*>//g; s/\.L[0-9A-Fa-f]*//g'; }
echo "base vs target differing lines: $(diff <(f "$W/base.o") <(f "$W/target.o") | grep -c '^[<>]')"
