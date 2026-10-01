#!/bin/bash
# fndiff.sh <objA> <objB> func...: disassembly diff of the named functions.
A="$1"; B="$2"; shift 2
for f in "$@"; do
  echo "=== $f"
  diff <(mipsel-linux-gnu-objdump -dr --disassemble="$f" "$A" | sed -n '/<'$f'>:/,$p' | cut -f3-) \
       <(mipsel-linux-gnu-objdump -dr --disassemble="$f" "$B" | sed -n '/<'$f'>:/,$p' | cut -f3-)
done
