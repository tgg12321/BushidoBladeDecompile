#!/bin/bash
# Ruling 11 (D)(1)/(2) record for func_8005490C: rot_z and player, reuse vs one-variable-per-value.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_8005490C/dumps"
CC1="../../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -o /dev/null"
sec() { sed -n '/^;; Function func_8005490C/,$p' "$1"; }
echo "# Ruling 11 (D) record, func_8005490C (laneB 2026-10-01). Dumps: memory/grind/func_8005490C/r11/scripts/dumps.sh (run from tmp/func_8005490C/; TU copies built by scripts/mk.py via xbm.sh)"
echo "# (cpp build flags -DPERMUTER | strip_other_fns | tools/gcc-2.7.2/cc1 <build flags> -dr -ds -dc -df -dl -dg;"
echo "#  the instrumented cc1's .s equals tools/gcc-2.7.2/build/cc1's except the options comment)."
echo "# Spellings: reuse = landing body (Ruling 11 comments stripped, r11/bodies/landing_nocomment.c); zsplit = rot_z split per block (player shared);"
echo "#            psplit = player split per write (rot_z shared); split = both split."
echo
echo "## rot_z — local-alloc.c:470-477 (reg_qty -2 only for reg_basic_block >= 0 && reg_n_deaths == 1)"
echo "##         and combine_regs local-alloc.c:1836 (no tie when the SET reg has reg_qty == -1)"
for spec in reuse:76:rot_z reuse:168:subtract_blk1 reuse:308:subtract_loop zsplit:163:rot_z0 zsplit:168:subtract_blk1 zsplit:304:rot_z1 zsplit:309:subtract_loop; do
  IFS=: read t p n <<< "$spec"
  echo "-- $t pseudo $p ($n)"
  sec $t/tu.i.lreg | grep -E "^Register $p |^;; Register $p "
  sec $t/tu.i.greg | grep -oE "(^| )$p in [0-9]+" | head -1 | sed 's/^/   greg: /'
done
echo
echo "## player — global.c find_reg (global.c:952): the loop value crosses the func_800198D0 call, so the one shared allocno"
echo "##          takes call-saved reg 16 (s0) for all three values; init-only allocnos take \$v0 (copy preference)"
for spec in reuse:75:player psplit:82:player0_init0 psplit:83:player1_init1 psplit:235:player2_loop; do
  IFS=: read t p n <<< "$spec"
  echo "-- $t pseudo $p ($n)"
  sec $t/tu.i.lreg | grep -E "^Register $p |^;; Register $p "
  sec $t/tu.i.greg | grep -oE "(^| )$p in [0-9]+" | head -1 | sed 's/^/   greg: /'
  (cd $t && BB2_FINDREG_DEBUG=$p $CC1 tu.i 2>&1 | grep -A6 "FINDREGDBG func=func_8005490C pseudo=$p " | head -8)
done
echo
echo "## target (asm/funcs/func_8005490C.s): 0x80054B24 subu v1,t1,t2 / 0x80054B30 sra t0,v1,12;"
echo "##   0x80054E04 subu v0,t2,t4 / 0x80054E0C sra t0,v0,12; 0x80054A50 / 0x80054A6C move s0,v0 after func_8004153C(0/1),"
echo "##   0x80054CF8 move s0,v0 after func_8004153C(i)."
for t in reuse zsplit psplit split; do
  echo "-- $t f.s (func_8005490C): subu feeding the >>12 of each rotation; the copy after each func_8004153C call"
  python3 ../sext.py $t/f.s
done
