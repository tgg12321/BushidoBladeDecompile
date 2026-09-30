cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
export BB2_ALLOC_DEBUG=1
python3 tmp/func_80070F78/dump.py "$@" --instr
