#!/bin/bash
# perm_setup.sh: decomp-permuter campaign dir from the full split body (pv_all, dumps/pv_all/t.i =
# the preprocessed text1b.c with every other function stripped) -> tmp/func_80057E84/r11/perm.
# compile.sh = build cc1 | prologue_fix | maspsx | as (the build's stages, function extracted);
# target.o = asm/funcs/func_80057E84.s assembled alone. Run: perm_run.sh.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
P=tmp/func_80057E84/r11/perm
mkdir -p $P
CCF=$(cat tmp/func_80057E84/r11/dumps/pv_all/ccflags.txt)
cp tmp/func_80057E84/r11/dumps/pv_all/t.i $P/base.c
printf 'func_name = "func_80057E84"\ncompiler_type = "gcc"\n' > $P/settings.toml
cat > $P/compile.sh <<EOF
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="\$1"; OUT="\$3"
TMPS=\$(mktemp /tmp/pqXXXXXX)
trap "rm -f \$TMPS \$TMPS.*" EXIT
tools/gcc-2.7.2/build/cc1 $CCF "\$IN" -o "\$TMPS.cc1.s" 2>/dev/null
python3 tools/prologue_fix.py < "\$TMPS.cc1.s" > "\$TMPS.pf.s"
python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section < "\$TMPS.pf.s" > "\$TMPS.m.s"
awk '/^[ \t]*\.ent[ \t]+func_80057E84\$/ {p=1} p {print} p && /^[ \t]*\.end[ \t]+func_80057E84\$/ {exit}' "\$TMPS.m.s" > "\$TMPS.fn.s"
( echo ".set noat"; echo ".set noreorder"; echo ".text"; cat "\$TMPS.fn.s" ) | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "\$OUT"
EOF
chmod +x $P/compile.sh
( echo '.include "macro.inc"'; echo ".set noat"; echo ".set noreorder"; echo ".text"; cat asm/funcs/func_80057E84.s ) | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $P/target.o
bash $P/compile.sh $P/base.c -o $P/base.o
# control: the reuse body through the same compile.sh must score 0 against target.o
cp tmp/func_80057E84/r11/dumps/reuse/t.i $P/reuse_control.c
bash $P/compile.sh $P/reuse_control.c -o $P/reuse_control.o
echo setup-ok
