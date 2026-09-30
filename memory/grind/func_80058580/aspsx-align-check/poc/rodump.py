#!/usr/bin/env python3
"""rodump.py <lo> <hi>: words of the reference binary with owning-symbol names from bb2.elf."""
import sys, struct, subprocess
R = "/home/user/BushidoBladeDecompile/"
b = open("/tmp/claude-0/ref_main.bin", "rb").read()
lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
nm = subprocess.run(["mipsel-linux-gnu-nm", "-n", R + "build/bb2.elf"], capture_output=True, text=True).stdout
names = {}
for l in nm.splitlines():
    p = l.split()
    if len(p) == 3 and not p[2].startswith("JTMARK"):
        names.setdefault(int(p[0], 16), []).append(p[2])
for a in range(lo, hi, 4):
    w = struct.unpack_from("<I", b, a - 0x80010000)[0]
    print("%08x ph%d %08x %s" % (a, a % 8, w, ",".join(names.get(a, []))[:60]))
