#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for d in landing pv_idx pv_idx2 pv_field pv_temp pv_nbits pv_nbits2; do
  echo "== $d"
  python3 tmp/func_800198D0/conf.py tmp/func_800198D0/dumps3/$d idx idx2 field temp nbits nbits2 kidx col step row h0 h1 h2 h3 h4 h5 kflag lo delta mag1 flag mag3 zeros len zeros2 len2
done
