#!/usr/bin/env python3
"""_SendPAD hand-written-asm evidence (2026-10-05). Reproduce: python memory/grind/_SendPAD/asm_evidence.py
Fetches sozud/psy-q 4.0 LIBAPI.LIB (SN LNK) and LIBAPI.A (ECOFF, same release) into tmp/sendpad/ via gh.
Prints, for SENDPAD and its C-compiled lib mate SEND: (1) the ECOFF source-file symbol and gcc markers,
(2) the SN OBJ section-declaration order (ccpsx/GCC vs ASMPSX), (3) _SendPAD's 10 words in the ECOFF
object, the SN OBJ and BB2."""
import os, re, struct, subprocess, sys
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
T = os.path.join(R, "tmp", "sendpad"); os.makedirs(T, exist_ok=True)
sys.path.insert(0, os.path.join(R, "tools", "libscan"))
from psyq_lib import lib_modules, parse_obj
def fetch(p):
    f = os.path.join(T, p.replace("/", "_"))
    if not os.path.exists(f):
        open(f, "wb").write(subprocess.check_output(["gh", "api", "repos/sozud/psy-q/contents/" + p,
                                                    "-H", "Accept: application/vnd.github.raw"]))
    return open(f, "rb").read()
lib, ar = fetch("4.0/PSX/LIB/LIBAPI.LIB"), fetch("4.0/COFF/LIB/LIBAPI.A")
mem, p = {}, 8
while p < len(ar):
    h = ar[p:p + 60]; n = h[:16].decode().strip().rstrip("/"); s = int(h[48:58]); mem[n] = ar[p + 60:p + 60 + s]; p += 60 + s + (s & 1)
def ecoff_text(b):
    _, ns, _, _, _, oh, _ = struct.unpack_from(">HHIIIHH", b, 0); q = 20 + oh
    for _ in range(ns):
        nm = b[q:q + 8].split(b"\0")[0]; sz, ptr = struct.unpack_from(">II", b, q + 16)
        if nm == b".text": return b[ptr:ptr + sz]
        q += 40
print("(1) ECOFF member strings (source file / gcc markers):")
for m in ("sendpad.o", "send.o", "chclrpad.o", "patch.o", "c57.o", "pad.o"):
    st = [x.decode() for x in re.findall(rb"[\x20-\x7e]{4,}", mem[m])]
    src = [x for x in st if re.search(r"\.[cs]$", x)]
    print(f"  {m:11s} src={src[:1]} gcc2_compiled={'gcc2_compiled.' in st} includes={[x for x in st if x.endswith('.h')]}")
print("(2) SN OBJ section-declaration order -> modules:")
groups, sn = {}, {}
for n, d in lib_modules(lib):
    o = parse_obj(d); sn[n] = o
    groups.setdefault(tuple(s["name"] for _, s in sorted(o.sections.items())), []).append(n)
for k, v in groups.items():
    print(f"  {len(v):3d} {' '.join(k)}\n      SENDPAD in group: {'SENDPAD' in v}; SEND in group: {'SEND' in v}; e.g. {v[:8]}")
exe = open(os.path.join(R, "disc", "SLUS_006.63"), "rb").read()
g = exe[0x80079000 - 0x80010000 + 0x800:][:0x28]
c = ecoff_text(mem["sendpad.o"])[:0x28]
s = bytes(next(x["bytes"] for x in sn["SENDPAD"].sections.values() if x["name"] == ".text"))[:0x28]
print("(3) _SendPAD words  ECOFF / SN-OBJ / BB2:")
for i in range(0, 0x28, 4):
    print("  " + "  ".join(f"{struct.unpack_from('<I', b, i)[0]:08x}" for b in (c, s, g)))
st = bytes(next(x["bytes"] for x in sn["SEND"].sections.values() if x["name"] == ".text"))
print("  SEND.OBJ SendPAD (send.c, GCC):", " ".join(f"{struct.unpack_from('<I', st, i)[0]:08x}" for i in range(0, len(st), 4)))
