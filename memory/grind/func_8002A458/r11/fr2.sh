#!/bin/bash
# find_reg traces for the segment-length pseudo: final (temp = 87), temp_all_own (len = 91),
# temp_temp2_split (reviewer's C1; len pseudo found by its site-2 `>> 3` set).
cd "$(dirname "$0")/../.." || exit 1
source .venv/bin/activate 2>/dev/null
run() {  # variant pseudo
  python3 tmp/func_8002A458/mk.py tmp/func_8002A458/r11v/$1.c a458_$1_fr$2 CC1=tools/gcc-2.7.2/cc1 BB2_FINDREG_DEBUG=$2 BB2_ALLOC_DEBUG=1 >/dev/null
  echo "== $1 pseudo $2"
  grep "^Register $2 " tmp/rtl/a458_$1_fr$2/f.lreg
  grep -A8 "FINDREGDBG func=func_8002A458 pseudo=$2 " tmp/rtl/a458_$1_fr$2/stderr.txt | grep -v "used_so_far\|pass0_used\|pass1_used"
  grep "func=func_8002A458 .* pseudo=$2 " tmp/rtl/a458_$1_fr$2/stderr.txt | grep ALLOCDBG
}
run final 87
run temp_all_own 91
python3 tmp/func_8002A458/mk.py tmp/func_8002A458/r11v/temp_temp2_split.c a458_temp_temp2_split_stock >/dev/null
P=$(grep -B1 "lshiftrt:SI (reg:SI [0-9]*)$" tmp/rtl/a458_temp_temp2_split_stock/f.lreg | grep -o "set (reg/v:SI [0-9]*)" | awk '{print $3}' | tr -d ')' | sed -n 3p)
echo "C1 site-2 small-arm root pseudo: $P"
run temp_temp2_split $P
