#!/bin/bash
# subword.sh: test clone - C-defined static-region symbols (0x800A3308..0x800A3618) at addresses not 4-aligned,
# with the object file that defines them (Sony ASPSX places every .lcomm 4-aligned: lcomm_align_probe.sh).
cd /tmp/q56r2/t || exit 1
for o in build/src/*.o; do
  mipsel-linux-gnu-objdump -t "$o" | awk -v f="$(basename "$o" .o)" '$0 ~ /\.sbss/ {print f, $NF}'
done > /tmp/q56r2/sbss_syms.txt
mipsel-linux-gnu-nm build/bb2.elf | awk '{print $3, $1}' | sort > /tmp/q56r2/elf_addrs.txt
python3 - <<'PY'
addr = {}
for l in open("/tmp/q56r2/elf_addrs.txt"):
    p = l.split()
    if len(p) == 2:
        addr.setdefault(p[0], int(p[1], 16))
rows = []
for l in open("/tmp/q56r2/sbss_syms.txt"):
    f, s = l.split()
    a = addr.get(s)
    if a and 0x800A3308 <= a < 0x800A3618 and a % 4:
        rows.append((a, f, s))
for a, f, s in sorted(rows):
    print(f"{a:#x} {f} {s}")
print(len(rows), "static-region C statics not 4-aligned")
PY
