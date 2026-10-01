"""sym.py <out.sym>: list (address, name) from a PSYLINK .sym file (MND v1: addr u32, type u8, len u8, name)."""
import struct, sys
d = open(sys.argv[1], "rb").read()
i, out = 8, []
while i + 6 <= len(d):
    addr = struct.unpack("<I", d[i:i + 4])[0]
    typ, n = d[i + 4], d[i + 5]
    if typ not in (1, 2) or n == 0:
        i += 1
        continue
    name = d[i + 6:i + 6 + n].decode("latin1")
    out.append((addr, name))
    i += 6 + n
base = min(a for a, _ in out if a > 0x80010000) if out else 0
for a, nm in sorted(out):
    print(f"{a:08x} +{a - base:3d} {nm}")
