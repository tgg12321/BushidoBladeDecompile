#!/bin/bash
# pseudos.sh: pseudos.py over the dumped bodies -> pseudos.txt; QTYDBG lines of the split values -> qty.txt
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=memory/grind/func_80057E84/r11
D=tmp/func_80057E84/r11/dumps
python3 $R/pseudos.py reuse pv_vtx pv_node pv_route pv_all r_vtx_a r_vtx_b r_vtx_dn r_vtx_up r_node_dn r_node_up r_route_dn r_route_up r_route_pick > $R/pseudos.txt
{
  echo "pv_vtx (132 vtx_a, 133 vtx_b, 134 vtx_dn, 135 vtx_up):"; grep -E "QTYDBG.* reg1=13[2-5] " $D/pv_vtx/pv_vtx.alloc
  echo "pv_node (133 node_dn, 134 node_up):"; grep -E "QTYDBG.* reg1=13[34] " $D/pv_node/pv_node.alloc
  echo "pv_all:"; grep -E "QTYDBG" $D/pv_all/pv_all.alloc | grep -E " reg1=1(3[2-9]|4[0-9]) "
} > $R/qty.txt
{
  for t in reuse pv_vtx pv_node pv_route pv_all; do echo "== $t"; grep "^Register" $D/$t/$t.lreg | head -40; done
} > $R/lreg_registers.txt
for t in reuse pv_vtx pv_node pv_route pv_all; do grep ALLOCDBG $D/$t/$t.alloc > $R/alloc_$t.txt; done
cat $R/pseudos.txt | grep -A9 "== pv_all"
cat $R/qty.txt
