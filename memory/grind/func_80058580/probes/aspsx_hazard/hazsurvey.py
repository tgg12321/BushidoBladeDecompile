"""Survey the ORIGINAL binary (asm/funcs, splat of the retail EXE): every mflo/mfhi followed by
exactly one instruction and then a label, and what comes right after the label."""
import re, glob, collections
INS = re.compile(r"/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]+ \*/\s+(\S+)\s*(.*)")
res = collections.Counter(); examples = collections.defaultdict(list)
for f in glob.glob("asm/funcs/*.s"):
    items = []
    for line in open(f):
        m = INS.search(line)
        if m:
            items.append(("i", m.group(2), m.group(1), m.group(3)))
        elif re.match(r"\s*\.L[0-9A-F]+:", line) or re.match(r"\s*jlabel", line):
            items.append(("L", None, None, None))
    for k in range(len(items) - 3):
        a, b, c, d = items[k:k + 4]
        if a[0] == "i" and a[1] in ("mflo", "mfhi") and b[0] == "i" and c[0] == "L" and d[0] == "i":
            nxt = d[1]
            if nxt == "nop" and k + 4 < len(items) and items[k + 4][0] == "i":
                key = "label; nop; " + items[k + 4][1]
            else:
                key = "label; " + nxt
            if b[1] in ("j", "b", "jr", "beq", "bne", "beqz", "bnez", "bgez", "bltz", "blez", "bgtz"):
                key = "[jump/branch between] " + key
            res[key] += 1
            examples[key].append(f.split("/")[-1] + "@" + a[2])
for k, v in res.most_common():
    print("%4d  %-40s e.g. %s" % (v, k, ", ".join(examples[k][:3])))
