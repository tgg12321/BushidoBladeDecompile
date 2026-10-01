#!/bin/bash
cp "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/splitcc1.py" /tmp/q56/splitcc1.py
cd /tmp/q56/model && source .venv/bin/activate
rm -rf build && make -j16 build/bb2.exe > build.log 2>&1; echo "make_rc=$?"
echo "exe_sha1=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)"
grep -E "Error|error" build.log | head
