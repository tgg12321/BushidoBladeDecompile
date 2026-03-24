"""
Split the monolithic asm/6CAC.s into individual function files in asm/funcs/.

Each function gets its own .s file containing just the function body,
suitable for use with INCLUDE_ASM() from C files.

Usage: python3 tools/split_funcs.py
"""

import os
import re

ASM_FILE = "asm/6CAC.s"
OUTPUT_DIR = "asm/funcs"

def split_functions():
    with open(ASM_FILE, "r") as f:
        lines = f.readlines()

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    funcs = []
    current_func = None
    current_lines = []
    in_func = False

    for line in lines:
        # Detect function start: "glabel func_XXXXXXXX"
        m = re.match(r'^glabel\s+(func_[0-9A-Fa-f]+|[A-Za-z_]\w*)', line)
        if m:
            func_name = m.group(1)
            current_func = func_name
            current_lines = [line]
            in_func = True
            continue

        # Detect function end: "endlabel func_XXXXXXXX"
        m = re.match(r'^endlabel\s+(func_[0-9A-Fa-f]+|[A-Za-z_]\w*)', line)
        if m and in_func:
            current_lines.append(line)
            funcs.append((current_func, current_lines))
            in_func = False
            current_func = None
            current_lines = []
            continue

        if in_func:
            current_lines.append(line)

    # Write individual function files
    for func_name, func_lines in funcs:
        out_path = os.path.join(OUTPUT_DIR, f"{func_name}.s")
        with open(out_path, "w") as f:
            for line in func_lines:
                f.write(line)

    print(f"Split {len(funcs)} functions into {OUTPUT_DIR}/")
    return funcs

if __name__ == "__main__":
    funcs = split_functions()
    # Print summary
    for name, lines in funcs[:5]:
        print(f"  {name}: {len(lines)} lines")
    if len(funcs) > 5:
        print(f"  ... and {len(funcs) - 5} more")
