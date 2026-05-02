"""
Extend a C file to cover more functions from asm/6CAC.s.

Takes a range of functions (by address) and:
1. Adds INCLUDE_ASM stubs for each function in the range
2. Removes those functions from asm/6CAC.s
3. Optionally replaces specific INCLUDE_ASM stubs with decompiled C code

Usage:
  python3 tools/extend_c.py <c_file> <start_func> <end_func>
  python3 tools/extend_c.py src/ings.c func_80017FA0 func_80018500
"""

import sys
import os
import re
import glob

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM_FILE = os.path.join(PROJECT_ROOT, "asm", "6CAC.s")
FUNCS_DIR = os.path.join(PROJECT_ROOT, "asm", "funcs")


def get_func_addr(name):
    """Extract address from function name."""
    m = re.match(r'func_([0-9A-Fa-f]+)', name)
    return int(m.group(1), 16) if m else None


def get_funcs_in_range(start_addr, end_addr):
    """Get all functions in address range from split function files."""
    funcs = []
    for path in sorted(glob.glob(os.path.join(FUNCS_DIR, "func_*.s"))):
        name = os.path.splitext(os.path.basename(path))[0]
        addr = get_func_addr(name)
        if addr and start_addr <= addr <= end_addr:
            funcs.append((addr, name))
    return funcs


def add_funcs_to_c(c_file, func_names):
    """Append INCLUDE_ASM stubs to the C file."""
    with open(c_file, "a") as f:
        f.write("\n")
        for name in func_names:
            f.write(f'INCLUDE_ASM("asm/funcs", {name});\n')


def remove_funcs_from_asm(first_func, last_func):
    """Remove a contiguous range of functions from asm/6CAC.s."""
    with open(ASM_FILE, "r") as f:
        lines = f.readlines()

    # Find start: nonmatching or glabel of first_func
    start_idx = None
    for i, line in enumerate(lines):
        if re.match(rf'^nonmatching\s+{re.escape(first_func)}\b', line):
            start_idx = i
            break
        if re.match(rf'^glabel\s+{re.escape(first_func)}\b', line):
            start_idx = i
            break

    if start_idx is None:
        print(f"  WARNING: {first_func} not found in asm")
        return False

    # Find end: endlabel of last_func
    end_idx = None
    for i, line in enumerate(lines):
        if re.match(rf'^endlabel\s+{re.escape(last_func)}\b', line):
            end_idx = i + 1
            if end_idx < len(lines) and lines[end_idx].strip() == "":
                end_idx += 1
            break

    if end_idx is None:
        print(f"  WARNING: endlabel {last_func} not found in asm")
        return False

    print(f"  Removing lines {start_idx+1}-{end_idx} from asm/6CAC.s")
    new_lines = lines[:start_idx] + lines[end_idx:]

    with open(ASM_FILE, "w") as f:
        f.writelines(new_lines)

    return True


def main():
    if len(sys.argv) < 4:
        print(f"Usage: {sys.argv[0]} <c_file> <start_func> <end_func>")
        sys.exit(1)

    c_file = sys.argv[1]
    start_func = sys.argv[2]
    end_func = sys.argv[3]

    start_addr = get_func_addr(start_func)
    end_addr = get_func_addr(end_func)

    if not start_addr or not end_addr:
        print("Invalid function names")
        sys.exit(1)

    # Get all functions in range
    funcs = get_funcs_in_range(start_addr, end_addr)
    if not funcs:
        print("No functions found in range")
        sys.exit(1)

    func_names = [name for _, name in funcs]
    print(f"Found {len(func_names)} functions: {func_names[0]} ... {func_names[-1]}")

    # Add to C file
    print(f"Adding INCLUDE_ASM stubs to {c_file}...")
    add_funcs_to_c(c_file, func_names)

    # Remove from asm
    print(f"Removing from asm/6CAC.s...")
    remove_funcs_from_asm(func_names[0], func_names[-1])

    print(f"\nDone! {len(func_names)} functions added to {c_file}")
    print(f"Run 'make' to verify.")


if __name__ == "__main__":
    main()
