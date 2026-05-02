"""
Integrate a verified C file into the build by removing its functions from the
monolithic asm and updating the linker script.

Strategy:
- For each function in the C file, remove it from asm/6CAC.s
- The C file should use INCLUDE_ASM for any non-decompiled functions it contains
- Update bb2.ld to link the C object at the correct position

Usage: python3 tools/integrate_func.py <c_file>

This reads the C object to determine which functions it defines, removes those
from the asm, and updates the linker script.
"""

import sys
import os
import re
import subprocess

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM_FILE = os.path.join(PROJECT_ROOT, "asm", "6CAC.s")
LD_SCRIPT = os.path.join(PROJECT_ROOT, "bb2.ld")
NM = "mipsel-linux-gnu-nm"


def get_obj_text_functions(c_file):
    """Compile the C file and get its text function symbols."""
    # Use the same pipeline as the Makefile
    obj_path = "/tmp/integrate_check.o"
    CPP = "mipsel-linux-gnu-cpp"
    CC1 = os.path.join(PROJECT_ROOT, "tools", "gcc-2.7.2", "build", "cc1")
    MASPSX = os.path.join(PROJECT_ROOT, "tools", "maspsx", "maspsx.py")
    AS = "mipsel-linux-gnu-as"

    cpp_flags = "-Iinclude -undef -Wall -lang-c -fno-builtin"
    cpp_defs = ("-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx "
                "-D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL "
                "-D_LANGUAGE_C -DLANGUAGE_C")
    cc_flags = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"
    maspsx_flags = "--expand-div --aspsx-version=2.34"
    as_flags = "-Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0"

    cmd = (f"{CPP} {cpp_flags} {cpp_defs} {c_file} | "
           f"{CC1} {cc_flags} | "
           f"python3 {MASPSX} {maspsx_flags} | "
           f"{AS} {as_flags} -o {obj_path}")

    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=PROJECT_ROOT)
    if result.returncode != 0:
        print(f"Compilation failed:\n{result.stderr}")
        return None

    # Get function symbols
    nm_result = subprocess.run([NM, "-n", obj_path], capture_output=True, text=True)
    funcs = []
    for line in nm_result.stdout.strip().split("\n"):
        parts = line.split()
        if len(parts) >= 3 and parts[1] == "T":
            funcs.append(parts[2])

    os.unlink(obj_path)
    return funcs


def remove_funcs_from_asm(func_names):
    """Remove function bodies from asm/6CAC.s, leaving just the label stubs."""
    with open(ASM_FILE, "r") as f:
        lines = f.readlines()

    output_lines = []
    skip_until_endlabel = None
    skip_nonmatching = None
    removed = set()

    i = 0
    while i < len(lines):
        line = lines[i]

        # Check for nonmatching line before a function we want to remove
        m = re.match(r'^nonmatching\s+(\w+)', line)
        if m and m.group(1) in func_names:
            skip_nonmatching = m.group(1)
            i += 1
            continue

        # Check for glabel of function to remove
        m = re.match(r'^glabel\s+(\w+)', line)
        if m and m.group(1) in func_names:
            skip_until_endlabel = m.group(1)
            i += 1
            continue

        # Skip function body
        if skip_until_endlabel:
            m = re.match(rf'^endlabel\s+{re.escape(skip_until_endlabel)}', line)
            if m:
                removed.add(skip_until_endlabel)
                skip_until_endlabel = None
            i += 1
            continue

        # Skip blank line after nonmatching
        if skip_nonmatching:
            if line.strip() == "":
                skip_nonmatching = None
                i += 1
                continue
            skip_nonmatching = None

        output_lines.append(line)
        i += 1

    with open(ASM_FILE, "w") as f:
        f.writelines(output_lines)

    return removed


def update_linker_script(c_file, func_names):
    """Add C object to linker script at the correct position in .text."""
    # Get the src path relative to build dir
    # src/foo.c -> build/src/foo.o
    basename = os.path.splitext(os.path.basename(c_file))[0]
    c_obj = f"build/src/{basename}.o"

    with open(LD_SCRIPT, "r") as f:
        content = f.read()

    # Insert C object's .text before the main asm .text
    # Also need .rodata before main rodata
    old = "        build/asm/6CAC.o(.text);"
    new = f"        {c_obj}(.text);\n        build/asm/6CAC.o(.text);"

    if old not in content:
        print(f"  WARNING: Could not find asm text reference in linker script")
        return False

    content = content.replace(old, new)

    # Also add rodata if needed
    old_rodata = "        build/asm/6CAC.o(.rodata);"
    new_rodata = f"        {c_obj}(.rodata);\n        build/asm/6CAC.o(.rodata);"
    content = content.replace(old_rodata, new_rodata)

    # Add data
    old_data = "        build/asm/6CAC.o(.data);"
    new_data = f"        {c_obj}(.data);\n        build/asm/6CAC.o(.data);"
    content = content.replace(old_data, new_data)

    with open(LD_SCRIPT, "w") as f:
        f.write(content)

    print(f"  Updated {LD_SCRIPT}: added {c_obj}")
    return True


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <c_file>")
        sys.exit(1)

    c_file = sys.argv[1]

    print(f"Analyzing {c_file}...")
    func_names = get_obj_text_functions(c_file)
    if func_names is None:
        sys.exit(1)

    print(f"  Found {len(func_names)} functions: {', '.join(func_names)}")

    print(f"Removing functions from asm...")
    removed = remove_funcs_from_asm(set(func_names))
    print(f"  Removed {len(removed)} functions from asm/6CAC.s")

    not_removed = set(func_names) - removed
    if not_removed:
        print(f"  WARNING: could not remove: {', '.join(not_removed)}")

    print(f"Updating linker script...")
    update_linker_script(c_file, func_names)

    print(f"\nDone! Run 'make' to verify.")


if __name__ == "__main__":
    main()
