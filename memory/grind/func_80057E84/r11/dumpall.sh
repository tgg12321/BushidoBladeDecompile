#!/bin/bash
# dumpall.sh: dump.sh for the reuse body and the one-variable-per-value twins (gen.sh output)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=memory/grind/func_80057E84/r11
B=tmp/func_80057E84/r11/b
bash $R/dump.sh reuse memory/grind/func_80057E84/candidate.c
for w in pv_vtx pv_node pv_route pv_all pvbs_all r_vtx_a r_vtx_b r_vtx_dn r_vtx_up r_node_dn r_node_up r_route_dn r_route_up r_route_pick; do
  bash $R/dump.sh $w $B/$w.c
done
