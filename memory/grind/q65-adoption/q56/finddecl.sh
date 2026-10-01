#!/bin/bash
cd /tmp/q56/model
for s in D_800A3924 D_800A3230 D_800A3204 D_800A3318 D_800A3714 D_800A379C D_800A3250 D_800A326C D_800A3418 D_800A3724 D_800A3354; do
  echo "== $s"; grep -n "\b$s\b" src/*.c include/*.h | grep -v "__typeof__" | head -4 | cut -c1-200
done
