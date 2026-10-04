#!/bin/bash
# snap.sh NAME: sha1 every build object + exe into tmp/s6/det/NAME.sha1 and diff vs serial1
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT=tmp/s6/det
(cd build && find . -name '*.o' -o -name 'bb2.exe' | sort | xargs sha1sum) > "$OUT/$1.sha1"
echo "== serial1 vs $1" >> "$OUT/summary.txt"
diff "$OUT/serial1.sha1" "$OUT/$1.sha1" >> "$OUT/summary.txt" && echo identical >> "$OUT/summary.txt"
tail -4 "$OUT/summary.txt"
