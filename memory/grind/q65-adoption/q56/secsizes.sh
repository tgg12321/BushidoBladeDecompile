#!/bin/bash
# secsizes.sh obj...: section sizes (.text .rodata .data .bss .sdata .sbss) of reference objects in the adopt clone
cd "/tmp/q56/adopt tree/build/src"
for o in "$@"; do
  printf "%-32s" "$o"
  mipsel-linux-gnu-objdump -h "$o.o" | awk '$2 ~ /^\.(text|rodata|data|bss|sdata|sbss)$/ {printf "%s=%s ", $2, $3}'
  echo
done
