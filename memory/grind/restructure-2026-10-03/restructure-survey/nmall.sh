#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
out=tmp/restructure-survey/objfuncs.txt
: > $out
for o in build/src/*.o; do
  mipsel-linux-gnu-nm -n --defined-only $o | awk -v o=$(basename $o .o) '$2 ~ /[TtDdRrBbSsVv]/ {print o, $2, $3}' >> $out
done
mipsel-linux-gnu-nm -n -S --defined-only build/bb2.elf > tmp/restructure-survey/elf_nm.txt
wc -l $out tmp/restructure-survey/elf_nm.txt
