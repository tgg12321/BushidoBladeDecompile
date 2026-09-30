"""Summarize the det / sum pseudos (life, seat, preferences) from cut RTL dumps.
usage: python tmp/gte/r11table.py <dumpdir>:<func> [...]"""
import re
import sys


def seat(r, lreg, greg):
    disp = dict(re.findall(r"(\d+) in (\d+)", greg[greg.index("Register dispositions"):]))
    if r in disp:
        return "global " + disp[r]
    m = re.search(r";; Register %s in (\d+)" % r, lreg)
    return ("local " + m.group(1)) if m else "?"


for arg in sys.argv[1:]:
    path, func = arg.split(":")
    base = f"{path}/{func}"
    lreg = open(base + ".lreg").read()
    greg = open(base + ".greg").read()
    flow = open(base + ".flow").read()
    det = re.search(r"\(set \(reg/v:SI (\d+)\)\s+\(ashiftrt:SI \(reg:SI \d+\)\s+\(const_int 12\)\)\)", flow).group(1)
    sm = re.search(r"\(ltu:SI \(reg/v:SI (\d+)\)\s+\(const_int 1024\)\)", flow).group(1)
    print("==", arg)
    for name, r in (("det", det), ("sum", sm)):
        life = re.search(r"^Register %s used.*$" % r, lreg, re.M).group(0)
        pref = re.search(r"^;; %s preferences:.*$" % r, greg, re.M)
        print(f"   {name}: pseudo {r}, seat {seat(r, lreg, greg)} | {life} | {pref.group(0) if pref else 'no preference line'}")
