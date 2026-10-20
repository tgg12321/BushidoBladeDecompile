#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tools/naming_member_apply.py --dry-run tmp/p3m11/pairs.txt > tmp/p3m11/dry.txt 2>&1
echo "exit=$?" >> tmp/p3m11/dry.txt
