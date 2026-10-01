#!/bin/bash
# smalldata.sh: every C object's .data symbols (address in the linked image, size) — which are <= 8 bytes?
cd "/tmp/q56/adopt tree/build/src"
for o in *.o; do
  s=$(mipsel-linux-gnu-objdump -h $o | awk '$2==".data"{print $3}')
  [ -z "$s" ] && continue
  [ "$s" = "00000000" ] && continue
  echo "== $o .data size 0x$s"
  mipsel-linux-gnu-nm -S --defined-only $o | awk '$3=="D"||$3=="d"' | head -20
done
