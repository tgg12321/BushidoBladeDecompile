#!/usr/bin/env python3
"""Find lbu+sll24+sra24 (expanded lb) and lhu+sll16+sra16 (expanded lh) patterns in assembly."""
import re
import sys

with open("asm/6CAC.s") as f:
    lines = f.readlines()

current_func = None
results = {}

for i in range(len(lines)):
    line = lines[i].strip()
    m = re.match(r"glabel (func_\w+)", line)
    if m:
        current_func = m.group(1)

    if not current_func:
        continue

    # Look for lbu followed by sll 24 + sra 24 (lb expansion)
    if "  lbu " in line or "\tlbu\t" in line or "\tlbu " in line:
        j = i + 1
        while j < len(lines) and "nop" in lines[j] and "sll" not in lines[j]:
            j += 1
        if j < len(lines):
            sll_line = lines[j].strip()
            if re.search(r"sll\s+.*,\s*24", sll_line) or re.search(r"sll\s+.*,\s*0x18", sll_line):
                k = j + 1
                if k < len(lines):
                    sra_line = lines[k].strip()
                    if re.search(r"sra\s+.*,\s*24", sra_line) or re.search(r"sra\s+.*,\s*0x18", sra_line):
                        if current_func not in results:
                            results[current_func] = []
                        results[current_func].append(("lb_expand", i+1))

    # Look for lhu followed by sll 16 + sra 16 (lh expansion)
    if "  lhu " in line or "\tlhu\t" in line or "\tlhu " in line:
        j = i + 1
        while j < len(lines) and "nop" in lines[j] and "sll" not in lines[j]:
            j += 1
        if j < len(lines):
            sll_line = lines[j].strip()
            if re.search(r"sll\s+.*,\s*16", sll_line) or re.search(r"sll\s+.*,\s*0x10", sll_line):
                k = j + 1
                if k < len(lines):
                    sra_line = lines[k].strip()
                    if re.search(r"sra\s+.*,\s*16", sra_line) or re.search(r"sra\s+.*,\s*0x10", sra_line):
                        if current_func not in results:
                            results[current_func] = []
                        results[current_func].append(("lh_expand", i+1))

print(f"Total functions with expanded load patterns: {len(results)}")
print()
for func, patterns in sorted(results.items()):
    types = set(p[0] for p in patterns)
    count_lb = sum(1 for p in patterns if p[0] == "lb_expand")
    count_lh = sum(1 for p in patterns if p[0] == "lh_expand")
    parts = []
    if count_lb:
        parts.append(f"{count_lb}x lb")
    if count_lh:
        parts.append(f"{count_lh}x lh")
    print(f"  {func}: {', '.join(parts)}")
