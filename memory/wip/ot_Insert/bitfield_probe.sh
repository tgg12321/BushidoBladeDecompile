#!/bin/bash
# Probe GCC 2.7.2's bitfield allocation direction on MIPS LE.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cat > tmp/bf_probe.c <<'EOF'
typedef unsigned int u32;
typedef struct { u32 len : 8; u32 addr : 24; } LenFirst;
typedef struct { u32 addr : 24; u32 len : 8; } AddrFirst;
u32 get_lenfirst_addr(LenFirst *p) { return p->addr; }
u32 get_addrfirst_addr(AddrFirst *p) { return p->addr; }
EOF
tools/gcc-2.7.2/build/cc1 -O2 -G0 -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w tmp/bf_probe.c -o tmp/bf_probe.s 2>&1
echo "== get_lenfirst_addr (struct {len:8; addr:24}) =="
sed -n '/get_lenfirst_addr:/,/jr/p' tmp/bf_probe.s | grep -E 'srl|sll|andi?|lw' | head -4
echo "== get_addrfirst_addr (struct {addr:24; len:8}) =="
sed -n '/get_addrfirst_addr:/,/jr/p' tmp/bf_probe.s | grep -E 'srl|sll|andi?|lw' | head -4
echo "(srl x,8 after lw  => that field sits in HIGH 24 bits; and/andi 0xFFFFFF or sll/srl 8 => LOW 24)"
