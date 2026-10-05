#!/bin/bash
# warn.sh TU... : new cc1 diagnostics (warnings AND errors, -w removed) in tmp/p2/wk/src/<TU>.c
# (with tmp/p2/wk/include) vs src/<TU>.c
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for t in "$@"; do
  line=$(grep -h -m1 -F "src/$t.c |" tmp/p2/*.make.log | head -1)
  d=$(dirname src/$t.c)
  for v in old new; do
    if [ $v = new ]; then srcf=tmp/p2/wk/src/$t.c; [ -f $srcf ] || srcf=src/$t.c; inc="-Itmp/p2/wk/include -I$d -Iinclude -undef"; else srcf=src/$t.c; inc="-I$d -Iinclude -undef"; fi
    cmd=${line//"-Iinclude -undef"/"$inc"}
    cmd=${cmd//" src/$t.c |"/" $srcf |"}
    cmd=${cmd//"-o build/src/$t.o"/"-o tmp/p2/w/w_$v.o"}
    cmd=${cmd//" -fno-builtin -w -mel"/" -fno-builtin -mel"}
    eval "$cmd" 2> tmp/p2/w/w_$v.raw > /dev/null
    python3 tmp/p2/w/wfilt.py < tmp/p2/w/w_$v.raw > tmp/p2/w/w_$v.txt
  done
  echo "== $t: $(grep -c . tmp/p2/w/w_old.txt) -> $(grep -c . tmp/p2/w/w_new.txt) diagnostics"
  diff tmp/p2/w/w_old.txt tmp/p2/w/w_new.txt | grep '^[<>]'
done
