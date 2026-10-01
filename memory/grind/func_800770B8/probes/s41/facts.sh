#!/bin/bash
# facts.sh: the dump facts the s41 package quotes, for F / F_norestore / F_split0
D=/tmp/l770m/d
echo "F_split0 asm list/arg0:"; grep -n 'addu	\$16,\$16,88\|addu	\$17' $D/F_split0/f.asm | head -3
echo "F asm:"; grep -n 'addu	\$17,\$16,88\|move	\$17\|move	\$3,\$17' $D/F/f.asm | head -4
echo "lreg dispositions:"; for v in F F_split0; do echo " $v: $(grep -E '^;; Register (72|75) in' $D/$v/f.lreg | tr '\n' ' ')"; done
echo "greg dispositions F:"; grep -oE '\b7[25] in [0-9]+' $D/F/f.greg | head -3
echo "F .cse after the call:"; awk '/func_8006E49C/{f=1} f' $D/F/f.cse | grep -E '^\(insn' | head -8
awk '/func_8006E49C/{f=1} f' $D/F/f.cse | grep -A2 -E '^\(insn (77|80|83|86|89|91|96) ' | head -30
echo "F_norestore .cse after the call:"; awk '/func_8006E49C/{f=1} f' $D/F_norestore/f.cse | grep -A2 -E '^\(insn (77|80|83|88|93) ' | head -20
