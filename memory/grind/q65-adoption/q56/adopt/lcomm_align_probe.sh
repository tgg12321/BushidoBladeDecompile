#!/bin/bash
# lcomm_align_probe.sh: how Sony ASPSX 2.34 -G8 + PSYLINK (psyq3.5) align `.lcomm` statics of sizes 2/4/6/8
# placed after a 4-byte one (is an 8-byte static 8-aligned or 4-aligned?). Output: the PSYLINK map + loads.
set -e
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/research36140"
W=/dev/shm/lap; rm -rf $W; mkdir -p $W; cd $W
mk() {
  n=$1; shift; printf '%s\n' "$@" > $n.s
  bash "$H/aspsx.sh" $n.s $n.obj -G8 >/dev/null 2>&1 || { echo "aspsx failed on $n"; exit 1; }
}
mk a "	.lcomm	a1,1" "	.lcomm	b8,8" "	.lcomm	c1,1" "	.lcomm	d2,2" "	.lcomm	e1,1" "	.lcomm	f3,3" "	.lcomm	g1,1" "	.lcomm	h5,5" "	.lcomm	i1,1" "	.lcomm	j6,6" "	.lcomm	k1,1" "	.lcomm	l7,7" "	.lcomm	m1,1" "	.lcomm	n4,4" "	.lcomm	o1,1" "	.lcomm	p16,16" "	.lcomm	q1,1" "	.lcomm	r12,12" "	.lcomm	s1x,1" "	.lcomm	t9,9" "	.text" "	.globl	fa" "fa:" "	lb	\$2,a1" "	lb	\$2,b8" "	lb	\$2,c1" "	lb	\$2,d2" "	lb	\$2,e1" "	lb	\$2,f3" "	lb	\$2,g1" "	lb	\$2,h5" "	lb	\$2,i1" "	lb	\$2,j6" "	lb	\$2,k1" "	lb	\$2,l7" "	lb	\$2,m1" "	lb	\$2,n4" "	lb	\$2,o1" "	lb	\$2,p16" "	lb	\$2,q1" "	lb	\$2,r12" "	lb	\$2,s1x" "	lb	\$2,t9" "	jr	\$31" "	nop"
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
EOF
python3 - <<'EOF'
import pathlib
p = pathlib.Path("link.lnk"); p.write_bytes(p.read_bytes().replace(b"\n", b"\r\n"))
EOF
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/psylink.sh" $W "/p /m @link.lnk,out.bin,out.sym,out.map" | tail -5
cat $W/OUT.MAP $W/out.map 2>/dev/null | tr -d '\r' | head -40
python3 "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/plp_decode.py" $W/out.bin 2>/dev/null | head -30 || true
ls $W
