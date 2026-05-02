#!/usr/bin/env python3
"""Find functions with adjacent lbu+nop+sll24+sra24 (ASPSX lb expansion) patterns."""
import re
import sys

def clean(l):
    return re.sub(r"/\*.*?\*/", "", l).strip()

def main():
    asmfile = sys.argv[1] if len(sys.argv) > 1 else "asm/6CAC.s"
    with open(asmfile) as f:
        lines = f.readlines()

    # Debug: test regex on known line
    l = clean(lines[27349])
    print(f"Test line: {repr(l)}")
    for i, ch in enumerate(l[:15]):
        print(f"  [{i}] = U+{ord(ch):04X} {repr(ch)}")

    # Try matching
    m = re.match(r"lbu\s+(\S+),", l)
    if m:
        print(f"Regex matched: group(1) = {repr(m.group(1))}")
    else:
        print("Regex did NOT match")

    # Now find all patterns
    current_func = None
    func_patterns = {}
    func_sizes = {}
    func_start = {}

    for i in range(len(lines)):
        stripped = lines[i].strip()
        if stripped.startswith("glabel "):
            current_func = stripped.split()[1]
            func_start[current_func] = i
            func_patterns[current_func] = 0
        if stripped.startswith("endlabel "):
            fn = stripped.split()[1]
            if fn in func_start:
                func_sizes[fn] = i - func_start[fn]

    # Pattern with nop: lbu REG, ... / nop / sll REG,REG,24 / sra REG,REG,24
    for i in range(len(lines) - 3):
        stripped = lines[i].strip()
        if stripped.startswith("glabel "):
            current_func = stripped.split()[1]
        if not current_func:
            continue

        l1 = clean(lines[i])
        l2 = clean(lines[i+1])
        l3 = clean(lines[i+2])
        l4 = clean(lines[i+3]) if i+3 < len(lines) else ""

        m1 = re.match(r"lbu\s+(\S+),", l1)
        if m1 and l2 == "nop":
            reg = re.escape(m1.group(1))
            pat_sll = r"sll\s+" + reg + r"\s*,\s*" + reg + r"\s*,\s*24"
            pat_sra = r"sra\s+" + reg + r"\s*,\s*" + reg + r"\s*,\s*24"
            if re.match(pat_sll, l3) and re.match(pat_sra, l4):
                func_patterns[current_func] = func_patterns.get(current_func, 0) + 1

    # Pattern without nop: lbu REG, ... / sll REG,REG,24 / sra REG,REG,24
    for i in range(len(lines) - 2):
        stripped = lines[i].strip()
        if stripped.startswith("glabel "):
            current_func = stripped.split()[1]
        if not current_func:
            continue

        l1 = clean(lines[i])
        l2 = clean(lines[i+1])
        l3 = clean(lines[i+2])

        m1 = re.match(r"lbu\s+(\S+),", l1)
        if m1:
            reg = re.escape(m1.group(1))
            pat_sll = r"sll\s+" + reg + r"\s*,\s*" + reg + r"\s*,\s*24"
            pat_sra = r"sra\s+" + reg + r"\s*,\s*" + reg + r"\s*,\s*24"
            if re.match(pat_sll, l2) and re.match(pat_sra, l3):
                func_patterns[current_func] = func_patterns.get(current_func, 0) + 1

    results = [(f, c) for f, c in func_patterns.items() if c > 0]
    results.sort(key=lambda x: -x[1])
    print(f"\nFound {len(results)} functions with adjacent lbu+[nop]+sll24+sra24:")
    for func, count in results[:50]:
        size = func_sizes.get(func, "?")
        print(f"  {func}: {count} patterns  (~{size} lines)")

    # Also count total lb instructions (NOT lbu) in INCLUDE_ASM functions
    print("\n--- Functions with real lb (not lbu) ---")
    func_lb = {}
    current_func = None
    for i in range(len(lines)):
        stripped = lines[i].strip()
        if stripped.startswith("glabel "):
            current_func = stripped.split()[1]
            func_lb[current_func] = 0
        if current_func:
            l = clean(lines[i])
            if re.match(r"lb\s+\S+,", l) and not re.match(r"lb[uh]\s", l):
                func_lb[current_func] = func_lb.get(current_func, 0) + 1

    lb_results = [(f, c) for f, c in func_lb.items() if c > 0]
    lb_results.sort(key=lambda x: -x[1])
    print(f"Found {len(lb_results)} functions with lb instructions:")
    for func, count in lb_results[:30]:
        size = func_sizes.get(func, "?")
        print(f"  {func}: {count} lb  (~{size} lines)")

if __name__ == "__main__":
    main()
