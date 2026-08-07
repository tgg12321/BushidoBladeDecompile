#!/usr/bin/env python3
"""Characterize the MOVOVL.EXE non-library remainder: where does code end and
rodata/data begin, how many game functions sit in the gaps, and which modules of
the expected libs failed to match."""
import struct, os, sys, json, glob, collections

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
from psyq_lib import lib_modules, parse_obj

exeb = open(os.path.join(REPO, "disc", "STR", "MOVOVL.EXE"), "rb").read()
pc, gp, taddr, tsize = struct.unpack_from("<IIII", exeb, 0x10)
body = exeb[2048:2048 + tsize]
NW = len(body) // 4
w = list(struct.unpack_from("<%dI" % NW, body, 0))

P = json.load(open(os.path.join(HERE, "placements.json")))
libspans = [(int(a, 16), int(b, 16)) for a, b, _ in P["regions"]]


def inlib(va):
    return any(a <= va < b for a, b in libspans)


# --- find "jr $ra" (0x03E00008) as a code marker; find last one ---
JR_RA = 0x03E00008
jrs = [i for i, x in enumerate(w) if x == JR_RA]
last_code = taddr + jrs[-1] * 4
print("last `jr $ra` in image: 0x%08X (index %d)" % (last_code, jrs[-1]))

# --- function starts: addiu $sp,$sp,-N  (0x27BD____ with negative imm) ---
def is_prologue(x):
    return (x & 0xFFFF0000) == 0x27BD0000 and (x & 0x8000)


prologues = [taddr + i * 4 for i, x in enumerate(w) if is_prologue(x)]
game_prologues = [a for a in prologues if not inlib(a)]
print("prologues total=%d  in-lib=%d  outside-lib=%d"
      % (len(prologues), len(prologues) - len(game_prologues), len(game_prologues)))

# distribution of outside-lib prologues by 4K bucket
buck = collections.Counter((a >> 12) << 12 for a in game_prologues)
print("outside-lib prologue distribution:")
for k in sorted(buck):
    print("  0x%08X  %d" % (k, buck[k]))

# --- ASCII string density per 4K to locate rodata ---
print("\nprintable-byte density per 4KB block (>0.6 => likely strings/data):")
for off in range(0, tsize, 0x1000):
    blk = body[off:off + 0x1000]
    pr = sum(1 for c in blk if 32 <= c < 127)
    zeros = sum(1 for c in blk if c == 0)
    va = taddr + off
    print("  0x%08X  printable=%.2f zero=%.2f %s"
          % (va, pr / len(blk), zeros / len(blk), "LIB" if inlib(va) else ""))

# --- module-level status for libs of interest ---
M = json.load(open(os.path.join(HERE, "matches.json")))
print("\nper-lib module status:")
by = collections.defaultdict(collections.Counter)
for r in M["results"]:
    by[r["lib"]][r["status"]] += 1
for lib in sorted(by):
    print("  %-10s %s" % (lib, dict(by[lib])))

print("\nLIBPRESS modules:")
for r in M["results"]:
    if r["lib"] == "LIBPRESS":
        print("   %-12s %-12s words=%s %s" % (r["mod"], r["status"], r.get("words"),
                                              r.get("vaddrs") or r.get("best_vaddr") or ""))
