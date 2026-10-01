#!/bin/bash
# where are the <0x800A3308 gp symbols defined in the reference build?
cd /tmp/q56/tree/build
for s in D_800A30DC D_800A30FC D_800A30FD D_800A31D8 D_800A31D9 D_800A31DA D_800A31F0 D_800A3200 D_800A3201 D_800A3203 D_800A3204 D_800A3205 D_800A321C D_800A3228 D_800A3234 D_800A3240 D_800A3248 D_800A324A D_800A324C D_800A3250 D_800A32E9 D_800A322C D_800A336C; do
  d=$(for o in $(find . -name '*.o'); do mipsel-linux-gnu-nm --defined-only $o 2>/dev/null | grep -q " $s\$" && echo $o; done | tr '\n' ' ')
  echo "$s defined_in: ${d:-<symbol file only>}"
done
grep -n "D_800A30DC\|D_800A3234 \|D_800A3240" /tmp/q56/tree/undefined_syms_auto.txt /tmp/q56/tree/named_syms.txt | head
