#!/bin/bash
# perm_mar3: full-TU compile (correct codegen context) + marionation-only
# extraction so the permuter metric is the real per-function diff.
set -e
cd "$(dirname "$0")/.."
mkdir -p tmp/perm_mar3
cp tmp/perm_mar/base.c tmp/perm_mar3/base.c
cat > tmp/perm_mar3/settings.toml <<'EOF'
func_name = "marionation_Exec"
compiler_type = "gcc"

[weight_overrides]
perm_inline = 0.0
EOF

cat > tmp/perm_mar3/compile.sh <<'EOF'
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"
OUT="$3"
TMPS=$(mktemp /tmp/marXXXXXX.s)
trap "rm -f $TMPS $TMPS.fn.s" EXIT
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 \
    -mips1 -mno-abicalls -fno-builtin -w "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 \
      --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt \
      --sdata-exclude=sdata_exclude.txt --expand-lb \
      --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt \
      --expand-dest-funcs=expand_dest_funcs.txt \
      --label-nop-funcs=maspsx_label_nop_funcs.txt \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$TMPS"
# extract marionation_Exec's region: from its glabel/ent to the next .end/.ent
awk '
/^\t\.ent\tmarionation_Exec$|^glabel marionation_Exec$|^marionation_Exec:$/ {p=1}
p {print}
p && /^\t\.end\tmarionation_Exec$/ {exit}
' "$TMPS" > "$TMPS.fn.s"
cat > /tmp/mar_prelude.s <<'PRE'
.set noat
.set noreorder
PRE
cat /tmp/mar_prelude.s "$TMPS.fn.s" \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 \
      -no-pad-sections -O1 -G0 -o "$OUT"
EOF
chmod +x tmp/perm_mar3/compile.sh

# target.o already built by perm_make3.sh; rebuild to be safe
sed 's/^.set gp=64$//' tools/decomp-permuter/prelude.inc > tmp/perm_mar3/prelude_r3k.inc
cat tmp/perm_mar3/prelude_r3k.inc asm/funcs/marionation_Exec.s > tmp/perm_mar3/target.s
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections \
  -O1 -G0 -o tmp/perm_mar3/target.o tmp/perm_mar3/target.s

# validate: compile base and diff against target
bash tmp/perm_mar3/compile.sh tmp/perm_mar3/base.c -o tmp/perm_mar3/base.o
f() {
  mipsel-linux-gnu-objdump -d "$1" | awk '/<marionation_Exec>:/{p=1;next} /^[0-9a-f]+ </{p=0} p' \
    | sed 's/^[^\t]*\t[^\t]*\t//' | sed 's/0x[0-9a-f]*//g' | grep -v '^$'
}
f tmp/perm_mar3/base.o > /tmp/pm3_base.txt
f tmp/perm_mar3/target.o > /tmp/pm3_tgt.txt
echo "base insns: $(wc -l < /tmp/pm3_base.txt)  target: $(wc -l < /tmp/pm3_tgt.txt)"
diff /tmp/pm3_base.txt /tmp/pm3_tgt.txt | head -20
