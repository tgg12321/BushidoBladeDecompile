#!/bin/bash
# usage: odiff.sh a.o b.o  -> prints differing instruction lines (reloc-agnostic mnemonic+operands)
f() { mipsel-linux-gnu-objdump -d --no-show-raw-insn "$1" | awk '/<func_8001A820>:/{p=1;next} /^$/{if(p)exit} p' | sed 's/^ *[0-9a-f]*:\t//; s/<[^>]*>//g'; }
diff <(f "$1") <(f "$2") | grep -c '^[<>]'
diff <(f "$1") <(f "$2") | head -${3:-40}
