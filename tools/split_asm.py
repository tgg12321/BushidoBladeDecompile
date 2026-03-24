"""Split asm/6CAC.s at a function boundary to allow inserting a C file."""
import re
import sys
import os

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM_FILE = os.path.join(PROJECT_ROOT, "asm", "6CAC.s")


def main():
    start_func = sys.argv[1]  # First func to extract
    end_func = sys.argv[2]    # Last func to extract
    out_asm = sys.argv[3]     # Output asm file for the post-split remainder

    with open(ASM_FILE) as f:
        lines = f.readlines()

    # Find split start
    split_start = None
    for i, line in enumerate(lines):
        if re.match(rf'^nonmatching\s+{re.escape(start_func)}\b', line):
            split_start = i
            break
        if re.match(rf'^glabel\s+{re.escape(start_func)}\b', line):
            split_start = i
            break

    # Find split end
    split_end = None
    for i, line in enumerate(lines):
        if re.match(rf'^endlabel\s+{re.escape(end_func)}\b', line):
            split_end = i + 1
            if split_end < len(lines) and lines[split_end].strip() == "":
                split_end += 1
            break

    if split_start is None or split_end is None:
        print(f"Could not find boundaries: start={split_start}, end={split_end}")
        sys.exit(1)

    # Get function names in range
    func_names = []
    for line in lines[split_start:split_end]:
        m = re.match(r'^glabel\s+(\w+)', line)
        if m:
            func_names.append(m.group(1))

    print(f"Extracting {len(func_names)} functions: {func_names[0]} to {func_names[-1]}")

    # Write first half (truncate 6CAC.s)
    with open(ASM_FILE, "w") as f:
        f.writelines(lines[:split_start])

    # Write second half as new asm file
    header = '.include "macro.inc"\n\n.set noat\n.set noreorder\n\n.section .text, "ax"\n\n'
    with open(out_asm, "w") as f:
        f.write(header)
        in_content = False
        for line in lines[split_end:]:
            if line.startswith("nonmatching") or line.startswith("glabel"):
                in_content = True
            if in_content:
                f.write(line)

    # Print function names for C file generation
    for name in func_names:
        print(f'INCLUDE_ASM("asm/funcs", {name});')

    print(f"\nDone: {ASM_FILE} truncated, {out_asm} created")
    print(f"Functions for C file: {len(func_names)}")


if __name__ == "__main__":
    main()
