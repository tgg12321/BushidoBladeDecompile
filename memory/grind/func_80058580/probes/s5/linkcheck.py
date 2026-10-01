#!/usr/bin/env python3
"""linkcheck.py <obj>: link text1b.o standalone at its real addresses (undefined symbols from build/bb2.elf)
and compare its .text / .rodata bytes with build/bb2.bin (== oracle exe body) byte-for-byte."""
import subprocess, sys, re
obj = sys.argv[1]
W = "tmp/func_80058580/full"
TEXT, RODATA = 0x80047ED0, 0x8001585C
LOAD, FILEOFF = 0x80010000, 0  # bb2.bin = text+data image starting at 0x80010000


def run(*a):
    return subprocess.run(a, check=True, capture_output=True, text=True).stdout


syms = {}
for line in run("mipsel-linux-gnu-nm", "build/bb2.elf").splitlines():
    p = line.split()
    if len(p) == 3:
        syms[p[2]] = int(p[0], 16)
undef = [l.split()[-1] for l in run("mipsel-linux-gnu-nm", "-u", obj).splitlines() if l.strip()]
missing = [u for u in undef if u not in syms]
with open(f"{W}/link.ld", "w") as f:
    f.write("_gp = 0x800A30CC;\n")
    for u in undef:
        if u in syms:
            f.write(f"{u} = 0x{syms[u]:08X};\n")
    f.write("SECTIONS {\n")
    f.write(f"  .text 0x{TEXT:08X} : {{ *(.text) }}\n")
    f.write(f"  .rodata 0x{RODATA:08X} : {{ *(.rodata) }}\n")
    f.write("  /DISCARD/ : { *(.reginfo) *(.pdr) *(.mdebug*) *(.comment) *(.note) *(.gnu.attributes) }\n")
    f.write("}\n")
if missing:
    print("MISSING symbols:", missing)
run("mipsel-linux-gnu-ld", "-nostdlib", "--no-check-sections", "-T", f"{W}/link.ld", "-o", f"{W}/t.elf", obj)
run("mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text", f"{W}/t.elf", f"{W}/text.bin")
run("mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".rodata", f"{W}/t.elf", f"{W}/rodata.bin")
img = open("build/bb2.bin", "rb").read()
for name, addr, path in (("text", TEXT, f"{W}/text.bin"), ("rodata", RODATA, f"{W}/rodata.bin")):
    b = open(path, "rb").read()
    ref = img[addr - LOAD: addr - LOAD + len(b)]
    nd = sum(1 for i in range(0, len(b), 4) if b[i:i+4] != ref[i:i+4])
    print(f"{name}: {len(b)} bytes, {nd} differing words")
    shown = 0
    for i in range(0, len(b), 4):
        if b[i:i+4] != ref[i:i+4] and shown < 12:
            print(f"  0x{addr + i:08X}: ours {b[i:i+4][::-1].hex()} ref {ref[i:i+4][::-1].hex()}")
            shown += 1
