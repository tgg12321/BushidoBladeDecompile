#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
python3 tmp/b60/tucheck.py code6cac_b_tu2 ${1:-tmp/d6a78/b_tu2.c} tmp/d6a78/inc/include 2>&1 | tail -15
python3 tmp/b60/tucheck.py code6cac_b tmp/d6a78/b.c tmp/d6a78/inc/include 2>&1 | tail -3
