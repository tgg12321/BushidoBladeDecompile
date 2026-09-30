#!/bin/bash
# Permuter workspace seeded from the one-variable-per-value (split rec) body.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
W=tmp/func_80029454/r11/perm
rm -rf $W; mkdir -p $W
cp tmp/func_80029454/r11/d_split/tu.i $W/base.c
cat > $W/settings.toml <<'EOF'
func_name = "func_80029454"
compiler_type = "gcc"
EOF
cat > $W/compile.sh <<'EOF'
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
IN="$1"; OUT="$3"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
 | python3 tools/prologue_fix.py \
 | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --expand-lb \
 | sed 's/\.align\t3/.align\t2/' | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
 | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
EOF
chmod +x $W/compile.sh
# target: the reuse (staged) body's object; its func_80029454 equals build/ (harness 0/0)
bash $W/compile.sh tmp/func_80029454/r11/d_reuse/tu.i -o $W/target.o
bash $W/compile.sh $W/base.c -o $W/base_check.o
ls -la $W
