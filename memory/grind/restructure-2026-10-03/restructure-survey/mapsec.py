"""Per-object section placement from build/bb2.map + libscan module placement per object."""
import re, json, sys, collections
MAP = "build/bb2.map"
txt = open(MAP).read()
i = txt.index("Linker script and memory map")
body = txt[i:]
lines = body.splitlines()
rows = []  # (sec, addr, size, obj)
pend = None
for ln in lines:
    m = re.match(r"^ (\.\w+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)", ln)
    if m:
        rows.append((m.group(1), int(m.group(2), 16), int(m.group(3), 16), m.group(4)))
        pend = None
        continue
    m = re.match(r"^ (\.\w+)\s*$", ln)
    if m:
        pend = m.group(1); continue
    if pend:
        m = re.match(r"^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)", ln)
        if m:
            rows.append((pend, int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
        pend = None
objs = collections.OrderedDict()
for sec, a, s, o in rows:
    if s == 0 or a < 0x80000000:
        continue
    objs.setdefault(o, {})[sec] = (a, s)
# libscan modules
d = json.load(open("docs/naming/libscan/matches.json"))
mods = []
for r in d["results"]:
    for v in r.get("vaddrs", []) or []:
        mods.append((int(v, 16), r["words"] * 4, r["lib"], r["mod"], r["status"]))
mods.sort()
def owner(addr):
    for o, secs in objs.items():
        if ".text" in secs:
            a, s = secs[".text"]
            if a <= addr < a + s:
                return o
    return None
out = []
for o, secs in objs.items():
    parts = []
    for sec in (".rodata", ".text", ".data", ".sdata", ".sbss", ".bss"):
        if sec in secs:
            a, s = secs[sec]
            parts.append(f"{sec}={a:08X}+{s:X}")
    out.append(f"{o}: " + " ".join(parts))
print("\n".join(out))
print("\n# libscan verbatim/near module placements by object")
byobj = collections.OrderedDict()
for a, s, lib, mod, st in mods:
    o = owner(a)
    oe = owner(a + s - 4) if s else o
    byobj.setdefault(o, []).append((a, s, lib, mod, st, oe))
for o, ms in byobj.items():
    secs = objs.get(o, {})
    ta, ts = secs.get(".text", (0, 0))
    print(f"\n{o} text {ta:08X}..{ta+ts:08X}")
    for a, s, lib, mod, st, oe in ms:
        flag = "" if oe == o else f"  SPANS->{oe}"
        print(f"  {a:08X}..{a+s:08X} {lib}/{mod} {st}{flag}")
