"""Excerpt table for the func_8002D780 Ruling 11 proof: for each dumped spelling, the cross-product
sets (insn uid -> pseudo) from the .sched dump, their flow/lreg/greg lines, and the sched1
SCHEDDBG ADJPRI/PICK lines of those insns.  usage: r11table.py <name>=<dir> ...  > table.txt"""
import re
import sys

SET = re.compile(r"^\(insn (\d+) \d+ \d+ \(set \(reg/v:SI (\d+)\)")
for arg in sys.argv[1:]:
    name, d = arg.split("=", 1)
    sched = open(f"{d}/tu.i.sched.D780", encoding="utf-8").read().split("\n")
    sets = []
    for i, l in enumerate(sched):
        m = SET.match(l)
        if m and i + 1 < len(sched) and "(minus:SI" in sched[i + 1]:
            nxt = sched[i + 1].strip()
            sets.append((m.group(1), m.group(2), nxt))
    # keep the six cross-product sets: the minus sets whose pseudo is a (reg/v) of the test blocks;
    # print all minus sets of reg/v pseudos >= 117 (test region) for transparency
    print(f"== {name}  ({d})")
    flow = open(f"{d}/tu.i.flow.D780", encoding="utf-8").read().split("\n")
    lreg = open(f"{d}/tu.i.lreg.D780", encoding="utf-8").read().split("\n")
    greg = open(f"{d}/tu.i.greg.D780", encoding="utf-8").read()
    disp = greg[greg.index(";; Register dispositions:"):]
    seat = dict(re.findall(r"(\d+) in (\d+)", disp))
    trace = open(f"{d}/sched1.D780.txt", encoding="utf-8").read().split("\n")
    for uid, reg, src in sets:
        # the six cross products subtract two product (mflo) pseudos; the edge differences
        # (dx, az, ...) subtract named locals (reg/v) and are skipped
        if int(reg) < 117 or int(reg) > 140 or not src.startswith("(minus:SI (reg:SI"):
            continue
        fl = next((l for l in flow if l.startswith(f"Register {reg} ")), "")
        ll = next((l for l in lreg if l.startswith(f"Register {reg} ")), "")
        print(f"  insn {uid}: set pseudo {reg} <- {src[:60]}")
        print(f"     flow: {fl}")
        print(f"     lreg: {ll}")
        print(f"     greg seat: {seat.get(reg, '(local-alloc)')}")
        for t in trace:
            if (f"ADJPRI insn={uid} " in t) or (f"picked={uid} " in t):
                print(f"     {t.replace('SCHEDDBG ', '')}")
    print()
