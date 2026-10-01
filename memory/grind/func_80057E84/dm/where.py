"""where.py: differing .text words between dm/text1b.o and build/src/text1b.o, mapped to functions,
plus the objdump lines of the first differences."""
import subprocess
from pathlib import Path

out, ref = "tmp/func_80057E84/dm/text1b.o", "build/src/text1b.o"
for o, b in ((out, "/tmp/e84a.bin"), (ref, "/tmp/e84b.bin")):
    subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text", o, b], check=True)
x, y = Path("/tmp/e84a.bin").read_bytes(), Path("/tmp/e84b.bin").read_bytes()
syms = []
for line in subprocess.run(["mipsel-linux-gnu-nm", "-n", ref], capture_output=True, text=True).stdout.splitlines():
    p = line.split()
    if len(p) == 3 and p[1] in "Tt":
        syms.append((int(p[0], 16), p[2]))
diffs = [i for i in range(0, len(x), 4) if x[i:i + 4] != y[i:i + 4]]
byf = {}
for d in diffs:
    f = max((s for s in syms if s[0] <= d), default=(0, "?"))
    byf.setdefault(f[1], []).append(d - f[0])
for f, offs in byf.items():
    print(f, len(offs), [hex(o) for o in offs[:12]])
