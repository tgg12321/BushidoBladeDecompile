cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for f in "$@"; do echo -n "$f "; python3 tmp/func_800720FC/rtu.py "$f" 2>&1 | tail -1; done
