#!/usr/bin/env python3
"""Check jump table label cross-references between .rodata and function .s files."""

import re
import os
import glob

with open("asm/data/800.rodata.s") as f:
    content = f.read()

labels = sorted(set(re.findall(r"\.L(8[0-9a-fA-F]{7})", content)))
print(f"Total jump table labels in .rodata: {len(labels)}")

# Check which function .s files define these labels
func_files = glob.glob("asm/funcs/func_*.s")
label_to_func = {}
for ff in func_files:
    with open(ff) as f:
        fc = f.read()
    for m in re.finditer(r"\.(L8[0-9a-fA-F]{7})", fc):
        lbl = m.group(1)[1:]  # strip leading L->just address
        if lbl in [l for l in labels]:
            fname = os.path.basename(ff)
            label_to_func[lbl] = fname

found = len(label_to_func)
missing = [l for l in labels if l not in label_to_func]
print(f"Labels found in function .s files: {found}")
print(f"Labels NOT found in any func .s file: {len(missing)}")

# Which functions have jump tables?
funcs_with_jtbl = sorted(set(label_to_func.values()))
print(f"\nFunctions with jump table references: {len(funcs_with_jtbl)}")
for ff in funcs_with_jtbl[:10]:
    count = sum(1 for v in label_to_func.values() if v == ff)
    print(f"  {ff} ({count} labels)")
if len(funcs_with_jtbl) > 10:
    print(f"  ... and {len(funcs_with_jtbl) - 10} more")

if missing:
    print(f"\nMissing labels (first 10):")
    for m in sorted(missing)[:10]:
        print(f"  .L{m}")
