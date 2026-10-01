#!/bin/bash
# fd.sh <obj> <func>: disassembly diff ref vs model for one function
o=$1; f=$2
d() { mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases $1 | awk -v f="<$f>:" '$2==f{p=1;next} /^[0-9a-f]+ </{p=0} p' | sed -E 's/^\s*[0-9a-f]+:\s*//'; }
diff <(d /tmp/q56/refobj/$o.o) <(d /tmp/q56/model/build/src/$o.o)
