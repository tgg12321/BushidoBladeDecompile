"""Summarize the len / temp pseudos (life, seat, preferences, find_reg trace) from cut RTL dumps.
usage: python r11table.py <dumpdir> [...]   (dirs written by dumps_all.sh)"""
import re
import sys


def seat(r, lreg, greg):
    m = re.search(r";; Register %s in (\d+)" % r, lreg)
    if m:
        return "local-alloc " + m.group(1)
    disp = dict(re.findall(r"(\d+) in (\d+)", greg[greg.index("Register dispositions"):]))
    return ("global " + disp[r]) if r in disp else "?"


def findreg(d, r):
    try:
        t = open(f"{d}/findreg_{r}.txt").read()
    except OSError:
        return "-"
    m = re.search(r"func=func_8002CD58 pseudo=%s .*?\n((?:FINDREGDBG  .*\n)+)" % r, t)
    if not m:
        return "-"
    keep = ("conflicts", "own_copy_prefs", "own_full_prefs")
    return "; ".join(l.replace("FINDREGDBG  ", "").strip() for l in m.group(1).splitlines()
                     if l.replace("FINDREGDBG  ", "").split(":")[0] in keep)


for d in sys.argv[1:]:
    base = f"{d}/func_8002CD58"
    lreg = open(base + ".lreg").read()
    greg = open(base + ".greg").read()
    flow = open(base + ".flow").read()
    v1 = re.search(r"\(ltu:SI \(reg/v:SI (\d+)\)\s+\(const_int 16384\)\)", flow).group(1)
    arg = re.findall(r"\(set \(reg:SI 5 a1\)\s+\(reg/v:SI (\d+)\)\)", flow)
    sq = re.findall(r"\(ltu:SI \(reg/v:SI (\d+)\)\s+\(const_int 1024\)\)", flow)
    sh = re.findall(r"\(set \(reg:SI (\d+)\)\s+\(ashift:SI \(reg/v:SI (\d+)\)\s+\(const_int 16\)\)\)", lreg)
    print("==", d)
    rows = [("site-1 |n| (guard)", v1), ("ratan2 arg2 #1 (|a.xz|)", arg[0]), ("ratan2 arg2 #2 (|n.xz|)", arg[1]),
            ("fallback squared length", sq[2])]
    for name, r in rows:
        life = re.search(r"^Register %s used.*$" % r, lreg, re.M).group(0)
        pref = re.search(r"^;; %s preferences:.*$" % r, greg, re.M)
        print(f"   {name}: pseudo {r}, seat {seat(r, lreg, greg)} | {life} | "
              f"{pref.group(0) if pref else 'no preference line'} | find_reg: {findreg(d, r)}")
    for dst, srcr in sh:
        print(f"   `<< 16`: (set (reg {dst}) (ashift (reg/v {srcr}) 16)), {dst} seat {seat(dst, lreg, greg)}")
