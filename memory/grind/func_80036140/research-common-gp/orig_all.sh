#!/bin/bash
# Original toolchain (cc1psx + ASPSX 2.34) on each storage-class probe, vs the target function.
cd "$(dirname "$0")/../.."
for p in "atv_comm.c cdrom_SetMix" "atv_extern.c cdrom_SetMix" "atv_static.c cdrom_SetMix" "atv_split.c cdrom_SetMix" "f35f78_comm.c func_80035F78" "f35f78_extern.c func_80035F78"; do
  set -- $p
  for g in -G8 -G0; do
    printf "%-16s vs %-13s cc1psx %s: " "$1" "$2" "$g"
    bash tmp/research36140/pipe.sh "tmp/research36140/c/$1" "$g" | python3 tmp/research36140/cmp.py "asm/funcs/$2.s" | tail -1
  done
done
