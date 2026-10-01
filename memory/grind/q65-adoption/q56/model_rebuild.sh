#!/bin/bash
cd /tmp/q56/model && source .venv/bin/activate
for i in 1 2 3 4; do
  make -k -j16 build/bb2.exe > build.log 2>&1 && break
  grep -q "undeclared here" build.log || break
  python3 "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/model_fixundecl.py"
done
echo "exe_sha1=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)"
grep -E "src/\w+\.c:[0-9]+: |MASPSX|Error [0-9]" build.log | grep -v warning | head -20
