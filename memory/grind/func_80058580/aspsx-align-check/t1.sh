#!/bin/bash
# t1.sh <secdirective> : two objects, A = 4 bytes of rodata, B = word, .align 3, word.
# Link A then B with Sony PSYLINK; print where B's words land.
set -e
P=/tmp/claude-0/psyq
W=$P/t1; rm -rf $W; mkdir -p $W
cp $P/psyq3.5/ASPSX.EXE $P/psyq3.5/PSYLINK.EXE $W/
SEC="$1"
printf '\t%s\n\t.word 0x11111111\n' "$SEC" | sed 's/$/\r/' > $W/A.S
printf '\t%s\n\t.word 0xB0B0B0B0\n\t.align 3\n\t.word 0xB1B1B1B1\n' "$SEC" | sed 's/$/\r/' > $W/B.S
bash $P/dos.sh $W "ASPSX.EXE -o A.OBJ A.S" | grep -i "error" || true
bash $P/dos.sh $W "ASPSX.EXE -o B.OBJ B.S" | grep -i "error" || true
bash $P/dos.sh $W "PSYLINK.EXE /p /o\$80010000 A.OBJ B.OBJ,OUT.BIN,OUT.SYM,OUT.MAP"
ls -la $W/*.BIN $W/*.OBJ 2>/dev/null
[ -f $W/OUT.BIN ] && xxd $W/OUT.BIN | head
