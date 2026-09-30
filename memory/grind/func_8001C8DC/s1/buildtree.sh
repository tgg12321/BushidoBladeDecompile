#!/bin/bash
# buildtree.sh: full clean-driver build of the scratch tree; prints the EXE SHA1.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/c8dc/tree"
source ../../../.venv/bin/activate 2>/dev/null
python3 - <<'PY'
import sys, hashlib
sys.path.insert(0, '.')
from engine import pipeline
exe = pipeline.build_all()
print('SHA1', hashlib.sha1(open(exe, 'rb').read()).hexdigest())
PY
