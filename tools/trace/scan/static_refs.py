#!/usr/bin/env python3
"""Static references to the hit-map targets no committed capture reaches.

A function is reached when any phase column of any committed hit map
(docs/naming/runtime/*/hitmap.tsv) is not '.', matched by address. For every
target left, this counts the references the retail code can make to it, read
from the bytes of the main EXE (disc/SLUS_006.63) and the movie overlay
(disc/STR/MOVOVL.EXE, loaded at its header's t_addr):

  jal   a jal whose target is the function
  j     a j whose target is the function
  word  a 32-bit word equal to the address (a pointer in a table)
  hilo  a lui with an addiu of the address's %lo into the same register
        within the next 11 instructions (lui %hi, carried when %lo >= 0x8000),
        or with an ori of %lo (lui of the upper half, no carry)

A function with no reference of any kind has no direct way in: no scenario
can reach it unless code builds its address some other way (an offset added
to another address, a register pair further apart). Names are the current
symbol_addrs.txt names (the last line for an address), else the hit map's.

  python tools/trace/scan/static_refs.py [--all]

prints one TSV row per unreached target (or per target with --all):
function, addr, jal, j, word, hilo, sites.
"""

import argparse
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
RUNTIME = ROOT / "docs" / "naming" / "runtime"
MAIN_EXE = ROOT / "disc" / "SLUS_006.63"
OVERLAY = ROOT / "disc" / "STR" / "MOVOVL.EXE"
HEADER_SIZE = 0x800
PAIR_WINDOW = 11


def read_hitmaps(paths):
    """{addr: name} of every target and the set of reached addresses."""
    targets, reached = {}, set()
    for path in paths:
        lines = path.read_text(encoding="utf-8").splitlines()
        for line in lines[1:]:
            cols = line.split("\t")
            if len(cols) < 2:
                continue
            addr = int(cols[1], 16)
            targets.setdefault(addr, cols[0])
            if any(c != "." for c in cols[2:]):
                reached.add(addr)
    return targets, reached


def read_symbol_names(path):
    names = {}
    if not path.exists():
        return names
    pat = re.compile(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;")
    for line in path.read_text(encoding="utf-8").splitlines():
        m = pat.match(line)
        if m:
            names[int(m.group(2), 16)] = m.group(1)
    return names


def load_exe(data):
    """(load address, words) of a PS-X EXE image."""
    t_addr = struct.unpack_from("<I", data, 0x18)[0]
    n = (len(data) - HEADER_SIZE) // 4
    return t_addr, struct.unpack_from("<%dI" % n, data, HEADER_SIZE)


def references(images, addr):
    """{kind: count} and the sites ('kind@pc') of references to addr."""
    jal = 0x0C000000 | ((addr >> 2) & 0x3FFFFFF)
    j = 0x08000000 | ((addr >> 2) & 0x3FFFFFF)
    lo = addr & 0xFFFF
    # addiu adds the sign-extended %lo, so its lui carries; ori does not.
    upper = {0x09: ((addr + 0x8000) >> 16) & 0xFFFF, 0x0D: addr >> 16 & 0xFFFF}
    counts = {"jal": 0, "j": 0, "word": 0, "hilo": 0}
    sites = []
    for load, words in images:
        for i, w in enumerate(words):
            pc = load + i * 4
            kind = None
            if w == jal:
                kind = "jal"
            elif w == j:
                kind = "j"
            elif w == addr:
                kind = "word"
            elif w >> 26 == 0x0F and w & 0xFFFF in upper.values():
                rt = (w >> 16) & 31
                for v in words[i + 1:i + 1 + PAIR_WINDOW]:
                    if (upper.get(v >> 26) == w & 0xFFFF
                            and (v >> 21) & 31 == rt and v & 0xFFFF == lo):
                        kind = "hilo"
                        break
            if kind:
                counts[kind] += 1
                sites.append("%s@%08X" % (kind, pc))
    return counts, sites


def rows(targets, reached, names, images, every=False):
    for addr in sorted(targets):
        if addr in reached and not every:
            continue
        counts, sites = references(images, addr)
        yield [names.get(addr, targets[addr]), "0x%08X" % addr,
               counts["jal"], counts["j"], counts["word"], counts["hilo"],
               ",".join(sites) or "-"]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--all", action="store_true",
                    help="every target, not only the unreached ones")
    args = ap.parse_args(argv)
    targets, reached = read_hitmaps(sorted(RUNTIME.glob("*/hitmap.tsv")))
    names = read_symbol_names(ROOT / "symbol_addrs.txt")
    images = [load_exe(MAIN_EXE.read_bytes()), load_exe(OVERLAY.read_bytes())]
    out = ["function\taddr\tjal\tj\tword\thilo\tsites"]
    out += ["\t".join(str(c) for c in r)
            for r in rows(targets, reached, names, images, args.all)]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
