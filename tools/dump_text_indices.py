#!/usr/bin/env python3
"""Dump numbered TEXT (instruction) indices for a function from the real build pipeline.

Usage:
    python3 tools/dump_text_indices.py <func_name> [<src_file>]

Runs the exact Makefile pipeline (cpp | cc1 | prologue_fix | maspsx) for the
source file containing the function, then extracts and numbers all instructions
within that function's scope (label to .end).

If <src_file> is omitted, greps src/*.c for the function to find it.
"""

import re
import subprocess
import sys
from pathlib import Path


def find_source_file(root, func_name):
    """Find which src/*.c file contains a function (INCLUDE_ASM or definition)."""
    src_dir = root / "src"
    for c_file in sorted(src_dir.glob("*.c")):
        text = c_file.read_text()
        if func_name in text:
            return c_file
    return None


def get_file_stem(src_file):
    return Path(src_file).stem


def read_makefile_var(makefile_text, var_name):
    """Read a Makefile variable value (handles line continuations with \\)."""
    pattern = rf'^{re.escape(var_name)}\s*:=\s*(.*?)(?<!\\)\s*$'
    # First try single-line
    m = re.search(rf'^{re.escape(var_name)}\s*:=\s*(.*)', makefile_text, re.MULTILINE)
    if not m:
        return ''
    # Handle line continuations
    val = m.group(1)
    pos = m.end()
    while val.rstrip().endswith('\\'):
        val = val.rstrip()[:-1]
        # Find next line
        next_nl = makefile_text.find('\n', pos)
        if next_nl == -1:
            break
        next_end = makefile_text.find('\n', next_nl + 1)
        if next_end == -1:
            next_end = len(makefile_text)
        val += ' ' + makefile_text[next_nl + 1:next_end].strip()
        pos = next_end
    return val.strip()


def read_makefile_list(makefile_text, var_name):
    """Read a Makefile variable and split into a list of words."""
    return read_makefile_var(makefile_text, var_name).split()


def build_pipeline_cmd(root, src_file):
    """Build the exact Makefile pipeline command for a source file.

    Reads CC_FLAGS, MASPSX_FLAGS, etc. directly from the Makefile
    to ensure perfect flag matching.
    """
    stem = get_file_stem(src_file)
    makefile = (root / "Makefile").read_text()

    # Read all relevant variables from Makefile
    gp_files = read_makefile_list(makefile, 'GP_FILES')
    expand_lb_files = read_makefile_list(makefile, 'EXPAND_LB_FILES')
    expand_lh_files = read_makefile_list(makefile, 'EXPAND_LH_FILES')
    rodata_align2_files = read_makefile_list(makefile, 'RODATA_ALIGN2_FILES')

    is_gp = stem in gp_files

    # Read exact flag strings from Makefile
    if is_gp:
        cc_flags = read_makefile_var(makefile, 'CC_FLAGS_GP')
        maspsx_flags = read_makefile_var(makefile, 'MASPSX_FLAGS_GP')
    else:
        cc_flags = read_makefile_var(makefile, 'CC_FLAGS')
        maspsx_flags = read_makefile_var(makefile, 'MASPSX_FLAGS')

    # Per-file maspsx additions (matching maspsx_flags_for in Makefile)
    if stem in expand_lb_files:
        maspsx_flags += ' --expand-lb'
    if stem in expand_lh_files:
        maspsx_flags += ' --expand-lh'

    cpp_flags = read_makefile_var(makefile, 'CPP_FLAGS')
    cpp_defs = read_makefile_var(makefile, 'CPP_DEFS')

    # Rodata alignment fix (sed pipe stage)
    rodata_sed = ""
    if stem in rodata_align2_files:
        rodata_sed = ' | sed "s/\\.align\\t3/.align\\t2/"'

    # Use relative paths — cwd is set to project root at call site.
    # Convert src_file to relative if it's absolute.
    try:
        rel_src = Path(src_file).relative_to(root)
    except ValueError:
        rel_src = src_file
    cmd = (
        f"mipsel-linux-gnu-cpp {cpp_flags} {cpp_defs} {rel_src}"
        f" | ./tools/gcc-2.7.2/build/cc1 {cc_flags}"
        f" | python3 ./tools/prologue_fix.py {stem}"
        f" | python3 ./tools/maspsx/maspsx.py {maspsx_flags} -{rodata_sed}"
    )
    return cmd


def is_instruction(line):
    s = line.strip()
    if not s:
        return False
    if s.startswith('.') or s.startswith('#') or s.endswith(':'):
        return False
    if s.startswith('gcc2_compiled'):
        return False
    return True


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 tools/dump_text_indices.py <func_name> [<src_file>]", file=sys.stderr)
        sys.exit(1)

    func_name = sys.argv[1]
    root = Path(__file__).resolve().parent.parent

    if len(sys.argv) >= 3:
        src_file = sys.argv[2]
    else:
        found = find_source_file(root, func_name)
        if not found:
            print(f"ERROR: {func_name} not found in any src/*.c file", file=sys.stderr)
            sys.exit(1)
        src_file = str(found)
        print(f"# Found {func_name} in {found.name}", file=sys.stderr)

    cmd = build_pipeline_cmd(root, src_file)
    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True, cwd=str(root)
    )

    if result.returncode != 0:
        # cc1 may emit warnings to stderr but still produce valid output.
        # Only fail if there is NO stdout at all.
        if not result.stdout.strip():
            print(f"Pipeline failed (no output):\n{result.stderr}", file=sys.stderr)
            sys.exit(1)
        print(f"Pipeline warnings (output still produced):\n{result.stderr[-500:]}", file=sys.stderr)

    asm_text = result.stdout
    in_func = False
    idx = 0
    label_pattern = re.compile(rf'^{re.escape(func_name)}:$')

    for line in asm_text.splitlines():
        s = line.strip()

        if label_pattern.match(s):
            in_func = True
            print(f"--- {s}")
            continue

        if in_func:
            if re.match(rf'^\s*\.end\s+{re.escape(func_name)}', s):
                print(f"--- {s}")
                print(f"\n# Total instructions: {idx}", file=sys.stderr)
                break

            if is_instruction(s):
                print(f"{idx:4d}: {s}")
                idx += 1
            else:
                print(f"    : {s}")


if __name__ == '__main__':
    main()
