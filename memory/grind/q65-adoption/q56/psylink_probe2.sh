#!/bin/bash
# psylink_probe2.sh: Sony PSYLINK 2.37 (psyq3.5, dosemu2) on ASPSX 2.34 -G8 objects:
#   file A: small static, a 12-byte static (> 8: not small data), small static, tentative
#   file B: small static, a 16-byte static, small static
# Where do the >8-byte statics go (.bss or .sbss)? Are the per-file .sbss blocks still contiguous?
# Each function takes the ADDRESS of every object (la) so the map/.text shows where each one landed.
set -e
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/research36140"
W=/dev/shm/plp2; rm -rf $W; mkdir -p $W; cd $W
mk() { n=$1; shift; printf '%s\n' "$@" > $n.s; bash "$H/aspsx.sh" $n.s $n.obj -G8 >/dev/null 2>&1 || { echo "aspsx failed $n"; exit 1; }; }
mk a "	.lcomm	a_s1,4" "	.lcomm	a_big,12" "	.lcomm	a_s2,4" "	.comm	a_com,4" "	.text" "	.globl	fa" "fa:" \
     "	la	\$2,a_s1" "	la	\$3,a_big" "	la	\$4,a_s2" "	la	\$5,a_com" "	jr	\$31" "	nop"
mk b "	.lcomm	b_s1,4" "	.lcomm	b_big,16" "	.lcomm	b_s2,2" "	.text" "	.globl	fb" "fb:" \
     "	la	\$2,b_s1" "	la	\$3,b_big" "	la	\$4,b_s2" "	jr	\$31" "	nop"
cat > link.lnk <<'EOF'
	org	$80010000
text	group
data	group
sdata	group
sbss	group	bss
bss	group	bss
	section	.text,text
	section	.data,data
	section	.sdata,sdata
	section	.sbss,sbss
	section	.bss,bss
	include	"a.obj"
	include	"b.obj"
EOF
python3 -c "import pathlib; p=pathlib.Path('link.lnk'); p.write_bytes(p.read_bytes().replace(b'\n', b'\r\n'))"
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/psylink.sh" $W "/p /m @link.lnk,out.bin,out.sym,out.map" | tail -3
tr -d '\r' < out.map | sed -n '1,12p'
tr -d '\r' < out.map | grep -A20 "address order"
# decode the la pairs (lui hi / addiu lo) in out.bin to recover every object's address, statics included
python3 - <<'EOF'
import struct
b = open("out.bin", "rb").read()
w = struct.unpack("<%dI" % (len(b) // 4), b[:len(b) // 4 * 4])
names = ["a_s1", "a_big", "a_s2", "a_com", None, None, "b_s1", "b_big", "b_s2"]
hi = {}
res = []
for i, x in enumerate(w[:24]):
    op = x >> 26
    if op == 0x0F:
        hi[(x >> 16) & 31] = (x & 0xFFFF) << 16
    elif op == 0x09 and ((x >> 21) & 31) in hi:
        r = (x >> 21) & 31
        v = hi.pop(r) + (x & 0xFFFF if x & 0x8000 == 0 else (x & 0xFFFF) - 0x10000)
        res.append(v & 0xFFFFFFFF)
labels = ["a_s1", "a_big", "a_s2", "a_com", "b_s1", "b_big", "b_s2"]
for n, v in zip(labels, res):
    print(f"  {n:6} at {v:08x}")
EOF
