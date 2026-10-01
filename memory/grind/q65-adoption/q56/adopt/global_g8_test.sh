#!/bin/bash
# global_g8_test.sh: scratch clone only - would maspsx -G8 on EVERY file (the rule text's global flag)
# reproduce the oracle, instead of the per-file SDATA_FILES list? Restores the tree afterwards.
A="/tmp/q56/adopt tree"; cd "$A" && source .venv/bin/activate
cp Makefile /tmp/q56/Makefile.keep
python3 - <<'EOF'
import re
s = open("Makefile").read()
s = s.replace("$(if $(filter $1,$(SDATA_FILES)), -G8)", " -G8")
open("Makefile", "w", newline="\n").write(s)
EOF
rm -rf build; make -k -j16 build/bb2.exe > /tmp/q56/g8global.log 2>&1
echo "global -G8: exe_sha1=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)"
grep -E "error|undefined|multiple|discarded" /tmp/q56/g8global.log | head -5
cp /tmp/q56/Makefile.keep Makefile
