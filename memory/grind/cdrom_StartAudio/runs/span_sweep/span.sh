#!/bin/bash
# Task 3/4: object-span sweep over EVERY C consumer of 0x80101E58..EA7 (three TUs), no-FAKE body (IB)
# and the landed FAKE body (FB).  Usage (WSL): bash span.sh [--psx]
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
LAYS="M 58/74 58/78 58/98 58/9C 58/A0 58/A4 58/74/9C 58/9C/9E/A0/A4 58/9C/A0 58/9C/A4 58/98/9C 58/6C/9C 58/64/9C 58/64 58/68 58/6C 58/64/6C"
SPECS=""
for L in $LAYS; do for B in IB FB; do SPECS="$SPECS $L:S:$B"; done; done
python3 tmp/audit-2026-09-29/q2-startaudio/sa.py "$@" --wd tmp/audit-2026-09-29/q2-startaudio/runs_span --files b5_post,b4_post,b5 $SPECS
