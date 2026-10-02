"""gpspan.py: Q89 split test input - every gp-relative symbol the ORIGINAL code of each src/text1b.c function
reaches (from asm/funcs/<func>.s, the splat disassembly of the shipped bytes), and which of those symbols
are reached from both sides of the cut immediately before math_RotMatrixZYX. Also lists text1b functions
with no asm/funcs file (unchecked)."""
import re, os, glob

src = open("src/text1b.c", encoding="utf-8").read()
order = []
for m in re.finditer(r'(?m)^INCLUDE_ASM\("asm/funcs", (\w+)\);|^[A-Za-z_][\w \*]*?\b(\w+)\([^;{]*\)\s*\{', src):
    name = m.group(1) or m.group(2)
    if name not in order:
        order.append(name)

# map label -> file (asm/funcs files may be named by address alias)
label_file = {}
for p in glob.glob("asm/funcs/*.s"):
    with open(p, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            mm = re.match(r"\s*glabel\s+(\w+)", line)
            if mm:
                label_file.setdefault(mm.group(1), p)

# named functions: resolve through named_syms.txt (name_ADDR = 0xADDR) to asm/funcs/func_ADDR.s
for line in open("named_syms.txt", encoding="utf-8", errors="replace"):
    mm = re.match(r"\s*(\w+?)_([0-9A-F]{8})\s*=\s*0x([0-9A-Fa-f]{8})", line)
    if mm and mm.group(1) not in label_file:
        q = f"asm/funcs/func_{mm.group(3).upper()}.s"
        if os.path.exists(q):
            label_file[mm.group(1)] = q

cut = order.index("math_RotMatrixZYX")
sides = {0: {}, 1: {}}
missing = []
for i, f in enumerate(order):
    p = label_file.get(f)
    if not p:
        missing.append(f)
        continue
    txt = open(p, encoding="utf-8", errors="replace").read()
    for sym in set(re.findall(r"%gp_rel\(([\w.]+)", txt)):
        sides[0 if i < cut else 1].setdefault(sym, []).append(f)

print(f"functions in text1b: {len(order)}; cut index {cut} (before {order[cut]}); "
      f"head part {order[0]} .. {order[cut - 1]}")
both = sorted(set(sides[0]) & set(sides[1]))
print("gp symbols reached on BOTH sides of the cut:", both if both else "none")
for s in both:
    print("  ", s, "head:", sides[0][s][:4], "tail:", sides[1][s][:4])
print("head-part gp symbols:", sorted(sides[0]))
print("functions without an asm/funcs file (unchecked):", missing)
