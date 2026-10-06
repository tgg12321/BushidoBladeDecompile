#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
paste <(bash memory/grind/phase2-2026-10-03/lt/f01/objd_b.sh $1 base | grep -v R_MIPS) <(bash memory/grind/phase2-2026-10-03/lt/f01/objd_b.sh $1 | grep -v R_MIPS) | expand -t 34
