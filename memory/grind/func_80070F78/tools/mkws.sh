#!/bin/bash
# Permuter workspace for func_80070F78: base.c = preprocessed mini TU
# (tmp/func_80070F78/mini/pre.c: text1b.c prefix with other bodies stripped,
# SelectEntryE534 pad->unk1) + the body. compile.sh = the build recipe with the
# landing's sdata_exclude/sdata_syms copies.
# usage: mkws.sh <body.c> <workspace dir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
BODY="$1"; WS="$2"
mkdir -p "$WS"
cat tmp/func_80070F78/mini/pre.c "$BODY" > "$WS/min.c"
CPP_DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $CPP_DEFS "$WS/min.c" 2>/dev/null \
  | grep -v '^__asm__(".include' > "$WS/base.c"
cat > "$WS/settings.toml" <<'EOF'
func_name = "func_80070F78"
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
TMPS=$(mktemp /tmp/f78XXXXXX.s)
trap "rm -f $TMPS $TMPS.fn.s" EXIT
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 \
    -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 \
      --sdata-syms=tmp/func_80070F78/sdata_syms.txt --sdata-funcs=sdata_funcs.txt \
      --sdata-exclude=tmp/func_80070F78/sdata_exclude.txt --expand-lb \
      --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt \
      --expand-dest-funcs=expand_dest_funcs.txt \
      --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
  | sed "s/\.align\t3/.align\t2/" \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$TMPS"
awk '
/^\.ent[ \t]+func_80070F78$/ {p=1}
p {print}
p && /^\.end[ \t]+func_80070F78$/ {exit}
' "$TMPS" > "$TMPS.fn.s"
{ echo ".set noat"; echo ".set noreorder"; cat "$TMPS.fn.s"; } \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 \
      -no-pad-sections -O1 -G0 -o "$OUT"
EOF
chmod +x "$WS/compile.sh"
sed 's/^.set gp=64$//' tools/decomp-permuter/prelude.inc > "$WS/prelude_r3k.inc"
cat "$WS/prelude_r3k.inc" asm/funcs/func_80070F78.s > "$WS/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections \
  -O1 -G0 -o "$WS/target.o" "$WS/target.s"
bash "$WS/compile.sh" "$WS/base.c" -o "$WS/base.o"
echo "base.o built: $(mipsel-linux-gnu-objdump -d "$WS/base.o" | grep -c '^ ')"
