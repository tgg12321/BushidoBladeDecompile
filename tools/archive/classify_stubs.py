#!/usr/bin/env python3
"""Classify remaining INCLUDE_ASM stubs by difficulty and blocker type."""
import re
import sys
import os
import glob as globmod

def clean(l):
    return re.sub(r"/\*.*?\*/", "", l).strip()

def load_func_asm(func_name):
    """Load assembly for a function from asm/funcs/."""
    path = f"asm/funcs/{func_name}.s"
    if not os.path.exists(path):
        return None
    with open(path) as f:
        return f.readlines()

def classify_func(func_name, func_lines):
    """Classify a single function's assembly."""
    instrs = 0
    has_lb = 0
    has_lbu_ext = 0
    is_handwritten = False
    has_jal = 0
    has_branch = 0

    cleaned = []
    for fl in func_lines:
        cl = clean(fl)
        if not cl or cl.startswith("glabel") or cl.startswith("endlabel") or cl.startswith("."):
            cleaned.append(("", fl))
            continue
        cleaned.append((cl, fl))
        instrs += 1

    for j, (cl, fl) in enumerate(cleaned):
        if not cl:
            continue

        if re.match(r"lb\s+\S+,", cl) and not re.match(r"lb[uh]\s", cl):
            has_lb += 1

        if re.match(r"lbu\s+\S+,", cl):
            m = re.match(r"lbu\s+(\S+),", cl)
            if m:
                reg = m.group(1)
                for k in range(j+1, min(j+20, len(cleaned))):
                    cl2 = cleaned[k][0]
                    if cl2 and re.match(r"sll\s+" + re.escape(reg) + r"\s*,\s*" + re.escape(reg) + r"\s*,\s*24", cl2):
                        has_lbu_ext += 1
                        break

        # Handwritten markers: add (not addu), addi (not addiu), sub (not subu), syscall
        if re.match(r"add\s", cl) and not re.match(r"add[iu]", cl):
            is_handwritten = True
        if re.match(r"addi\s", cl) and not re.match(r"addiu", cl):
            is_handwritten = True
        if re.match(r"sub\s", cl) and not re.match(r"subu", cl):
            is_handwritten = True
        if cl.startswith("syscall"):
            is_handwritten = True
        if "jalr" in cl and "$t" in cl:
            is_handwritten = True

        if cl.startswith("jal "):
            has_jal += 1
        if re.match(r"b(eq|ne|gtz|lez|ltz|gez)\s", cl):
            has_branch += 1

    # Tier classification
    if is_handwritten:
        tier = "HANDWR"
    elif instrs <= 15:
        tier = "TINY"
    elif instrs <= 30:
        tier = "SMALL"
    elif instrs <= 100:
        tier = "MEDIUM"
    elif instrs <= 300:
        tier = "LARGE"
    else:
        tier = "EXPERT"

    return {
        "instrs": instrs,
        "lb": has_lb,
        "lbu_ext": has_lbu_ext,
        "handwritten": is_handwritten,
        "jal": has_jal,
        "branch": has_branch,
        "tier": tier,
    }

def main():
    # Find INCLUDE_ASM stubs per C file
    stubs_by_file = {}
    for cfile in sorted(globmod.glob("src/*.c")):
        with open(cfile) as f:
            content = f.read()
        stubs = re.findall(r'INCLUDE_ASM\("asm/funcs",\s*(func_\w+)\)', content)
        if stubs:
            stubs_by_file[cfile] = stubs

    print(f"{'Function':<20} {'File':<20} {'Instrs':>6} {'lb':>3} {'lbu+ext':>7} {'Hand':>4} {'Tier':<10} {'Notes'}")
    print("-" * 100)

    total_by_tier = {}
    total_by_file = {}
    all_results = []

    for cfile, stubs in stubs_by_file.items():
        fname = os.path.basename(cfile)
        for stub in stubs:
            func_lines = load_func_asm(stub)
            if func_lines is None:
                print(f"{stub:<20} {fname:<20} {'???':>6} {'':>3} {'':>7} {'':>4} {'MISSING':<10}")
                continue

            info = classify_func(stub, func_lines)

            total_by_tier[info["tier"]] = total_by_tier.get(info["tier"], 0) + 1
            total_by_file[fname] = total_by_file.get(fname, 0) + 1

            notes = []
            if info["lb"]:
                notes.append(f"{info['lb']} lb")
            if info["lbu_ext"]:
                notes.append(f"{info['lbu_ext']} lbu+ext")
            if info["jal"]:
                notes.append(f"{info['jal']} calls")
            if info["branch"]:
                notes.append(f"{info['branch']} branches")

            all_results.append((stub, fname, info))

            print(f"{stub:<20} {fname:<20} {info['instrs']:>6} {info['lb']:>3} {info['lbu_ext']:>7} {'Y' if info['handwritten'] else '':>4} {info['tier']:<10} {', '.join(notes)}")

    print(f"\n=== Summary by Tier ===")
    for tier in ["TINY", "SMALL", "MEDIUM", "LARGE", "EXPERT", "HANDWR"]:
        count = total_by_tier.get(tier, 0)
        if count:
            print(f"  {tier} (<=15/<=30/<=100/<=300/300+/asm): {count}")
    print(f"  TOTAL: {sum(total_by_tier.values())}")

    print(f"\n=== Summary by File ===")
    for fname, count in sorted(total_by_file.items(), key=lambda x: -x[1]):
        print(f"  {fname}: {count}")

    print(f"\n=== Tier x File Matrix ===")
    files = sorted(set(fname for _, fname, _ in all_results))
    tiers = ["TINY", "SMALL", "MEDIUM", "LARGE", "EXPERT", "HANDWR"]
    print(f"{'File':<20}", end="")
    for t in tiers:
        print(f" {t:>7}", end="")
    print(f" {'TOTAL':>7}")
    for fname in files:
        print(f"{fname:<20}", end="")
        row_total = 0
        for t in tiers:
            count = sum(1 for _, fn, info in all_results if fn == fname and info["tier"] == t)
            row_total += count
            print(f" {count:>7}" if count else f" {'':>7}", end="")
        print(f" {row_total:>7}")

if __name__ == "__main__":
    main()
