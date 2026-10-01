#!/bin/bash
# perm_setup.sh: decomp-permuter campaign dir from the full split body (pv_all, dumps/pv_all/t.i =
# the preprocessed text1b.c with every other function stripped) -> tmp/func_80057E84/r11/perm.
# compile.sh = the build's text1b stages after cpp (stages.py: cc1 | prologue_fix | maspsx | multu_pad),
# function extracted, then as; target.o = asm/funcs/func_80057E84.s assembled alone. Run: perm_run.sh.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
P=tmp/func_80057E84/r11/perm
mkdir -p $P
mapfile -t ST < <(python3 memory/grind/func_80057E84/r11/stages.py)
cp tmp/func_80057E84/r11/dumps/pv_all/t.i $P/base.c
printf 'func_name = "func_80057E84"\ncompiler_type = "gcc"\n' > $P/settings.toml
cat > $P/compile.sh <<EOF
#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="\$1"; OUT="\$3"
TMPS=\$(mktemp /tmp/pqXXXXXX)
trap "rm -f \$TMPS \$TMPS.*" EXIT
${ST[0]} "\$IN" -o "\$TMPS.cc1.s" 2>/dev/null
${ST[1]} < "\$TMPS.cc1.s" | ${ST[2]} | ${ST[3]} > "\$TMPS.m.s"
awk '/^[ \t]*\.ent[ \t]+func_80057E84\$/ {p=1} p {print} p && /^[ \t]*\.end[ \t]+func_80057E84\$/ {exit}' "\$TMPS.m.s" > "\$TMPS.fn.s"
( echo ".set noat"; echo ".set noreorder"; echo ".text"; cat "\$TMPS.fn.s" ) | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "\$OUT"
EOF
chmod +x $P/compile.sh
( echo '.include "macro.inc"'; echo ".set noat"; echo ".set noreorder"; echo ".text"; cat asm/funcs/func_80057E84.s ) | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $P/target.o
bash $P/compile.sh $P/base.c -o $P/base.o
cp tmp/func_80057E84/r11/dumps/reuse/t.i $P/reuse_control.c
bash $P/compile.sh $P/reuse_control.c -o $P/reuse_control.o
echo setup-ok
