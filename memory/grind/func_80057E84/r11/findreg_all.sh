#!/bin/bash
# findreg_all.sh: the find_reg traces banked in findreg.txt (pseudo numbers from pseudos.txt)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=memory/grind/func_80057E84/r11
{
  echo "=== reuse: 132 vtx, 133 node, 103 route"; bash $R/findreg.sh reuse 132 133 103
  echo "=== pv_route: 103 route_dn, 104 route_up, 105 route_pick"; bash $R/findreg.sh pv_route 103 104 105
  echo "=== r_route_pick: 103 route (dn/up), 104 route_pick"; bash $R/findreg.sh r_route_pick 103 104
  echo "=== r_route_dn: 104 route_dn, 103 route (up/pick)"; bash $R/findreg.sh r_route_dn 104 103
  echo "=== r_vtx_a: 132 vtx (b/dn/up)"; bash $R/findreg.sh r_vtx_a 132
  echo "=== r_vtx_dn: 132 vtx (a/b/up)"; bash $R/findreg.sh r_vtx_dn 132
} > $R/findreg.txt 2>&1
cat $R/findreg.txt
