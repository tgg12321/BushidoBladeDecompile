#!/bin/bash
# Compile selected TUs N times with the exact Makefile recipe; report distinct object hashes.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
N=${N:-40}
OUT=tmp/s6/loop
rm -rf "$OUT"; mkdir -p "$OUT"
for id in main/87A0 main/9F9C main/17AFC main/3AB48 main/51268 main/368E4; do
  cmd=$(make -n -B "build/src/$id.o" 2>/dev/null | grep -F "src/$id.c" | head -1)
  [ -z "$cmd" ] && { echo "no recipe for $id"; continue; }
  safe=${id//\//_}
  for i in $(seq 1 $N); do
    o="$OUT/$safe.$i.o"
    c=${cmd//"build/src/$id.o"/$o}
    bash -o pipefail -c "$c" >/dev/null 2>&1 || echo "fail $id $i"
    objcopy_cmd=$(make -n -B "build/src/$id.o" 2>/dev/null | grep -F -- "--set-section-alignment" | head -1)
    [ -n "$objcopy_cmd" ] && bash -c "${objcopy_cmd//"build/src/$id.o"/$o}" >/dev/null 2>&1
  done
  echo "$id: $(sha1sum $OUT/$safe.*.o | awk '{print $1}' | sort | uniq -c | tr '\n' ' ') ref=$(sha1sum build/src/$id.o | cut -c1-40)"
done
