#!/bin/bash
# usage: mkperm.sh cand.c permdir
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D="$2"
mkdir -p "$D"
bash tmp/f8b488s2/pp.sh "$1" "$D/base.c"
cp tmp/f8b488s2/compile.sh "$D/compile.sh"
chmod +x "$D/compile.sh"
cp tmp/f8b488s2/target.o "$D/target.o"
printf 'func_name = "func_8008B488"\ncompiler_type = "gcc"\n' > "$D/settings.toml"
wc -l "$D/base.c"
ls "$D"
