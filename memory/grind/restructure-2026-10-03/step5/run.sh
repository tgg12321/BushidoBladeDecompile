#!/bin/bash
# run.sh <python script> args... : run a tmp/s5 script in WSL from the repo root with the venv.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate && python3 "$@"
