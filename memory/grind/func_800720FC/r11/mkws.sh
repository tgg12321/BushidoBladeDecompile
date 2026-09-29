#!/bin/bash
# Permuter workspace for func_800720FC (Ruling 11 (D)(4): campaign from the
# one-variable-per-value body). base.c = a minimal TU: text1b.c's include lines
# + the file-scope declarations func_800720FC names (split/tusplit.py logic,
# D_800A3578 retyped s16 as in the landing) + the body; preprocessed.
# usage: mkws.sh <body.c> <workspace dir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
BODY="$1"; WS="$2"
mkdir -p "$WS"
python3 - "$BODY" "$WS/min.c" <<'PY'
import subprocess, sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm
src = Path('src/text1b.c').read_text(encoding='utf-8')
src = inlineasm.substitute_body(src, 'func_800720FC', Path(sys.argv[1]).read_text(encoding='utf-8'))
src = src.replace('extern u16 D_800A3578;', 'extern s16 D_800A3578;')
i = src.index('void func_800720FC(s32 arg0, s32 arg1, s32 mode) {')
# keep the body's own preceding declarations (from the INCLUDE_ASM position)
j = src.index('extern s16 D_8009BCC4[][2];')
k = src.index('\n}\n', i) + 3
Path('tmp/func_800720FC/_mk_src.c').write_text(src[:j] + '\nvoid __split_here(void) {\n}\n' + src[j:k], encoding='utf-8', newline='\n')
subprocess.run([sys.executable, 'memory/grind/func_800720FC/split/tusplit.py', '__split_here',
                'tmp/func_800720FC/_mk_pre.c', sys.argv[2], 'tmp/func_800720FC/_mk_rep.txt',
                'tmp/func_800720FC/_mk_src.c'], check=True, capture_output=True)
PY
CPP_DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $CPP_DEFS "$WS/min.c" 2>/dev/null \
  | grep -v '^__asm__(".include' > "$WS/base.c"
cat > "$WS/settings.toml" <<'EOF'
func_name = "func_800720FC"
compiler_type = "gcc"

[weight_overrides]
perm_inline = 0.0
EOF
cat > "$WS/compile.sh" <<'EOF'
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"
OUT="$3"
TMPS=$(mktemp /tmp/f720XXXXXX.s)
trap "rm -f $TMPS $TMPS.fn.s" EXIT
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 \
    -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 \
      --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt \
      --sdata-exclude=sdata_exclude.txt --expand-lb \
      --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt \
      --expand-dest-funcs=expand_dest_funcs.txt \
      --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
  | sed "s/\.align\t3/.align\t2/" \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$TMPS"
awk '
/^\.ent[ \t]+func_800720FC$/ {p=1}
p {print}
p && /^\.end[ \t]+func_800720FC$/ {exit}
' "$TMPS" > "$TMPS.fn.s"
{ echo ".set noat"; echo ".set noreorder"; cat "$TMPS.fn.s"; } \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 \
      -no-pad-sections -O1 -G0 -o "$OUT"
EOF
chmod +x "$WS/compile.sh"
sed 's/^.set gp=64$//' tools/decomp-permuter/prelude.inc > "$WS/prelude_r3k.inc"
cat "$WS/prelude_r3k.inc" asm/funcs/func_800720FC.s > "$WS/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections \
  -O1 -G0 -o "$WS/target.o" "$WS/target.s"
bash "$WS/compile.sh" "$WS/base.c" -o "$WS/base.o"
echo "base.o built: $(mipsel-linux-gnu-objdump -d "$WS/base.o" | grep -c '^ ')"
