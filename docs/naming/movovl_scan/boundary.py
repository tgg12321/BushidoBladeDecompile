#!/usr/bin/env python3
"""Pin the code/data boundary in MOVOVL.EXE and enumerate the game-code
functions in the non-library span."""
import struct, os, sys, json, collections

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
exeb = open(os.path.join(REPO, "disc", "STR", "MOVOVL.EXE"), "rb").read()
pc, gp, taddr, tsize = struct.unpack_from("<IIII", exeb, 0x10)
body = exeb[2048:2048 + tsize]
NW = len(body) // 4
w = list(struct.unpack_from("<%dI" % NW, body, 0))

VALID_OP = set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
                0x10, 0x11, 0x12, 0x13, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                0x26, 0x28, 0x29, 0x2A, 0x2B, 0x2E, 0x30, 0x32, 0x38, 0x3A])


def plausible(x):
    return (x >> 26) in VALID_OP


print("plausible-instruction fraction, 256-byte windows from 0x801E3000:")
start = (0x801E3000 - taddr) // 4
for i in range(start, min(NW, (0x801E6000 - taddr) // 4), 64):
    blk = w[i:i + 64]
    f = sum(1 for x in blk if plausible(x)) / len(blk)
    print("  0x%08X  %.2f" % (taddr + i * 4, f))

# --- game functions: non-library span 0x801D8800..0x801DA094 ---
lo, hi = 0x801D8800, 0x801DA094
i0, i1 = (lo - taddr) // 4, (hi - taddr) // 4
JR_RA = 0x03E00008
ends = [taddr + i * 4 for i in range(i0, i1) if w[i] == JR_RA]
print("\ngame-code span 0x%08X..0x%08X (%d bytes)" % (lo, hi, hi - lo))
print("  `jr $ra` count (function returns, incl. multiple per fn):", len(ends))
pro = [taddr + i * 4 for i in range(i0, i1)
       if (w[i] & 0xFFFF0000) == 0x27BD0000 and (w[i] & 0x8000)]
print("  stack-frame prologues:", len(pro))
print("  prologue addrs:", ", ".join("0x%08X" % a for a in pro))
# jal targets inside the game span = called game functions
jals = collections.Counter()
for i in range(NW):
    x = w[i]
    if (x >> 26) == 3:
        t = ((taddr + i * 4) & 0xF0000000) | ((x & 0x03FFFFFF) << 2)
        jals[t] += 1
game_targets = sorted(t for t in jals if lo <= t < hi)
print("  distinct jal targets inside game span:", len(game_targets))
for t in game_targets:
    print("    0x%08X  (%d call sites)" % (t, jals[t]))
print("\n  entry point 0x%08X, header gp=0x%08X" % (pc, gp))
