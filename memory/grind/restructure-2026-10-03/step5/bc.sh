#!/bin/bash
# bc.sh NAME [BASE] : make (SHA1 gate) then snapshot NAME and compare against BASE (default base).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
make 2>&1 | grep -E "error|Error|warning: conflicting|MISMATCH|OK: bb2|\*\*\*" | head -30
python3 tmp/s5/snap.py "$1" && python3 tmp/s5/cmp.py "${2:-base}" "$1"
