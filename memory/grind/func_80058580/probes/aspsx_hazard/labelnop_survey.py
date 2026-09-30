#!/usr/bin/env python3
"""labelnop_survey.py: in the ORIGINAL binary (asm/funcs), every `nop` that sits right after a
label where the instruction before the label is not a branch/jump (so the nop is not a delay
slot, it can only be an assembler hazard nop). Classify each by what the hazard is, and flag
whether the function is still INCOMPLETE (in engine/queue.json)."""
import re, glob, json, collections
INS = re.compile(r"/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]+ \*/\s+(\S+)\s*(.*)")
BR = {"j", "jal", "jr", "jalr", "b", "beq", "bne", "beqz", "bnez", "bgez", "bltz", "blez", "bgtz",
      "bgezal", "bltzal", "bal"}
LOADS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "lwc2"}
q = json.load(open("engine/queue.json"))
queued = {it["func"] for it in q["items"]}
regs = lambda s: set(re.findall(r"\$[a-z0-9]+", s))
out = collections.defaultdict(list)
for f in sorted(glob.glob("asm/funcs/*.s")):
    fn = f.split("/")[-1][:-2]
    items = []
    for line in open(f):
        m = INS.search(line)
        if m:
            items.append(("i", m.group(2), m.group(3), m.group(1)))
        elif re.match(r"\s*(\.L[0-9A-F]+:|jlabel)", line):
            items.append(("L", None, None, None))
    for k in range(1, len(items) - 2):
        if items[k][0] != "L" or items[k + 1][0] != "i" or items[k + 1][1] != "nop":
            continue
        prev = items[k - 1]
        if prev[0] != "i" or prev[1] in BR:
            continue
        # also skip if prev is itself a delay slot of a branch before it
        if k >= 2 and items[k - 2][0] == "i" and items[k - 2][1] in BR:
            continue
        nxt = items[k + 2] if items[k + 2][0] == "i" else None
        prev2 = items[k - 2] if k >= 2 and items[k - 2][0] == "i" else None
        dest = prev[2].split(",")[0].strip() if prev[2] else ""
        if prev[1] in LOADS and nxt and dest in regs(nxt[2]):
            kind = "load-delay (handled globally since 2026-09-14)"
        elif (prev[1] in ("mflo", "mfhi") or (prev2 and prev2[1] in ("mflo", "mfhi"))) and nxt and nxt[1] in ("mult", "multu", "div", "divu"):
            kind = "mflo/mfhi -> mult/div (NOT handled for .L labels)"
        else:
            kind = "other: %s ; L: nop ; %s" % (prev[1], nxt[1] if nxt else "?")
        out[kind].append((fn, prev[3], fn in queued))
for kind, rows in sorted(out.items(), key=lambda kv: -len(kv[1])):
    inq = sorted({r[0] for r in rows if r[2]})
    print("%4d  %s" % (len(rows), kind))
    print("      in queue: %s" % (", ".join(inq) if inq else "none"))
    print("      e.g. %s" % ", ".join("%s@%s" % (r[0], r[1]) for r in rows[:4]))
