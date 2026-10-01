#!/bin/bash
# facts2.sh: func_800770B8's .cse / .greg between the func_8006E49C call and the loop, F and F_norestore
D=/tmp/l770m/d
for v in F F_norestore; do
  echo "=== $v .cse"
  awk '/func_8006E49C/{f=1} f{print} /NOTE_INSN_LOOP_BEG/{if(f)exit}' $D/$v/f.cse | grep -v '^$' | grep -v '^ *(nil))*$' | grep -vE 'expr_list|clobber|call \(mem|const_int 16'
  echo "=== $v .greg (hard regs)"
  awk '/func_8006E49C/{f=1} f{print} /NOTE_INSN_LOOP_BEG/{if(f)exit}' $D/$v/f.greg | grep -v '^$' | grep -E '^\(insn|reg|mem' | head -30
done
