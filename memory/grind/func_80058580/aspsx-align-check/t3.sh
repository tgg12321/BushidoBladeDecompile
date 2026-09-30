#!/bin/bash
# t3.sh: three objects in .rdata. A = 4 bytes; B = word, .align 3, word; C = .align 3, word.
set -e
P=/tmp/claude-0/psyq
W=$P/t3; rm -rf $W; mkdir -p $W
cp $P/psyq3.5/ASPSX.EXE $P/psyq3.5/PSYLINK.EXE $W/
printf '\t.rdata\n\t.word 0x11111111\n' | sed 's/$/\r/' > $W/A.S
printf '\t.rdata\n\t.word 0xB0B0B0B0\n\t.align 3\n\t.word 0xB1B1B1B1\n' | sed 's/$/\r/' > $W/B.S
printf '\t.rdata\n\t.align 3\n\t.word 0xC0C0C0C0\n' | sed 's/$/\r/' > $W/C.S
for f in A B C; do bash $P/dos.sh $W "ASPSX.EXE -o $f.OBJ $f.S" | grep -i " error" || true; done
bash $P/dos.sh $W "PSYLINK.EXE /p /o\$80010000 A.OBJ B.OBJ C.OBJ,OUT.BIN,OUT.SYM,OUT.MAP" | grep -i "error" || true
od -A x -t x4 $W/OUT.BIN
