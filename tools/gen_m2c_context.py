#!/usr/bin/env python3
"""
Generate m2c context file with inferred function signatures.

Analyzes each asm stub to determine:
- If a0 is used as a struct pointer (GameObj*)
- If a1/a2 are used as struct pointers
- Return type (void vs s32)

Writes include/m2c_context.h with struct definitions + function signatures.

Usage: python3 tools/gen_m2c_context.py
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_DIR = ROOT / "asm" / "funcs"

# Types that are safe to use in m2c context (defined in the header)
KNOWN_TYPES = {
    'void', 'u8', 's8', 'u16', 's16', 'u32', 's32', 'u64', 's64',
    'vu8', 'vs8', 'vu16', 'vs16', 'vu32', 'vs32',
    'GameObj', 'Vec3s16', 'Vec3s32', 'Vec2s16',
}

# Struct definition — hardcoded to avoid circular read/overwrite issues
STRUCT_DEFS = r"""
/* -----------------------------------------------------------------------
 * Main game object struct (accessed via a0/s0 in ~340 functions)
 *
 * Offsets determined from load/store instruction patterns across all asm
 * stubs. Fields with mixed lw/lh access at word-aligned offsets indicate
 * the field is a pair of s16s that is sometimes loaded as a full word.
 *
 * Range 0x00-0x03: status/flag bytes
 * Range 0x04-0x1F: position, rotation, velocity (s16 pairs -> s32 words)
 * Range 0x20-0x3F: additional transform data
 * Range 0x40-0x5F: animation/physics vectors (s16 pairs)
 * Range 0x60-0x6F: extended state
 * Range 0x70-0xBF: matrices, pointers, extra data
 * Range 0xC0-0xFF: more data fields
 * ----------------------------------------------------------------------- */
typedef struct GameObj {
    /* 0x00 */ u8 field_00;
    /* 0x01 */ u8 field_01;
    /* 0x02 */ s16 field_02;
    /* 0x04 */ s16 field_04;
    /* 0x06 */ s16 field_06;
    /* 0x08 */ s16 field_08;
    /* 0x0A */ s16 field_0A;
    /* 0x0C */ s16 field_0C;
    /* 0x0E */ s16 field_0E;
    /* 0x10 */ s16 field_10;
    /* 0x12 */ s16 field_12;
    /* 0x14 */ s16 field_14;
    /* 0x16 */ s16 field_16;
    /* 0x18 */ s32 field_18;
    /* 0x1C */ s32 field_1C;
    /* 0x20 */ s32 field_20;
    /* 0x24 */ s32 field_24;
    /* 0x28 */ s32 field_28;
    /* 0x2C */ s32 field_2C;
    /* 0x30 */ s16 field_30;
    /* 0x32 */ s16 field_32;
    /* 0x34 */ s16 field_34;
    /* 0x36 */ s16 field_36;
    /* 0x38 */ s16 field_38;
    /* 0x3A */ s16 field_3A;
    /* 0x3C */ s16 field_3C;
    /* 0x3E */ s16 field_3E;
    /* 0x40 */ s16 field_40;
    /* 0x42 */ s16 field_42;
    /* 0x44 */ s32 field_44;
    /* 0x48 */ s32 field_48;
    /* 0x4C */ s32 field_4C;
    /* 0x50 */ s32 field_50;
    /* 0x54 */ s16 field_54;
    /* 0x56 */ s16 field_56;
    /* 0x58 */ s32 field_58;
    /* 0x5C */ s16 field_5C;
    /* 0x5E */ s16 field_5E;
    /* 0x60 */ s32 field_60;
    /* 0x64 */ s32 field_64;
    /* 0x68 */ s32 field_68;
    /* 0x6C */ s32 field_6C;
    /* 0x70 */ s32 field_70;
    /* 0x74 */ s32 field_74;
    /* 0x78 */ s32 field_78;
    /* 0x7C */ s32 field_7C;
    /* 0x80 */ s32 field_80;
    /* 0x84 */ s16 field_84;
    /* 0x86 */ s16 field_86;
    /* 0x88 */ s16 field_88;
    /* 0x8A */ s16 field_8A;
    /* 0x8C */ s32 field_8C;
    /* 0x90 */ s32 field_90;
    /* 0x94 */ s32 field_94;
    /* 0x98 */ s32 field_98;
    /* 0x9C */ s32 field_9C;
    /* 0xA0 */ s32 field_A0;
    /* 0xA4 */ s32 field_A4;
    /* 0xA8 */ s32 field_A8;
    /* 0xAC */ s32 field_AC;
    /* 0xB0 */ s32 field_B0;
    /* 0xB4 */ s32 field_B4;
    /* 0xB8 */ s32 field_B8;
    /* 0xBC */ s32 field_BC;
    /* 0xC0 */ s32 field_C0;
    /* 0xC4 */ s32 field_C4;
    /* 0xC8 */ s32 field_C8;
    /* 0xCC */ s32 field_CC;
    /* 0xD0 */ s32 field_D0;
    /* 0xD4 */ s32 field_D4;
    /* 0xD8 */ s32 field_D8;
    /* 0xDC */ s32 field_DC;
    /* 0xE0 */ s32 field_E0;
    /* 0xE4 */ s32 field_E4;
    /* 0xE8 */ s32 field_E8;
    /* 0xEC */ s32 field_EC;
    /* 0xF0 */ s32 field_F0;
    /* 0xF4 */ s32 field_F4;
    /* 0xF8 */ s16 field_F8;
    /* 0xFA */ s16 field_FA;
    /* 0xFC */ s32 field_FC;
} GameObj;

/* Short vector types used for angles, positions, etc. */
typedef struct Vec3s16 {
    s16 x;
    s16 y;
    s16 z;
} Vec3s16;

typedef struct Vec3s32 {
    s32 x;
    s32 y;
    s32 z;
} Vec3s32;

typedef struct Vec2s16 {
    s16 x;
    s16 y;
} Vec2s16;
"""


def get_field_accesses(asm_path, reg):
    """Get all offsets accessed via a given register in a function."""
    content = open(asm_path).read()
    offsets = {}
    for m in re.finditer(
        r'(lw|sw|lh|sh|lb|sb|lhu|lbu)\s+\$\w+,\s*(-?(?:0x[0-9a-fA-F]+|\d+))\(\$' + reg + r'\)',
        content
    ):
        instr, offset_str = m.groups()
        offset = int(offset_str, 16) if '0x' in offset_str.lower() else int(offset_str)
        if 0 <= offset < 512:
            if offset not in offsets:
                offsets[offset] = set()
            offsets[offset].add(instr)
    return offsets


def check_reg_saved(asm_path, src_reg, dst_reg):
    """Check if src_reg is saved to dst_reg early in the function."""
    content = open(asm_path).read()
    lines = content.split('\n')[:15]
    for line in lines:
        if re.search(r'addu\s+\$' + dst_reg + r',\s*\$' + src_reg + r',\s*\$zero', line) or \
           re.search(r'move\s+\$' + dst_reg + r',\s*\$' + src_reg, line) or \
           re.search(r'or\s+\$' + dst_reg + r',\s*\$' + src_reg + r',\s*\$zero', line):
            return True
    return False


def is_struct_pointer(asm_path, reg):
    """Check if reg is used as a GameObj struct pointer.

    Also checks if the register is saved to a callee-saved register
    (a0->s0, a1->s1, etc.) and then used from there.
    """
    offsets = get_field_accesses(asm_path, reg)

    # Check callee-saved register mapping
    save_map = {'a0': ['s0', 's1', 's2'], 'a1': ['s1', 's2', 's3'], 'a2': ['s2', 's3']}
    for dst in save_map.get(reg, []):
        if check_reg_saved(asm_path, reg, dst):
            saved_offsets = get_field_accesses(asm_path, dst)
            offsets.update(saved_offsets)
            break

    # Need 3+ distinct field accesses to be a struct pointer
    if len(offsets) < 3:
        return False

    # Check that offsets are in GameObj range (0x00-0xFC)
    gameobj_offsets = [o for o in offsets if 0 <= o <= 0xFC]
    return len(gameobj_offsets) >= 3


def infer_return_type(asm_path):
    """Check if function returns a value in $v0."""
    content = open(asm_path).read()
    lines = content.strip().split('\n')

    # Find the jr $ra instruction
    jr_idx = None
    for i, line in enumerate(lines):
        if re.search(r'jr\s+\$ra', line):
            jr_idx = i
            break

    if jr_idx is None:
        return 'void'

    # Check the delay slot and 2 instructions before jr $ra
    check_range = lines[max(0, jr_idx - 3):jr_idx + 2]
    for line in check_range:
        # v0 is explicitly set near the return
        if re.search(r'(lw|lh|lhu|lb|lbu|addiu|addu|subu|sll|srl|sra|ori|andi|sltu?i?|move)\s+\$v0,', line):
            return 's32'

    return 'void'


def count_args_conservative(asm_path):
    """Count arguments by checking which $aN regs are read before being overwritten.

    Conservative: only counts if the register is clearly used as input.
    """
    content = open(asm_path).read()
    lines = content.split('\n')

    # Track first use of each arg register
    first_read = {}
    first_write = {}
    line_num = 0

    for line in lines:
        # Skip non-instruction lines
        if not re.search(r'\*/\s+\w', line):
            continue
        line_num += 1

        for reg in ['a0', 'a1', 'a2', 'a3']:
            # Skip if both already found
            if reg in first_read and reg in first_write:
                continue

            # Check for read: used as base register or source operand
            is_read = False
            if re.search(r'\(\$' + reg + r'\)', line):
                is_read = True
            # Source in arithmetic/logic: op $dst, $aN, ... or op $dst, ..., $aN
            if re.search(r',\s*\$' + reg + r'(?:\s|,|$)', line):
                is_read = True
            # Used in sw/sh/sb as source: sw $aN, offset($X)
            if re.search(r'(sw|sh|sb)\s+\$' + reg + r',', line):
                is_read = True

            # Check for write: reg is destination
            is_write = re.search(
                r'(lw|lh|lhu|lb|lbu|addiu|addu|subu|sll|srl|sra|move|or|andi|ori|lui|mflo|mfhi)\s+\$' + reg + r',',
                line
            ) is not None

            if is_read and reg not in first_read:
                first_read[reg] = line_num
            if is_write and reg not in first_write:
                first_write[reg] = line_num

    # An arg register is an input if it's read before (or without) being written
    arg_regs = ['a0', 'a1', 'a2', 'a3']
    count = 0
    for reg in arg_regs:
        if reg in first_read:
            if reg not in first_write or first_read[reg] <= first_write[reg]:
                count += 1
            else:
                break  # Written before read = not an input arg
        else:
            break  # Not used at all = no more args

    return max(count, 0)


def infer_arg_type(asm_path, reg):
    """Infer the type of an argument register."""
    if is_struct_pointer(asm_path, reg):
        return 'GameObj *'
    return 's32'


def find_all_stubs():
    """Find all INCLUDE_ASM stubs."""
    stubs = []
    for src in sorted(ROOT.glob("src/*.c")):
        with open(src) as f:
            for line in f:
                m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
                if m:
                    stubs.append(m.group(1))
    return stubs


def sanitize_type(type_str):
    """Replace unknown types with s32 or void*."""
    # Strip pointer suffix for checking
    base = type_str.replace('*', '').strip()
    if base in KNOWN_TYPES or base == '':
        return type_str
    # Replace unknown pointer types with void*
    if '*' in type_str:
        return 'void *'
    return 's32'


def find_decompiled_signatures():
    """Extract function signatures from already-decompiled code in src/*.c."""
    sigs = {}
    for src in sorted(ROOT.glob("src/*.c")):
        content = open(src).read()
        for m in re.finditer(
            r'^(\w[\w\s\*]*?)\s+(func_[0-9A-Fa-f]+)\s*\(([^)]*)\)\s*\{',
            content, re.MULTILINE
        ):
            ret_type, name, params = m.groups()
            ret_type = ret_type.strip()

            # Sanitize types
            ret_type = sanitize_type(ret_type)
            if params.strip() and params.strip() != 'void':
                sanitized_params = []
                for param in params.split(','):
                    param = param.strip()
                    # Split type from name
                    parts = param.rsplit(' ', 1)
                    if len(parts) == 2:
                        ptype, pname = parts
                        ptype = sanitize_type(ptype.strip())
                        sanitized_params.append(f"{ptype} {pname}")
                    else:
                        sanitized_params.append(sanitize_type(param))
                params = ', '.join(sanitized_params)

            sigs[name] = f"{ret_type} {name}({params})"
    return sigs


def main():
    stubs = find_all_stubs()
    decompiled_sigs = find_decompiled_signatures()

    print(f"Stubs: {len(stubs)}, Decompiled signatures: {len(decompiled_sigs)}")

    # Infer signatures for stubs
    signatures = {}
    gameobj_count = 0

    for func in sorted(stubs):
        asm_file = ASM_DIR / f"{func}.s"
        if not asm_file.exists():
            continue

        nargs = count_args_conservative(asm_file)
        ret_type = infer_return_type(asm_file)

        args = []
        for reg in ['a0', 'a1', 'a2', 'a3'][:nargs]:
            arg_type = infer_arg_type(asm_file, reg)
            if arg_type == 'GameObj *':
                gameobj_count += 1
            args.append(arg_type)

        param_str = ', '.join(args) if args else 'void'
        signatures[func] = f"{ret_type} {func}({param_str})"

    # Merge with decompiled signatures
    all_sigs = {}
    all_sigs.update(decompiled_sigs)
    all_sigs.update(signatures)  # stubs override if conflict (shouldn't happen)

    print(f"Stubs with GameObj* parameter: {gameobj_count}")

    # Generate the context file
    lines = []
    lines.append("/* m2c context: auto-generated by tools/gen_m2c_context.py")
    lines.append(" *")
    lines.append(" * This file is NOT included by the build — it is only used by m2c")
    lines.append(" * via --context to improve decompilation output.")
    lines.append(" */")
    lines.append("")

    # Basic types
    for typedef in [
        "typedef unsigned char u8;",
        "typedef signed char s8;",
        "typedef unsigned short u16;",
        "typedef signed short s16;",
        "typedef unsigned int u32;",
        "typedef signed int s32;",
        "typedef unsigned long long u64;",
        "typedef signed long long s64;",
        "typedef volatile u8 vu8;",
        "typedef volatile s8 vs8;",
        "typedef volatile u16 vu16;",
        "typedef volatile s16 vs16;",
        "typedef volatile u32 vu32;",
        "typedef volatile s32 vs32;",
    ]:
        lines.append(typedef)
    lines.append("")

    # Struct definitions
    lines.append(STRUCT_DEFS.strip())
    lines.append("")

    # Global arrays (indexed access patterns found in asm)
    lines.append("/* Global arrays (indexed access patterns found in asm) */")
    lines.append("extern s16 D_800973FC[];       /* 4096-entry sine LUT, 19 funcs */")
    lines.append("extern s32 D_800A9A10[];       /* 7 funcs */")
    lines.append("extern s32 D_800A3860[];       /* 6 funcs */")
    lines.append("extern s32 D_80106F28[];       /* 6 funcs */")
    lines.append("extern s16 D_80101F12[];       /* 5 funcs */")
    lines.append("extern s32 D_800A11DC[];       /* 5 funcs */")
    lines.append("extern u8  D_80102A68[];       /* 5 funcs */")
    lines.append("extern s32 D_80103608[];       /* 4 funcs */")
    lines.append("extern s32 D_800F66A0[];       /* 5 funcs */")
    lines.append("")

    # Top globals
    lines.append("/* Top referenced globals */")
    globals_list = [
        ("D_800A374C", 320), ("D_800A38B4", 171), ("D_800A387C", 77),
        ("D_8009BC7C", 71), ("D_80099D88", 65), ("D_8009BF48", 58),
        ("D_800A3834", 56), ("D_8009BD38", 47), ("D_8009BF7C", 46),
        ("D_800A3820", 44), ("D_8009BF78", 42), ("D_800A2CDC", 39),
        ("D_800F116C", 36), ("D_800A36AC", 36), ("D_8009C928", 36),
        ("D_8009BE6C", 36), ("D_800F1180", 35), ("D_800A3560", 35),
        ("D_8009BE70", 35),
    ]
    for name, refs in globals_list:
        lines.append(f"extern s32 {name};     /* {refs} refs */")
    lines.append("")

    # Function signatures
    lines.append("/* Function signatures */")
    for func in sorted(all_sigs.keys()):
        sig = all_sigs[func]
        lines.append(f"{sig};")

    output = '\n'.join(lines) + '\n'
    out_path = ROOT / "include" / "m2c_context.h"
    out_path.write_text(output)
    print(f"\nWrote {out_path} ({len(all_sigs)} function signatures)")

    # Stats
    gameobj_all = sum(1 for s in all_sigs.values() if 'GameObj' in s)
    void_ret = sum(1 for s in all_sigs.values() if s.startswith('void'))
    s32_ret = sum(1 for s in all_sigs.values() if s.startswith('s32'))
    print(f"  Total with GameObj*: {gameobj_all}")
    print(f"  void return: {void_ret}")
    print(f"  s32 return: {s32_ret}")


if __name__ == "__main__":
    main()
