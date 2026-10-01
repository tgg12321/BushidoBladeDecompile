"""Width/sign check of func_80023F08's new PracticeMenuRec member types against every ORIGINAL absolute access
(asm/funcs/*.s `op reg, %lo(D_<addr>)` where addr = 0x80101EC8 + k*0x44C + off, k = 0..1) — the record fields the
original code reaches by symbol. usage: python asm_width_check.py > asm_width.txt"""
import glob
import re
import sys

sys.path.insert(0, "tmp/func_80023F08")
from consumer_scan import M  # noqa: E402  (offset -> (member, type))

LOAD = {"lb": ("s8", 1), "lbu": ("u8", 1), "lh": ("s16", 2), "lhu": ("u16", 2), "lw": ("s32", 4),
        "sb": ("x8", 1), "sh": ("x16", 2), "sw": ("x32", 4), "lwl": ("x32", 4), "lwr": ("x32", 4)}
W = {"s8": 1, "u8": 1, "s16": 2, "u16": 2, "s32": 4, "u32": 4, "ptr": 4}
want = {}
for k in range(2):  # records 0 and 1 (0x80101EC8..0x80102313); 0x80102760+ holds other globals (D_80102760, the PadState at 0x80102788)
    for off, (mem, t) in M.items():
        want["D_%08X" % (0x80101EC8 + k * 0x44C + off)] = (k, off, mem, t)
pat = re.compile(r"\s(l[bhw]u?|s[bhw]|lw[lr])\s+\$\w+,\s*%lo\((D_[0-9A-F]{8})\)")
rows = []
for f in sorted(glob.glob("asm/funcs/*.s")):
    for n, line in enumerate(open(f), 1):
        m = pat.search(line)
        if m and m.group(2) in want:
            k, off, mem, t = want[m.group(2)]
            at, aw = LOAD[m.group(1)]
            if aw != W[t]:
                v = "WIDTH"
            elif at[0] in "su" and t[0] in "su" and at != t:
                v = "SIGN(load)"
            else:
                v = "AGREE"
            rows.append((f.split("/")[-1].split("\\")[-1], n, m.group(1), m.group(2), "rec%d+0x%X" % (k, off), mem, t, v))
for r in rows:
    print("%-26s %5d %-4s %s %-11s %-22s %-4s %s" % r)
from collections import Counter
print("#", Counter(r[-1] for r in rows))
