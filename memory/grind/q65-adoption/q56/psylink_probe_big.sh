#!/bin/bash
# psylink_probe_big.sh: variant of psylink_probe.sh with statics and tentatives larger than 8 bytes, to see
# which section a large .lcomm / .comm goes to and whether it sits inside the per-file static block.
# Objects assembled by Sony ASPSX 2.34 -G8, linked by Sony PSYLINK (psyq3.5). Output: map + decoded addresses.
set -e
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/research36140"
W=/dev/shm/plpb; rm -rf $W; mkdir -p $W; cd $W
mk() {
  n=$1; shift; printf '%s\n' "$@" > $n.s
  bash "$H/aspsx.sh" $n.s $n.obj -G8 >/dev/null 2>&1 || { echo "aspsx failed on $n"; exit 1; }
}
# file A: small static, big static (16), small static, small tentative, big tentative (16)
mk a  "	.lcomm	a_s1,4" "	.lcomm	a_big,16" "	.lcomm	a_s2,4" "	.comm	a_com,4" "	.comm	a_bigc,16" "	.text" "	.globl	fa" "fa:" "	lw	\$2,a_s1" "	la	\$3,a_big" "	lw	\$4,a_s2" "	lw	\$5,a_com" "	la	\$6,a_bigc" "	jr	\$31" "	nop"
# file B: small static, big static (12), small static
mk b  "	.lcomm	b_s1,4" "	.lcomm	b_big,12" "	.lcomm	b_s2,4" "	.text" "	.globl	fb" "fb:" "	lw	\$2,b_s1" "	la	\$3,b_big" "	lw	\$4,b_s2" "	jr	\$31" "	nop"
cat > link.lnk <<'LNK'
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
LNK
python3 - <<'PY'
import pathlib
p = pathlib.Path("link.lnk"); p.write_bytes(p.read_bytes().replace(b"\n", b"\r\n"))
PY
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/psylink.sh" $W "/p /m @link.lnk,out.bin,out.sym,out.map" | tail -4
cat $W/out.map | tr -d '\r' | sed '/^\s*$/d'
echo "=== decoded (gp = start of .sdata group; lui/addiu pairs resolved)"
python3 - <<'PY'
import struct, re
b = open("/dev/shm/plpb/out.bin", "rb").read()
m = open("/dev/shm/plpb/out.map", "rb").read().decode("latin1")
secs = {s: int(a, 16) for a, s in re.findall(r"^\s*([0-9A-F]{8})\s+[0-9A-F]{8}\s+[0-9A-F]{8}\s+[0-9A-F]{8}\s+\S+\s+(\S+)", m, re.M)}
print("sections:", {k: hex(v) for k, v in secs.items()})
gp = secs.get(".SDATA", secs.get(".SBSS"))
names = iter(["fa: a_s1", "fa: a_big", "fa: a_s2", "fa: a_com", "fa: a_bigc", "fb: b_s1", "fb: b_big", "fb: b_s2"])
hi = {}
for i in range(len(b) // 4):
    w = struct.unpack_from("<I", b, i * 4)[0]
    op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    if op == 0x0F:
        hi[rt] = imm << 16
    elif op == 0x09 and rs in hi:
        print(f"{next(names):14} la  -> {0xFFFFFFFF & (hi.pop(rs) + simm):08x}")
    elif op == 0x23 and rs == 28:
        print(f"{next(names):14} lw gp{simm:+d} -> {0xFFFFFFFF & (gp + simm):08x}" if gp else f"lw {simm}($gp)")
PY
