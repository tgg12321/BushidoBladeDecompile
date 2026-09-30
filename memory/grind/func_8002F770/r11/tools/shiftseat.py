"""For each dump dir: the `<< 16` shift's destination pseudo, its source pseudo, and the destination's local seat."""
import re
import sys

for arg in sys.argv[1:]:
    path, f = arg.split(":")
    lreg = open(f"{path}/{f}.lreg").read()
    m = re.search(r"\(set \(reg:SI (\d+)\)\s+\(ashift:SI \(reg(?:/v)?:SI (\d+)\)\s+\(const_int 16\)\)\)", lreg)
    d, s = m.groups()
    loc = re.search(r";; Register %s in (\d+)" % d, lreg)
    print(f"{arg}: shift dest pseudo {d} (local seat {loc.group(1) if loc else 'none'}), source pseudo {s}")
