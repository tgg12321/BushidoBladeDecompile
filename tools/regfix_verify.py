#!/usr/bin/env python3
"""Binary-level verification: compare built function against original.

Usage:
    python3 tools/regfix_verify.py <func_name>
    python3 tools/regfix_verify.py --all          # check all C functions

Compares the built binary (build/bb2.exe) against the original (disc/SLUS_006.63)
at the byte level for a specific function, and decodes any differing instructions
back to TEXT instruction indices so you know exactly which regfix rule to fix.

Output: per-instruction diff with decoded MIPS, mapped to TEXT indices.
Exit code: 0 if match, 1 if diffs found.
"""

import re
import struct
import subprocess
import sys
from pathlib import Path

PS_EXE_HEADER = 0x800
LOAD_ADDR = 0x80010000

MIPS_OPCODES = {
    0: 'SPECIAL', 1: 'REGIMM', 2: 'j', 3: 'jal',
    4: 'beq', 5: 'bne', 6: 'blez', 7: 'bgtz',
    8: 'addi', 9: 'addiu', 10: 'slti', 11: 'sltiu',
    12: 'andi', 13: 'ori', 14: 'xori', 15: 'lui',
    32: 'lb', 33: 'lh', 34: 'lwl', 35: 'lw',
    36: 'lbu', 37: 'lhu', 38: 'lwr',
    40: 'sb', 41: 'sh', 42: 'swl', 43: 'sw',
    46: 'swr',
}

SPECIAL_FUNCTS = {
    0: 'sll', 2: 'srl', 3: 'sra', 4: 'sllv', 6: 'srlv', 7: 'srav',
    8: 'jr', 9: 'jalr', 12: 'syscall', 13: 'break',
    16: 'mfhi', 17: 'mthi', 18: 'mflo', 19: 'mtlo',
    24: 'mult', 25: 'multu', 26: 'div', 27: 'divu',
    32: 'add', 33: 'addu', 34: 'sub', 35: 'subu',
    36: 'and', 37: 'or', 38: 'xor', 39: 'nor',
    42: 'slt', 43: 'sltu',
}

REGIMM_RTS = {0: 'bltz', 1: 'bgez', 16: 'bltzal', 17: 'bgezal'}

REG_NAMES = {
    0: '$zero', 1: '$at', 2: '$v0', 3: '$v1',
    4: '$a0', 5: '$a1', 6: '$a2', 7: '$a3',
    8: '$t0', 9: '$t1', 10: '$t2', 11: '$t3',
    12: '$t4', 13: '$t5', 14: '$t6', 15: '$t7',
    16: '$s0', 17: '$s1', 18: '$s2', 19: '$s3',
    20: '$s4', 21: '$s5', 22: '$s6', 23: '$s7',
    24: '$t8', 25: '$t9', 26: '$k0', 27: '$k1',
    28: '$gp', 29: '$sp', 30: '$fp', 31: '$ra',
}


def reg(n):
    return REG_NAMES.get(n, f'${n}')


def decode_mips(word, addr=0):
    op = (word >> 26) & 0x3F
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    rd = (word >> 11) & 0x1F
    sa = (word >> 6) & 0x1F
    funct = word & 0x3F
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    target = word & 0x3FFFFFF

    if word == 0:
        return 'nop'

    if op == 0:  # SPECIAL
        name = SPECIAL_FUNCTS.get(funct, f'special_{funct}')
        if funct in (0, 2, 3):  # shift by immediate
            return f'{name} {reg(rd)},{reg(rt)},{sa}'
        if funct in (4, 6, 7):  # shift by register
            return f'{name} {reg(rd)},{reg(rt)},{reg(rs)}'
        if funct in (8, 9):  # jr/jalr
            if funct == 9:
                return f'jalr {reg(rd)},{reg(rs)}'
            return f'jr {reg(rs)}'
        if funct in (16, 18):  # mfhi/mflo
            return f'{name} {reg(rd)}'
        if funct in (17, 19):  # mthi/mtlo
            return f'{name} {reg(rs)}'
        if funct in (24, 25, 26, 27):  # mult/div
            return f'{name} {reg(rs)},{reg(rt)}'
        if funct >= 32:  # add/sub/and/or/etc
            return f'{name} {reg(rd)},{reg(rs)},{reg(rt)}'
        return f'{name} ...'

    if op == 1:  # REGIMM
        name = REGIMM_RTS.get(rt, f'regimm_{rt}')
        branch_target = addr + 4 + (simm << 2)
        return f'{name} {reg(rs)},0x{branch_target:08x}'

    if op in (2, 3):  # j/jal
        name = MIPS_OPCODES[op]
        jump_addr = (addr & 0xF0000000) | (target << 2)
        return f'{name} 0x{jump_addr:08x}'

    if op in (4, 5, 6, 7):  # branches
        name = MIPS_OPCODES[op]
        branch_target = addr + 4 + (simm << 2)
        if op in (6, 7):  # blez/bgtz
            return f'{name} {reg(rs)},0x{branch_target:08x}'
        return f'{name} {reg(rs)},{reg(rt)},0x{branch_target:08x}'

    if op == 15:  # lui
        return f'lui {reg(rt)},0x{imm:04x}'

    if op in (8, 9, 10, 11, 12, 13, 14):  # immediate ALU
        name = MIPS_OPCODES[op]
        if op in (12, 13, 14):  # unsigned imm
            return f'{name} {reg(rt)},{reg(rs)},0x{imm:x}'
        return f'{name} {reg(rt)},{reg(rs)},{simm}'

    if op in (32, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 46):  # load/store
        name = MIPS_OPCODES[op]
        return f'{name} {reg(rt)},{simm}({reg(rs)})'

    return f'op{op} {reg(rs)},{reg(rt)},0x{imm:x}'


def get_func_address_range(root, func_name):
    """Get function start address and size from the linker map."""
    map_file = root / 'build' / 'bb2.map'
    if not map_file.exists():
        return None, None

    map_text = map_file.read_text()
    pattern = rf'^\s+0x([0-9a-f]+)\s+{re.escape(func_name)}$'
    m = re.search(pattern, map_text, re.MULTILINE)
    if not m:
        return None, None

    func_addr = int(m.group(1), 16)

    # Find the next symbol after this one to determine size
    all_syms = re.findall(r'^\s+0x([0-9a-f]+)\s+(\S+)$', map_text, re.MULTILINE)
    syms_sorted = sorted(set((int(a, 16), n) for a, n in all_syms))
    for i, (addr, name) in enumerate(syms_sorted):
        if addr == func_addr and name == func_name:
            if i + 1 < len(syms_sorted):
                next_addr = syms_sorted[i + 1][0]
                return func_addr, next_addr - func_addr
            break

    return func_addr, None


def get_text_instruction_map(root, func_name, func_addr):
    """Map binary offsets to TEXT instruction indices by running the full pipeline.

    Returns dict: binary_offset_from_func_start -> text_index
    """
    cmd = f"python3 tools/dump_text_indices.py {func_name}"
    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True, cwd=str(root)
    )
    if result.returncode != 0 and not result.stdout.strip():
        return {}

    # Parse TEXT indices and track pseudo-instruction expansions
    insn_map = {}
    binary_offset = 0

    for line in result.stdout.splitlines():
        m = re.match(r'\s*(\d+):\s*(.*)', line)
        if not m:
            continue
        text_idx = int(m.group(1))
        raw = m.group(2).strip()

        insn_map[binary_offset] = text_idx

        # Check for pseudo-instructions that expand to 2 words
        if is_pseudo_2word_raw(raw):
            binary_offset += 8  # 2 instructions
        else:
            binary_offset += 4

    return insn_map


def is_pseudo_2word_raw(raw):
    norm = raw.strip()
    if re.match(r'la\s+\$', norm, re.IGNORECASE):
        return True
    m = re.match(r'li\s+\$\d+\s*,\s*(.+)', norm, re.IGNORECASE)
    if m:
        try:
            val = int(m.group(1).strip(), 0)
            if val > 0xFFFF or val < -0x8000:
                return True
        except ValueError:
            return True
    if re.match(r'(?:lb|lbu|lh|lhu|lw|sb|sh|sw)\s+\$\d+\s*,\s*[A-Za-z_]', norm):
        return True
    return False


def verify_function(root, func_name, verbose=True):
    """Compare built binary against original for a specific function.

    Returns list of (offset, addr, ours_word, target_word, text_idx, ours_decoded, target_decoded).
    """
    build_exe = root / 'build' / 'bb2.exe'
    orig_exe = root / 'disc' / 'SLUS_006.63'

    if not build_exe.exists():
        print(f"ERROR: {build_exe} not found (run make first)", file=sys.stderr)
        return None
    if not orig_exe.exists():
        print(f"ERROR: {orig_exe} not found", file=sys.stderr)
        return None

    func_addr, func_size = get_func_address_range(root, func_name)
    if func_addr is None:
        print(f"ERROR: {func_name} not found in build/bb2.map", file=sys.stderr)
        return None

    if func_size is None:
        func_size = 0x1000  # fallback: scan up to 4KB

    with open(build_exe, 'rb') as f:
        build_data = f.read()
    with open(orig_exe, 'rb') as f:
        orig_data = f.read()

    # Get TEXT instruction index mapping
    insn_map = get_text_instruction_map(root, func_name, func_addr)

    file_offset = PS_EXE_HEADER + (func_addr - LOAD_ADDR)
    diffs = []

    for i in range(0, func_size, 4):
        off = file_offset + i
        if off + 4 > len(build_data) or off + 4 > len(orig_data):
            break

        our_word = struct.unpack_from('<I', build_data, off)[0]
        tgt_word = struct.unpack_from('<I', orig_data, off)[0]

        if our_word != tgt_word:
            addr = func_addr + i
            text_idx = insn_map.get(i)
            our_decoded = decode_mips(our_word, addr)
            tgt_decoded = decode_mips(tgt_word, addr)
            diffs.append((i, addr, our_word, tgt_word, text_idx, our_decoded, tgt_decoded))

    if verbose:
        if not diffs:
            print(f"{func_name}: MATCH (0 diffs in {func_size} bytes)")
        else:
            print(f"{func_name}: {len(diffs)} instruction(s) differ "
                  f"(function at 0x{func_addr:08X}, {func_size} bytes)")
            for offset, addr, ours, tgt, text_idx, our_dec, tgt_dec in diffs:
                idx_str = f"TEXT idx {text_idx}" if text_idx is not None else "???"
                xor = ours ^ tgt
                print(f"  0x{addr:08X} (+{offset:3d}) [{idx_str}]:")
                print(f"    ours:   0x{ours:08X}  {our_dec}")
                print(f"    target: 0x{tgt:08X}  {tgt_dec}")
                # Highlight which fields differ
                if xor & 0x03E00000:  # rs field
                    print(f"    -> rs differs")
                if xor & 0x001F0000:  # rt field
                    print(f"    -> rt differs")
                if xor & 0x0000F800:  # rd field
                    print(f"    -> rd differs")
                if xor & 0x0000FFFF and not (xor & 0xFFFF0000):
                    print(f"    -> immediate/offset differs")

    return diffs


def get_all_c_functions(root):
    """Find all functions that are defined as C code (not INCLUDE_ASM stubs)."""
    map_file = root / 'build' / 'bb2.map'
    if not map_file.exists():
        return []

    # Get all symbols from the map
    map_text = map_file.read_text()
    all_syms = re.findall(r'^\s+0x([0-9a-f]+)\s+(\w+)$', map_text, re.MULTILINE)

    # Find which functions are C code (have definitions, not INCLUDE_ASM)
    c_funcs = set()
    for c_file in sorted((root / 'src').glob('*.c')):
        text = c_file.read_text()
        # Find function definitions (not INCLUDE_ASM)
        for m in re.finditer(r'^(?:void|int|s32|u32|u8|s8|u16|s16)\s+(\w+)\s*\([^;]*\)\s*\{', text, re.MULTILINE):
            c_funcs.add(m.group(1))
        # Also match plain type-less: static void, etc
        for m in re.finditer(r'^(?:static\s+)?(?:void|int|s32|u32)\s+(\w+)\s*\(', text, re.MULTILINE):
            if m.group(1) not in c_funcs:
                # Check it's a definition (has {)
                rest = text[m.end():]
                if '{' in rest[:200]:
                    c_funcs.add(m.group(1))

    # Filter to only symbols in the map
    sym_set = {name for _, name in all_syms}
    return sorted(c_funcs & sym_set)


def main():
    root = Path(__file__).resolve().parent.parent

    if len(sys.argv) < 2:
        print("Usage: python3 tools/regfix_verify.py <func_name>", file=sys.stderr)
        print("       python3 tools/regfix_verify.py --all", file=sys.stderr)
        sys.exit(1)

    if sys.argv[1] == '--all':
        funcs = get_all_c_functions(root)
        if not funcs:
            print("No C functions found", file=sys.stderr)
            sys.exit(1)
        total_diffs = 0
        failed = []
        for func in funcs:
            diffs = verify_function(root, func, verbose=False)
            if diffs is None:
                continue
            if diffs:
                failed.append((func, len(diffs)))
                total_diffs += len(diffs)

        if failed:
            print(f"\n{len(failed)} functions with diffs:")
            for func, count in failed:
                print(f"  {func}: {count} instruction(s)")
            print(f"\nTotal: {total_diffs} differing instructions")
            sys.exit(1)
        else:
            print(f"All {len(funcs)} C functions match")
    else:
        func_name = sys.argv[1]
        diffs = verify_function(root, func_name)
        if diffs is None:
            sys.exit(2)
        sys.exit(1 if diffs else 0)


if __name__ == '__main__':
    main()
