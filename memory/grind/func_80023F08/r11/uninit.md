# Reads with no write on some path (checklist 1001c item 5), landing body
`tools/gcc-2.7.2/build/cc1 <build flags> -Wuninitialized` on the staged src/code6cac_tu2.c (dproof2.sh's tu.i) reports one
warning in func_80023F08: `keys` might be used uninitialized. keys is written in every case of
`switch (cmd & 0x30)` (0x00, 0x10, 0x20, 0x30) and read after it; cmd & 0x30 can only be one of those four values,
so the switch-with-no-match path is infeasible. No warning for temp (each of its values is written before its
reads in the same block).
