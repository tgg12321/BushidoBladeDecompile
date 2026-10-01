#!/bin/bash
# cc1psx_decl_probe.sh: how the ORIGINAL PsyQ cc1psx (-G8) and our cc1 (-G0) emit each file-scope storage
# class (calibration only): static uninit / tentative / zero-init / nonzero-init, in declaration order.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cat > /tmp/q56/declprobe.c <<'EOF'
static int s1;
int t1;
int z1 = 0;
int w1 = 5;
static int s2;
int t2;
static short s3[3];
int t3;
void f(void) { s1++; t1++; z1++; w1++; s2++; t2++; s3[1]++; t3++; }
EOF
echo "=== cc1psx -G8"
bash tools/cc1psx_wrapper.sh -O2 -G8 -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < /tmp/q56/declprobe.c | grep -vE "^\s*#|^$" | grep -E "\.(comm|lcomm|local|sdata|sbss|data|bss|globl|extern|word|space|align)|:$" | head -40
echo "=== our cc1 -G0"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < /tmp/q56/declprobe.c | grep -E "\.(comm|lcomm|local|sdata|sbss|data|bss|globl|extern|word|space|align)|:$" | head -40
echo "=== our cc1 -G8"
tools/gcc-2.7.2/build/cc1 -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < /tmp/q56/declprobe.c | grep -E "\.(comm|lcomm|local|sdata|sbss|data|bss|globl|extern|word|space|align)|:$" | head -40
