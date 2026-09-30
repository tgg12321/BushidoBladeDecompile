set -e
mipsel-linux-gnu-as -march=r3000 -G0 -o t.o t.s
mipsel-linux-gnu-ld -T t.ld -o t.elf t.o 2>&1 || echo "LD FAILED"
mipsel-linux-gnu-nm t.elf | grep foo
mipsel-linux-gnu-objdump -d t.elf | grep -A3 "<f>:" 
