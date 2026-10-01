#!/bin/bash
# psylink_probe.sh: how Sony's PSYLINK (psyq3.5) lays out .lcomm (static), .comm (tentative) and .sdata
# objects from several files. Objects assembled by Sony ASPSX 2.34 -G8. Output: the PSYLINK map.
set -e
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/research36140"
W=/dev/shm/plp; rm -rf $W; mkdir -p $W; cd $W
mk() {  # mk NAME lines...
  n=$1; shift; printf '%s\n' "$@" > $n.s
  bash "$H/aspsx.sh" $n.s $n.obj -G8 >/dev/null 2>&1 || { echo "aspsx failed on $n"; exit 1; }
}
# file A: two statics, one tentative, one initialized small datum, one tentative shared with B
mk a  "	.lcomm	a_st1,4" "	.comm	a_com,4" "	.lcomm	a_st2,8" "	.comm	shared_com,4" "	.sdata" "a_sd:" "	.word	0x11" "	.text" "	.globl	fa" "fa:" "	lw	\$2,a_st1" "	lw	\$3,a_com" "	lw	\$4,a_st2" "	lw	\$5,shared_com" "	lw	\$6,a_sd" "	jr	\$31" "	nop"
# file B: one static, one tentative, the shared tentative, initialized small datum
mk b  "	.lcomm	b_st1,4" "	.comm	b_com,4" "	.comm	shared_com,4" "	.sdata" "b_sd:" "	.word	0x22" "	.text" "	.globl	fb" "fb:" "	lw	\$2,b_st1" "	lw	\$3,b_com" "	lw	\$5,shared_com" "	lw	\$6,b_sd" "	jr	\$31" "	nop"
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
python3 - <<'EOF'
import pathlib
p = pathlib.Path("link.lnk"); p.write_bytes(p.read_bytes().replace(b"\n", b"\r\n"))
EOF
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/psylink.sh" $W "/p /m @link.lnk,out.bin,out.sym,out.map" | tail -15
ls -la $W
cat $W/OUT.MAP $W/out.map 2>/dev/null | tr -d '\r' | head -80
