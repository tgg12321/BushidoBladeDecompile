#!/usr/bin/env python3
"""Generate C files with INCLUDE_ASM stubs from assembly segment files.

Parses the .s segment file to extract function boundaries and sizes,
computes inter-function NOP padding, and generates a C file with
INCLUDE_ASM macros and PAD_NOPS for matching builds.
"""

import re
import sys


def generate_c_file(asm_file, c_file_name, segment_comment):
    with open(asm_file, "r") as f:
        content = f.read()

    entries = []
    for m in re.finditer(r"nonmatching\s+(\w+),\s+(0x[0-9A-Fa-f]+)", content):
        name = m.group(1)
        size = int(m.group(2), 16)
        if name.startswith("func_"):
            addr = int(name[5:], 16)
        elif name.startswith("D_"):
            addr = int(name[2:], 16)
        else:
            continue
        entries.append((addr, size, name))

    # Find data labels without nonmatching (size will be inferred from gap)
    for m in re.finditer(r"dlabel\s+(\w+)", content):
        name = m.group(1)
        if name.startswith("D_"):
            addr = int(name[2:], 16)
            if not any(e[2] == name for e in entries):
                entries.append((addr, 0, name))

    entries.sort(key=lambda x: x[0])

    # For entries with size 0, infer size from gap to next entry
    for i in range(len(entries)):
        if entries[i][1] == 0 and i < len(entries) - 1:
            inferred = entries[i + 1][0] - entries[i][0]
            entries[i] = (entries[i][0], inferred, entries[i][2])

    lines = []
    lines.append("#define INCLUDE_ASM_USE_MACRO_INC 1")
    lines.append('#include "common.h"')
    lines.append('#include "include_asm.h"')
    lines.append("")
    pad_nop_1 = '#define PAD_NOPS_1 __asm__(".section .text\\n    nop\\n")'
    pad_nop_2 = '#define PAD_NOPS_2 __asm__(".section .text\\n    nop\\n    nop\\n")'
    pad_nop_3 = '#define PAD_NOPS_3 __asm__(".section .text\\n    nop\\n    nop\\n    nop\\n")'
    lines.append("/* Padding NOP macro */")
    lines.append(pad_nop_1)
    lines.append(pad_nop_2)
    lines.append(pad_nop_3)
    lines.append("")
    lines.append(f"/* --- {segment_comment} --- */")
    lines.append("")

    nop_count = 0
    func_count = 0
    data_count = 0

    for i, (addr, size, name) in enumerate(entries):
        is_data = name.startswith("D_")
        is_func = name.startswith("func_")

        if is_func:
            lines.append(f'INCLUDE_ASM("asm/funcs", {name});')
            func_count += 1
        elif is_data:
            lines.append("__asm__(")
            lines.append('    ".section .text\\n"')
            lines.append('    "    .set noat\\n"')
            lines.append('    "    .set noreorder\\n"')
            lines.append(f'    "    .include \\"asm/funcs/{name}.s\\"\\n"')
            lines.append('    "    .set reorder\\n"')
            lines.append('    "    .set at\\n"')
            lines.append(");")
            data_count += 1

        # Calculate gap to next entry
        if i < len(entries) - 1:
            next_addr = entries[i + 1][0]
            end_addr = addr + size
            gap = next_addr - end_addr
            if gap > 0:
                nops = gap // 4
                remaining = nops
                while remaining > 0:
                    if remaining >= 3:
                        lines.append(f"PAD_NOPS_3; /* padding after {name} */")
                        remaining -= 3
                    elif remaining == 2:
                        lines.append(f"PAD_NOPS_2; /* padding after {name} */")
                        remaining -= 2
                    else:
                        lines.append(f"PAD_NOPS_1; /* padding after {name} */")
                        remaining -= 1
                nop_count += nops
            elif gap < 0:
                lines.append(f"/* WARNING: overlap {gap} bytes between {name} and {entries[i+1][2]} */")

    lines.append("")

    with open(f"src/{c_file_name}", "w", newline="\n") as f:
        f.write("\n".join(lines))

    print(f"Generated src/{c_file_name}: {func_count} functions, {data_count} data blobs, {nop_count} total padding NOPs")


if __name__ == "__main__":
    if len(sys.argv) >= 4:
        generate_c_file(sys.argv[1], sys.argv[2], sys.argv[3])
    else:
        # Default: generate both
        generate_c_file(
            "asm/text1b.s", "text1b.c",
            "Functions from text1b segment (0x80047ED0 - 0x80079A30)"
        )
        generate_c_file(
            "asm/6CAC.s", "code6cac.c",
            "Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0)"
        )
