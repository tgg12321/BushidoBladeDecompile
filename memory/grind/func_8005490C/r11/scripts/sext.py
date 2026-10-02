"""print, for func_8005490C in <f.s>: each `subu` immediately followed (within 2 insns) by `sra ...,12`,
and the first insns after each `jal func_8004153C` (delay slot + 2)."""
import re, sys
lines = open(sys.argv[1]).read().split("\n")
st = lines.index("func_8005490C:")
ins = [l.strip() for l in lines[st:] if l.startswith("\t") and not l.strip().startswith(".")]
for i, l in enumerate(ins):
    if l.startswith("subu") and "$sp" not in l and ",$0," not in l:
        for j in (1, 2):
            if i + j < len(ins) and re.match(r"sra\s+\$\d+,\$\d+,12$", ins[i + j]):
                print("   ", l.replace("\t", " "), "|", ins[i + j].replace("\t", " "))
    if l.startswith("jal") and "func_8004153C" in l:
        print("   ", " | ".join(x.replace("\t", " ") for x in ins[i:i + 4]))
