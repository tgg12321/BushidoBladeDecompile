#!/bin/bash
# headdiag.sh : cc1 diagnostics of every game TU, HEAD sources+headers ("new" column) vs tree ("old"); '>' = HEAD-only, '<' = tree-only (new in tree)
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
rm -rf tmp/p2/wk && mkdir -p tmp/p2/wk/src/main tmp/p2/wk/include
for f in $(git diff HEAD --name-only -- src include); do git show HEAD:$f > tmp/p2/wk/$f; done
T=$(ls src/main/*.c | sed 's|src/||; s|\.c$||')
for t in $T; do [ -f tmp/p2/wk/src/$t.c ] || cp src/$t.c tmp/p2/wk/src/$t.c; done
wsl bash tmp/p2/w/warn.sh $T 2>&1 | grep -v "^== .*: \([0-9]*\) -> \1 diag"
echo headdiag done
